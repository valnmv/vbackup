// Archivator - create archive for a volume
//

#include "pch.h"
#include "Archiver.h"
#include "zlib_intf.h" // for CHUNK

#include <iostream>

#include <cassert>
#include <filesystem>
#include <future>
#include <thread>

const int compressorCount = 3;

namespace fs = std::experimental::filesystem;

void Archiver::Run(const std::wstring &src, const std::wstring &dest)
{
    auto start = std::chrono::high_resolution_clock::now();

    source = src;
    destination = dest;

    writer.Init(destination, &indexBlocks);
    auto fu = std::async(std::launch::async, &BlockWriter::WriteLoop, &writer);

    compressor.StartThreads(&writer, compressorCount);

    ListFiles(source);
    writer.Complete(jobsCreated);
    done = true;
    compressor.Complete();
    //writerQueueHasData.notify_all();

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << elapsed.count();
}

void Archiver::ListFiles(const std::wstring &path)
{
    IndexBlock indexBlock;
    IndexRecord header{};
    header.name = path;
    indexBlock.push_back(header);

    for (const auto &p : fs::directory_iterator(path))
    {
        IndexRecord rec{};
        rec.type = static_cast<short>(p.status().type());
        if (p.status().type() == fs::file_type::regular)
        {
            rec.length = fs::file_size(p);
        }
        rec.name = p.path().filename();
        rec.fileNo = ++filesIndexed;
        indexBlock.push_back(rec);
    }

    indexBlocks.push_back(indexBlock);
    ProcessIndexBlock(indexBlock);
}

void Archiver::ProcessIndexBlock(const IndexBlock &block)
{
    IndexRecord header = block[0];
	for (std::size_t i = 1; i < block.size(); i++)
    {
		const auto &rec = block[i];
        if (static_cast<fs::file_type>(rec.type) == fs::file_type::directory)
            ListFiles(fs::path(header.name) / fs::path(rec.name));

        if (static_cast<fs::file_type>(rec.type) == fs::file_type::regular)
        {
			// TODO index block#           
            CreateJobs(fs::path(header.name) / fs::path(rec.name), rec.fileNo, 0, i);
        }
    }
}

void Archiver::CreateJobs(const std::wstring &path, const uint64_t fileNo, const std::size_t indexBlockNo, 
    const std::size_t indexRecNo)
{
    if (path == destination)
        return;

    uintmax_t bytesLeft = fs::file_size(path);
    uintmax_t bytesToRead = bytesLeft < CHUNK ? bytesLeft : CHUNK;
    bytesLeft -= bytesToRead;
    uintmax_t readpos = 0;
    while (bytesToRead > 0)
    {
        // TODO use fixed number of read buffers (with fixes sizes?)
        Job job{ ++jobsCreated, fileNo, indexBlockNo, indexRecNo };
        job.inbuf.resize(bytesToRead);
        std::ifstream is(path, std::ios::binary);
        is.seekg(readpos, is.beg);
        is.read(reinterpret_cast<char*>(&job.inbuf[0]), bytesToRead);
        assert(bytesToRead == is.gcount()); // TODO change to run-time exception
        readpos += bytesToRead;
        job.inbuf.resize(is.gcount());
        bytesToRead = bytesLeft < CHUNK ? bytesLeft : CHUNK;
        bytesLeft -= bytesToRead;

        compressor.PushJob(job);
    }
}
