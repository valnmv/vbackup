#include "pch.h"
#include "FileBlocks.h"
#include "Unarchiver.h"
#include "ProgressIndicator.h"

#include <cassert>
#include <filesystem>

#include "zlib_intf.h"

namespace fs = std::experimental::filesystem;

// Read index and compressed data files, decompress and restore
void Unarchiver::Run(const std::wstring &src, const std::wstring &dest)
{
    dataFile = src;
    destination = dest;
    if (fs::path(destination).generic_string().back() != fs::path::preferred_separator)
        destination += fs::path::preferred_separator;

    indexFile = dataFile + L".i";
    uint64_t totalBytes = fs::file_size(fs::path(dataFile));
    dataStream.open(dataFile, std::ios::binary);
    indexStream.open(indexFile);
    indexStream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
    

    ProgressIndicator indicator;
    indicator.PrintText(L"Restoring...");
    // TODO check long double max
    blockReadyFunc = [&indicator, totalBytes](uint64_t pos) {
        long double value = static_cast<long double>(pos) / totalBytes;
        indicator.Update(static_cast<float>(value)); };

    IndexBlock indexBlock;
    while (ReadIndexBlock(indexBlock))
    {
        ProcessIndexBlock(indexBlock);
    }
    indicator.PrintTimeElapsed();
}

bool Unarchiver::ReadIndexBlock(IndexBlock &block)
{
    if (indexStream.eof() || EOF == dataStream.peek())
        return false;

    block.read(indexStream);
    return true;
}

// Restore files per index blocks, creates directories
void Unarchiver::ProcessIndexBlock(const IndexBlock &index)
{
    fs::path dir = index.records[0].name;
    dir = destination / dir.relative_path();
    fs::create_directories(dir);

    for (const auto &r : index.records)
    {
        fs::path path = dir / r.name;
        if (r.type == static_cast<short>(fs::file_type::directory))
            fs::create_directories(path);

        if (r.type == static_cast<short>(fs::file_type::regular))
            RestoreFile(path, r);
    }
}

void Unarchiver::RestoreFile(const std::wstring &path, const IndexRecord &rec)
{
    std::ofstream os(path, std::ios::binary);
    dataStream.seekg(rec.offset);
    uint64_t bytesProcessed = 0; // per file
    DataBlock block;
    while (ReadDataBlock(block))
    {
        DecompressChunk(block);
        os.write(reinterpret_cast<const char*>(&inflateBuffer[0]), block.origLength);
        bytesProcessed += block.origLength;
        assert(bytesProcessed <= rec.length);
        
        // TODO implement better progress measure
        decompressedBytes += block.length;
        blockReadyFunc(decompressedBytes);

        if (bytesProcessed == rec.length)
            break;
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
