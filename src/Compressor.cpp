#include "pch.h"

#include "Compressor.h"
#include "zlib_intf.h"

#include <cassert>

void Compressor::StartThreads(BlockWriter *blockWriter, int count)
{
    assert(futures.empty());
    writer = blockWriter;
    threadCount = count;
    for (int i = 0; i < count; ++i)
    {
        futures.push_back(std::async(std::launch::async, &Compressor::CompressLoop, this));
    }
}

void Compressor::PushJob(Job &job)
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCond.wait(lock, [this] { return  // done ||
            queue.size() < threadCount * 2; });

        queue.push(std::move(job));
    }
    queueCond.notify_one();
}

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
    do
    {
        Job job;
        if (!GetCompressJob(job))
            break;

        queueCond.notify_all();
        if (0 != CompressChunk(job))
        {
            // TODO show error
        }
        writer->PushJob(job);
    } while (!stop);
}

bool Compressor::GetCompressJob(Job &job)
{
    std::unique_lock<std::mutex> lock(queueMutex);
    queueCond.wait(lock, [this] { return stop || queue.size() > 0; });
    if (stop)
        return false;

    job = std::move(queue.front());
    queue.pop();
    return true;
}

int Compressor::CompressChunk(Job &job)
{
    return deflate(compressionLevel, job.inbuf, job.outbuf);
}