#include "pch.h"
#include "FileBlocks.h"
#include "Unarchiver.h"

#include <cassert>
#include <filesystem>

#include "zlib_intf.h"

namespace fs = std::experimental::filesystem;

// Read index and compressed data files, decompress and restore
void Unarchiver::Run(const std::wstring &src, const std::wstring &dest)
{
    dataFile = src;
    destination = dest;
    indexFile = dataFile + L".i";
    dataStream.open(dataFile, std::ios::binary);
    indexStream.open(indexFile);
    indexStream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

    IndexBlock indexBlock;
    while (ReadIndexBlock(indexBlock))
    {
        ProcessIndexBlock(indexBlock);
    }
}

void Unarchiver::RestoreFile(const std::wstring &path, const IndexRecord &rec)
{
    std::ofstream os(path, std::ios::binary);
    dataStream.seekg(rec.offset);
    uint64_t bytesProcessed = 0;
    DataBlock block;
    while (ReadDataBlock(block))
    {
        DecompressChunk(block);
        os.write(reinterpret_cast<const char*>(&inflateBuffer[0]), block.origLength);
        bytesProcessed += block.origLength;
        if (bytesProcessed == rec.length)
            break;
    }
}

// Restore files per index blocks, creates directories
void Unarchiver::ProcessIndexBlock(const IndexBlock &index)
{
    fs::path dir = index.records[0].name;
    dir = destination / dir.relative_path();
    fs::create_directories(dir);

    for (const auto &r : index.records)
    {
        if (r.type == static_cast<short>(fs::file_type::directory))
            fs::create_directories(r.name);

        if (r.type == static_cast<short>(fs::file_type::regular))
            RestoreFile((dir) / fs::path(r.name), r);
    }
}

bool Unarchiver::ReadDataBlock(DataBlock &block)
{
	if (dataStream.eof() || EOF == dataStream.peek())
		return false;

	block.read(dataStream);
	return true;
}

int Unarchiver::DecompressChunk(const DataBlock &block)
{
	if (inflateBuffer.size() < block.origLength)
		inflateBuffer.resize(block.origLength);

	return inflate(block.data, inflateBuffer);
}

bool Unarchiver::ReadIndexBlock(IndexBlock &block)
{
    if (indexStream.eof())
        return false;

    block.read(indexStream);
    return true;
}
