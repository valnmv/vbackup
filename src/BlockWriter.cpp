#include "pch.h"
#include "BlockWriter.h"

void BlockWriter::Init(const std::wstring &filename, std::vector<IndexBlock> *index)
{
    fileIndex = index;
    stream.open(filename, std::ios::binary | std::ios::app);
}

void BlockWriter::Write()
{
    while (!stop)
    {
        Job job;
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

void BlockWriter::PushJob(const Job &job)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        writeQueue.push(std::move(job));
    }
    queueDataCond.notify_all();
}

void BlockWriter::Complete(uint64_t jobCount)
{
    std::unique_lock<std::mutex> lock(queueMutex);
    queueDataCond.wait(lock, [this, jobCount]() { return jobsWriten == jobCount; });
    stream.close();
    stop = true;
    queueDataCond.notify_all();
}

bool BlockWriter::PopJob(Job &job)
{
    std::unique_lock<std::mutex> lock(queueMutex);
    queueDataCond.wait(lock, [this] { return stop ||
        (writeQueue.size() > 0 && (jobsWriten + 1 == writeQueue.top().no)); });

    if (stop)
        return false;

    job = std::move(writeQueue.top());
    writeQueue.pop();
    return true;
}

DataBlock BlockWriter::DataBlockFromJob(const Job &job)
{
    DataBlock block{ job.no, job.inbuf.size(), job.outbuf.size() };
    block.data = std::move(job.outbuf);
    return std::move(block);
}