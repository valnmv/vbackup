#include "pch.h"
#include "ArchiveWriter.h"

DataBlock DataBlockFromJob(const Job &job)
{
    DataBlock block{ job.no, job.inbuf.size(), job.outbuf.size() };
    block.data = std::move(job.outbuf);
    return std::move(block);
}

void ArchiveWriter::Init(const std::wstring &filename, std::vector<IndexBlock> *index)
{
    fileIndex = index;
    stream.open(filename, std::ios::binary | std::ios::app);
}

void ArchiveWriter::Write()
{
    Job job;
    while (!stop)
    {
        if (!PopJob(job))
            break;

        if (job.fileNo != fileNoWriting)
        {
            fileNoWriting = job.fileNo;
            uint64_t pos = stream.tellp();
            (*fileIndex)[job.indexBlockNo][job.indexRecNo].offset = pos;
        }

        DataBlock block = DataBlockFromJob(job);
        block.write(stream);
        ++jobsWriten;
        queueDataCond.notify_all();
    }
}

void ArchiveWriter::PushJob(const Job &job)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        writerQueue.push(std::move(job));
    }
    queueDataCond.notify_all();
}

void ArchiveWriter::Complete(uint64_t jobCount)
{
    std::unique_lock<std::mutex> lock(queueMutex);
    queueDataCond.wait(lock, [this, jobCount]() { return jobsWriten == jobCount; });
    stream.close();
    stop = true;
}

bool ArchiveWriter::PopJob(Job &job)
{
    std::unique_lock<std::mutex> lock(queueMutex);
    queueDataCond.wait(lock, [this] { return stop ||
        (writerQueue.size() > 0 && (jobsWriten + 1 == writerQueue.top().no)); });

    if (stop)
        return false;

    job = std::move(writerQueue.top());
    writerQueue.pop();
    return true;
}
