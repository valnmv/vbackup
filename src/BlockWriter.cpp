// BlockWriter - processes jobs to write to disk compressed data, 
// uses its own priority queue ordered by job#
//

#include "pch.h"
#include "BlockWriter.h"
#include "FileBlocks.h"

// Start writer thread; <offsetFunc> is used to store compressed data offset in index blocks;
// <jobFinishedFunction> is to update progress indicator and write index block when the last file 
// in an index block was processed
void BlockWriter::Start(const std::wstring &filename, const SetOffsetFunction &offsetFunc,
	const WriteJobFinishedFunction &jobFinishedFunction)
{
	SetFileOffset = offsetFunc;
	WriteJobFinished = jobFinishedFunction;
    stream.open(filename, std::ios::binary);
	future = std::async(std::launch::async, &BlockWriter::WriteLoop, this);
}

// Block writer thread
void BlockWriter::WriteLoop()
{
    while (!stop)
    {
        Job job;
        if (!GetJob(job))
            break;

        if (job.fileNo != fileNoWriting)
        {
            fileNoWriting = job.fileNo;
            uint64_t pos = stream.tellp();
			SetFileOffset(job.indexBlockNo, job.indexRecNo, pos);
        }

		DataBlock block{ job.no, job.inbuf.size(), job.outbuf.size(), std::move(job.outbuf) };
        block.write(stream);
        ++jobsWriten;
        queueCond.notify_all();
        if (job.lastFileJob)
		    WriteJobFinished(job);
    }
}

// For clients - to push job for writing
void BlockWriter::Enqueue(Job &job)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        queue.push(std::move(job));
    }
    queueCond.notify_all();
}

// Stop the thread when all jobs are processed
void BlockWriter::Complete(uint64_t jobCount)
{
    std::unique_lock<std::mutex> lock(queueMutex);
    queueCond.wait(lock, [this, jobCount]() { return jobsWriten == jobCount; });
    stream.close();
    stop = true;
    queueCond.notify_all();
}

// Get a job from the own processing queue
bool BlockWriter::GetJob(Job &job)
{
    std::unique_lock<std::mutex> lock(queueMutex);
    queueCond.wait(lock, [this] { return stop ||
        (queue.size() > 0 && (jobsWriten + 1 == queue.top().no)); });

    if (stop)
        return false;

    job = std::move(queue.top());
    queue.pop();
    return true;
}
