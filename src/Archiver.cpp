// Archivator - create archive for a volume
//

#include "pch.h"
#include "Archiver.h"

#include <iostream>

#include <cassert>
#include <fstream>
#include <filesystem>
#include <future>
#include <queue>
#include <thread>

#include "zlib_intf.h"

namespace fs = std::experimental::filesystem;
const int compressorCount = 3;
DataBlock DataBlockFromJob(const Job &job);

DataBlock DataBlockFromJob(const Job &job)
{
    DataBlock block{ job.no, job.inbuf.size(), job.outbuf.size() };
	block.data = std::move(job.outbuf);
	return std::move(block);
}

void Archiver::Run(const std::wstring &src, const std::wstring &dest)
{
    source = src;
    destination = dest;

    auto start = std::chrono::high_resolution_clock::now();
	destinationStream.open(destination, std::ios::binary | std::ios::app);

    auto writer = std::async(std::launch::async, &Archiver::Write, this);
    std::vector<std::future<void>> compressors;
    for (int i = 0; i < compressorCount; ++i)
    {
        compressors.push_back(std::async(std::launch::async, &Archiver::Compress, this));
    }

    ListFiles(source);

    std::mutex endMutex;
    std::unique_lock<std::mutex> lock(endMutex);
    writerQueueFinished.wait(lock, [this]() { return jobsCreated == jobsWriten; });
    done = true;
    compQueueHasData.notify_all();
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

        {
            std::unique_lock<std::mutex> lock(compQueueMutex);
            compQueueHasData.wait(lock, [this] { return done || 
                compQueue.size() < compressorCount * 2; });

            compQueue.push(std::move(job));
        }
        compQueueHasData.notify_one();
    }
}

// compressor thread
void Archiver::Compress()
{
    do
    {
        Job job;
        if (!GetCompressJob(job))
            break;

        compQueueHasData.notify_all();
        if (0 != CompressChunk(job))
        {
            // TODO show error
        }
        {
            std::lock_guard<std::mutex> lock(writerQueueMutex);
            writerQueue.push(std::move(job));
        }
        writerQueueHasData.notify_one();
    } while (!done);
}

bool Archiver::GetCompressJob(Job &job)
{
    std::unique_lock<std::mutex> lock(compQueueMutex);
    compQueueHasData.wait(lock, [this] { return done || compQueue.size() > 0; });
    if (done)
        return false;

    job = std::move(compQueue.front());
    compQueue.pop();
    return true;
}

// writer thread
void Archiver::Write()
{
    Job job;
    while (!done)
    {
        if (!GetWriteJob(job))
            break;

		if (job.fileNo != fileNoWriting)
		{
			fileNoWriting = job.fileNo;
			uint64_t pos = destinationStream.tellp();
			indexBlocks[job.indexBlockNo][job.indexRecNo].offset = pos;
		}

		DataBlock block = DataBlockFromJob(job);
        block.write(destinationStream);
        ++jobsWriten;
        writerQueueFinished.notify_one();
    }
}

bool Archiver::GetWriteJob(Job &job)
{
    std::unique_lock<std::mutex> lock(writerQueueMutex);
    writerQueueHasData.wait(lock, [this] { return done ||
        (writerQueue.size() > 0 && (jobsWriten + 1 == writerQueue.top().no)); });

    if (done)
        return false;

    job = std::move(writerQueue.top());
    writerQueue.pop();
    return true;
}

int Archiver::CompressChunk(Job &job)
{
	return deflate(compressionLevel, job.inbuf, job.outbuf);
}
