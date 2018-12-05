#pragma once

#include "BlockWriter.h"

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>

class Compressor
{
private:
    int compressionLevel = -1;

    std::queue<Job> queue;
    std::mutex queueMutex;
    std::condition_variable queueCond;
    std::vector<std::future<void>> futures;
    bool stop = false;
    int threadCount;
    BlockWriter *writer;

    bool GetCompressJob(Job &job);
    int CompressChunk(Job &job);
    void CompressLoop();
public:
    void StartThreads(BlockWriter *blockWriter, int count);
    void PushJob(Job &job);
    void Complete();
};