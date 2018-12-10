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
const int queueSizeMultiplicator = 10; // to limit memory usage

class Compressor
{
private:
    int compressionLevel = -1;
    int threadCount = 0;

    std::queue<Job> queue; // queue of compression jobs
    std::mutex queueMutex;
    std::condition_variable queueCond;
    std::vector<std::future<void>> futures;
    bool stop = false;
    WriterEnqueueFunc EnqueueWriterJob; // to send a job for writing

    inline int MaxQueueSize() const { return threadCount * queueSizeMultiplicator; }
    bool GetJob(Job &job);
    int CompressChunk(Job &job);
    void CompressLoop();
public:
    void Start(int count, const WriterEnqueueFunc &writerEnqueueFunc);
    void Enqueue(Job &job); // for compressor clients - add job to compression queue
    void Complete(); // let the comression threads finish
};