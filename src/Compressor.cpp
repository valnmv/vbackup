#include "pch.h"

#include "Compressor.h"
#include "zlib_intf.h"

#include <cassert>

// Start <count> compression threads; <writerEnqueueFunc> is used by the threads after compression 
// to push a job to writer queue
void Compressor::Start(int count, const WriterEnqueueFunc &writerEnqueueFunc)
{
    assert(futures.empty());
	EnqueueWriterJob = writerEnqueueFunc;
    threadCount = count;
    for (int i = 0; i < count; ++i)
    {
        futures.push_back(std::async(std::launch::async, &Compressor::CompressLoop, this));
    }
}

// For compressor clients - add job to the compression queue
void Compressor::Enqueue(Job &job)
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCond.wait(lock, [this] { return stop || (queue.size() < MaxQueueSize()); });
		if (!stop)
			queue.push(std::move(job));
    }
    queueCond.notify_one();
}

// Stop compression threads
void Compressor::Complete()
{
    stop = true;
    queueCond.notify_all();
    for (auto &f : futures)
        f.get();
}

// Compressor thread
void Compressor::CompressLoop()
{
	while (!stop)
    {
        Job job;
        if (!GetJob(job))
            break;

        if (0 != CompressChunk(job))
        { // TODO show error
        }
		EnqueueWriterJob(job);
    }
}

// Get job to process from the compression queue
bool Compressor::GetJob(Job &job)
{
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		queueCond.wait(lock, [this] { return stop || queue.size() > 0; });
		if (stop)
			return false;

		job = std::move(queue.front());
		queue.pop();
	}
	queueCond.notify_all();
    return true;
}

// Compress data from the job.inbuf to job.outbuf
int Compressor::CompressChunk(Job &job)
{
    return deflate(compressionLevel, job.inbuf, job.outbuf);
}