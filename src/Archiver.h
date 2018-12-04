#pragma once

#include "FileBlocks.h"
#include "Job.h"

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>

class Archiver
{
private:
    int compressionLevel = -1;
    std::wstring source;
    std::wstring destination;
    bool done = false;

    std::atomic<uint64_t> jobsCreated = 0;
    std::atomic<uint64_t> jobsWriten = 0;

    std::queue<Job> compQueue;
    std::mutex compQueueMutex;
    std::condition_variable compQueueHasData;

    //std::vector<DataBlock> dataBlocks;
    std::queue<IndexBlock> indexQueue;

    std::queue<Job> writerQueue;
    std::mutex writerQueueMutex;
    std::condition_variable writerQueueHasData;
    std::condition_variable writerQueueFinished;

    void ListFiles(const std::wstring &path);
    void ProcessIndexBlock(const IndexBlock &block);
    void CreateJobs(const std::wstring &path);
    void Compress();
    int CompressChunk(Job &task);
    void Write();
public:
    void Run(const std::wstring &src, const std::wstring &dest);
};