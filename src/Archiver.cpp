// Archivator - create archive for a volume
//

#include "pch.h"
#include "Archiver.h"
#include "BlockWriter.h"
#include "Compressor.h"
#include "ProgressIndicator.h"

#include "zlib_intf.h" // for CHUNK
#include <iostream>

#include <cassert>
#include <algorithm>
#include <filesystem>

namespace fs = std::experimental::filesystem;

void Archiver::Init(const std::wstring &src, const std::wstring &dest)
{
    source = src;
    dataFile = dest;
    indexFile = dataFile + L".i";
    indexStream.open(indexFile);
    indexStream.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
    compressorCount = std::thread::hardware_concurrency();
}

void Archiver::Run(const std::wstring &src, const std::wstring &dest)
{
    Init(src, dest);
    Compressor compressor;
    BlockWriter writer;
    ProgressIndicator indicator;

    auto writeJobFinished = [this, &indicator](const Job& job) { WriteJobFinished(job);
        indicator.Update(static_cast<float>(static_cast<long double>(bytesCompressed) / totalBytes));  };
    auto setFileOffset = [this](size_t blockNo, size_t recNo, uintmax_t offset) {
        indexBlocks[blockNo]->records[recNo].offset = offset; };

    //indicator.Start();
    EnqueueCompressorJob = [&compressor](Job &job) { compressor.Enqueue(job); };
    writer.Start(dataFile, setFileOffset, writeJobFinished);
    compressor.Start(compressorCount - 1, [&writer](Job &job) { writer.Enqueue(job); });

    ListFiles(source);
    writer.Complete(jobsCreated);
    compressor.Complete();
}

void Archiver::ListFiles(const std::wstring &path)
{
    std::unique_ptr<IndexBlock> indexBlockPtr = std::make_unique<IndexBlock>();
	indexBlockPtr->no = ++blockNo;
    IndexRecord header{};
    header.name = path;
    indexBlockPtr->AddRecord(header);

    for (const auto &p : fs::directory_iterator(path))
    {
		if (p.path() == dataFile || p.path() == indexFile)
			continue;

        IndexRecord rec{};
        rec.type = static_cast<short>(p.status().type());
        if (p.status().type() == fs::file_type::regular)
        {
            rec.length = fs::file_size(p);
            totalBytes += rec.length;
        }
        rec.name = p.path().filename();
        rec.fileNo = ++filesIndexed;
        indexBlockPtr->AddRecord(rec);
    }

    indexBlocks[indexBlockPtr->no] = std::move(indexBlockPtr);
    ProcessIndexBlock(*indexBlocks[blockNo]);
}

void Archiver::ProcessIndexBlock(const IndexBlock &block)
{
    IndexRecord header = block.records[0];
	for (std::size_t i = 1; i < block.records.size(); i++)
    {
		const auto &rec = block.records[i];
        if (static_cast<fs::file_type>(rec.type) == fs::file_type::directory)
            ListFiles(fs::path(header.name) / fs::path(rec.name));

        if (static_cast<fs::file_type>(rec.type) == fs::file_type::regular)
            CreateJobs(fs::path(header.name) / fs::path(rec.name), rec.fileNo, block.no, i);
    }
}

void Archiver::CreateJobs(const std::wstring &path, const uint64_t fileNo, const std::size_t indexBlockNo, 
    const std::size_t indexRecNo)
{
	std::ifstream is(path, std::ios::binary);
	uintmax_t bytesLeft = fs::file_size(path);
	uintmax_t bytesToRead = bytesLeft < CHUNK ? bytesLeft : CHUNK;
	bytesLeft -= bytesToRead;
	while (bytesToRead > 0)
    {
        // TODO use fixed size, fixed number of read buffers? Based on profiling not needed
        Job job{ ++jobsCreated, fileNo, indexBlockNo, indexRecNo };
		ReadChunk(is, job.inbuf, bytesToRead);
        bytesToRead = bytesLeft < CHUNK ? bytesLeft : CHUNK;
        bytesLeft -= bytesToRead;
        job.lastFileJob = bytesToRead == 0;
        EnqueueCompressorJob(job);
    }
}

void Archiver::ReadChunk(std::ifstream &is, std::vector<uint8_t> &buffer, uintmax_t bytes)
{
	buffer.resize(bytes);
	is.read(reinterpret_cast<char*>(&buffer[0]), bytes);
	assert(bytes == is.gcount()); // TODO change to run-time exception
	buffer.resize(is.gcount());
}

void Archiver::WriteJobFinished(const Job &job)
{
    auto &recs = indexBlocks[job.indexBlockNo]->records;
    auto &rec = recs[job.indexRecNo];
    rec.lastBlockNo = job.no;
    bytesCompressed += rec.length;

    // all files processed?
    bool done = std::all_of(recs.crbegin(), recs.crend(), [](const auto &r) {
        return (r.type != static_cast<short>(fs::file_type::regular)) ||
            (r.lastBlockNo != 0); });

    if (done)
        indexStream << *indexBlocks[job.indexBlockNo];
}
