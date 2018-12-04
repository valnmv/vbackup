#pragma once

#include "FileBlocks.h"
#include "Job.h"

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <map>
#include <mutex>
#include <queue>

struct CompareJob
{
    bool operator() (const Job &lhs, const Job &rhs) { return lhs.no > rhs.no; }
};

class Archiver
{
private:
    int compressionLevel = -1;

	std::wstring source;
    std::wstring destination;
	std::ofstream destinationStream;
	
	bool done = false;

    std::atomic<uint64_t> filesIndexed = 0;
    std::atomic<uint64_t> jobsCreated = 0;
    std::atomic<uint64_t> jobsWriten = 0;
	std::atomic<uint64_t> fileNoWriting = 0;

    std::queue<Job> compQueue;
    std::mutex compQueueMutex;
    std::condition_variable compQueueHasData;

    std::vector<IndexBlock> indexBlocks;

    std::priority_queue<Job, std::vector<Job>, CompareJob> writerQueue;
    std::mutex writerQueueMutex;
    std::condition_variable writerQueueHasData;
    std::condition_variable writerQueueFinished;

    void ListFiles(const std::wstring &path);
    void ProcessIndexBlock(const IndexBlock &block);
    void CreateJobs(const std::wstring &path, const uint64_t fileNo, const std::size_t indexBlockNo,
        const std::size_t indexRecNo);
    void Compress();
    bool GetCompressJob(Job &job);
    int CompressChunk(Job &job);
    void Write();
    bool GetWriteJob(Job &job);
public:
    void Run(const std::wstring &src, const std::wstring &dest);
};