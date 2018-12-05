#pragma once

#include "BlockWriter.h"
#include "FileBlocks.h"
#include "Job.h"

#include <atomic>
#include <condition_variable>
#include <fstream>
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
    std::atomic<uint64_t> filesIndexed = 0;
    std::atomic<uint64_t> jobsCreated = 0;
    std::queue<Job> compQueue;
    std::mutex compQueueMutex;
    std::condition_variable compQueueHasData;
    std::vector<IndexBlock> indexBlocks;
    BlockWriter archiveWriter;

    void ListFiles(const std::wstring &path);
    void ProcessIndexBlock(const IndexBlock &block);
    void CreateJobs(const std::wstring &path, const uint64_t fileNo, const std::size_t indexBlockNo,
        const std::size_t indexRecNo);
    void Compress();
    bool GetCompressJob(Job &job);
    int CompressChunk(Job &job);
public:
    void Run(const std::wstring &src, const std::wstring &dest);
};


