#include "pch.h"

#include "Compressor.h"
#include "zlib_intf.h"

#include <cassert>

// start compression threads
void Compressor::Start(int count, const std::function<void(Job &)> &writerEnqueueFunc)
{
    assert(futures.empty());
	EnqueueWriterJob = writerEnqueueFunc;
    threadCount = count;
    for (int i = 0; i < count; ++i)
    {
        futures.push_back(std::async(std::launch::async, &Compressor::CompressLoop, this));
    }
}

void Compressor::Enqueue(Job &job)
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCond.wait(lock, [this] { return  stop || (queue.size() < threadCount * 2); });
		if (!stop)
			queue.push(std::move(job));
    }
    queueCond.notify_one();
}

// stop compression threads
void Compressor::Complete()
{
    stop = true;
    queueCond.notify_all();
    for (auto &f : futures)
        f.get();
}

// compressor thread
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

int Compressor::CompressChunk(Job &job)
{
    return deflate(compressionLevel, job.inbuf, job.outbuf);
}