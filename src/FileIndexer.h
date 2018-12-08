// FileIndexer - traverse files, create file index, enqueue jobs for compression
//

#pragma once

#include "FileBlocks.h"
#include "Job.h"

#include <fstream>
#include <functional>
#include <map>

using EnqueueCompressorJobFunc = std::function<void(Job &)>;
using FileIndex = std::map<uint64_t, std::unique_ptr<IndexBlock>>;

struct FileIndexerStatistics
{
    uint64_t totalBytes = 0;
    uint64_t bytesCompressed = 0;
    std::atomic<uint64_t> filesIndexed = 0;
    std::atomic<uint64_t> jobsCreated = 0;
};

class FileIndexer
{
private:
	std::wstring source;
    std::wstring dataFile;
    std::wstring indexFile;
    std::wofstream indexStream;

    bool indexingFinished = false;
    FileIndexerStatistics statistics;
    FileIndex indexBlocks;
	uint64_t blockNo = 0;
    EnqueueCompressorJobFunc EnqueueCompressorJob;
    
    void ProcessIndexBlock(const IndexBlock &block);
    void CreateJobs(const std::wstring &path, const uint64_t fileNo, const std::size_t indexBlockNo,
        const std::size_t indexRecNo);
	void ReadChunk(std::ifstream &is, std::vector<uint8_t> &buffer, uintmax_t bytes);
public:
    const FileIndexerStatistics& Statistics() const { return statistics; }
    void WriteJobFinished(const Job &job);
    void SetFileOffset(size_t blockNo, size_t recNo, uintmax_t offset);
    void ListFiles(const std::wstring &path);
    void Start(const std::wstring &src, const std::wstring &dest, const std::wstring &index,
        const EnqueueCompressorJobFunc &enqueueCompressorJobFunc);
};


