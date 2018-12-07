// Compressor - compress source data chunks, uses a job queue
//

#pragma once

#include "Job.h"

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <vector>

using WriterEnqueueFunc = std::function<void(Job &)>;

class Compressor
{
private:
    int compressionLevel = -1;

    std::queue<Job> queue;
    std::mutex queueMutex;
    std::condition_variable queueCond;
    std::vector<std::future<void>> futures;
    bool stop = false;
    int threadCount = 0;
    WriterEnqueueFunc EnqueueWriterJob;

    bool GetJob(Job &job);
    int CompressChunk(Job &job);
    void CompressLoop();
public:
    void Start(int count, const WriterEnqueueFunc &writerEnqueueFunc);
    void Enqueue(Job &job); // for compressor clients - add job to compression queue
    void Complete();
};