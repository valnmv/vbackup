#pragma once

#include "FileBlocks.h"
#include "Job.h"

#include <atomic>
#include <map>
#include <queue>

class Archiver
{
private:
    int compressionLevel = -1;
    std::wstring source;
    std::wstring destination;

    std::atomic<uint64_t> jobNo;
    std::queue<Job> jobQueue;

    std::queue<IndexBlock> indexQueue;
    std::queue<Job> writerQueue;

    void ListFiles(const std::wstring &path);
    void ProcessIndexBlock(const IndexBlock &block);
    void CreateJobs(const std::wstring &path);
    void ProcessJob();
    int CompressChunk(Job &task);
public:
    void Run(const std::wstring &src, const std::wstring &dest);
};