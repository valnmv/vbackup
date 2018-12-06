#pragma once

#include "Job.h"

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <vector>

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
	std::function<void(Job &)> EnqueueWriterJob;

    bool GetJob(Job &job);
    int CompressChunk(Job &job);
    void CompressLoop();
public:
    void Start(int count, const std::function<void(Job &)> &writerEnqueueFunc);
    void Enqueue(Job &job);
    void Complete();
};