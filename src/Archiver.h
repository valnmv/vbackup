#pragma once

#include "BlockWriter.h"
#include "Compressor.h"
#include "FileBlocks.h"
#include "Job.h"

#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>

class Archiver
{
private:
	std::wstring source;
    std::wstring destination;	
	bool done = false;
    std::atomic<uint64_t> filesIndexed = 0;
    std::atomic<uint64_t> jobsCreated = 0;
    std::vector<IndexBlock> indexBlocks;

    Compressor compressor;
    BlockWriter writer;

    void ListFiles(const std::wstring &path);
    void ProcessIndexBlock(const IndexBlock &block);
    void CreateJobs(const std::wstring &path, const uint64_t fileNo, const std::size_t indexBlockNo,
        const std::size_t indexRecNo);
public:
    void Run(const std::wstring &src, const std::wstring &dest);
};


