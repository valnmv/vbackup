#pragma once

#include "FileBlocks.h"

#include <fstream>
#include <functional>
#include <string>
#include <vector>

using BlockReadyFunc = std::function<void(uint64_t)>;

class Unarchiver
{
private:
    std::wstring dataFile;
    std::wstring indexFile;
    std::wstring destination;
    std::ifstream dataStream;
    std::wifstream indexStream;
    uint64_t decompressedBytes = 0;
    std::vector<uint8_t> inflateBuffer;
    bool ReadIndexBlock(IndexBlock & block);
    void ProcessIndexBlock(const IndexBlock & index);
    void RestoreFile(const std::wstring &name, const IndexRecord &rec);
    bool ReadDataBlock(DataBlock &);
    int DecompressChunk(const DataBlock &block);
    BlockReadyFunc blockReadyFunc;
public:
    void Run(const std::wstring &src, const std::wstring &dest);
};

