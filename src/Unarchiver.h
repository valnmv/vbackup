#pragma once

#include <fstream>
#include <string>
#include <vector>

class Unarchiver
{
private:
    std::wstring source;
    std::wstring destination;
	std::ifstream sourceStream;
	std::vector<uint8_t> inflateBuffer;
    bool ReadBlock(DataBlock &);
    int DecompressChunk(const DataBlock &block);

public:
    void Run(const std::wstring &src, const std::wstring &dest);
};

