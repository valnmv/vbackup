#pragma once

#include "FileBlocks.h"
#include "Job.h"

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <map>
#include <queue>

struct CompareJob
{
    bool operator() (const Job &lhs, const Job &rhs) { return lhs.no > rhs.no; }
};

class ArchiveWriter
{
private:
    std::ofstream stream;
    std::priority_queue<Job, std::vector<Job>, CompareJob> writerQueue;
    std::mutex queueMutex;
    std::condition_variable queueDataCond;
    std::atomic<uint64_t> jobsWriten = 0;
    std::atomic<uint64_t> fileNoWriting = 0;
    std::vector<IndexBlock> *fileIndex;
    bool stop = false;

    bool PopJob(Job &job);
public:
    void Init(const std::wstring &filename, std::vector<IndexBlock> *index);
    void Write();
    void PushJob(const Job &job);
    void Complete(uint64_t jobCount);
};
