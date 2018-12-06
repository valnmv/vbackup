#pragma once

#include "FileBlocks.h"
#include "Job.h"

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <future>
#include <map>
#include <mutex>
#include <queue>

class Archiver
{
private:
	std::wstring source;
    std::wstring dataFile;
    std::wstring indexFile;
    std::wofstream indexStream;

    std::atomic<uint64_t> filesIndexed = 0;
    std::atomic<uint64_t> jobsCreated = 0;
    std::map<uint64_t, std::unique_ptr<IndexBlock>> indexBlocks;
	uint64_t blockNo = 0;
    std::function<void(Job &)> EnqueueCompressorJob;
    
    void ListFiles(const std::wstring &path);
    void ProcessIndexBlock(const IndexBlock &block);
    void CreateJobs(const std::wstring &path, const uint64_t fileNo, const std::size_t indexBlockNo,
        const std::size_t indexRecNo);
	void ReadChunk(std::ifstream &is, std::vector<uint8_t> &buffer, uintmax_t bytes);
public:
    void Run(const std::wstring &src, const std::wstring &dest);
};


