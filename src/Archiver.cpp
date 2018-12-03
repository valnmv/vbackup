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

#include "zlib.h"
void zerr(int ret);
const int CHUNK = 512*1024;

const int compressorCount = 1;

void Archiver::Run(const std::wstring &src, const std::wstring &dest)
{
    source = src;
    destination = dest;
    //dataBlocks.resize(compressorCount);

    auto start = std::chrono::high_resolution_clock::now();

    std::future<void> writer = std::async(std::launch::async, &Archiver::Write, this);
    std::vector<std::future<void>> compressors;
    for (int i = 0; i < compressorCount; ++i)
    {
        compressors.push_back(std::async(std::launch::async, &Archiver::Compress, this));
    }

    ListFiles(source);

    // TODO notify on jobsWritten
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);
    writerQueueFinished.wait(lock, [this]() { return jobNo == jobsWriten; });
    done = true;
    compQueueHasData.notify_all();
    writerQueueHasData.notify_all();

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << elapsed.count();
}

namespace fs = std::experimental::filesystem;

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
        rec.name = p.path().filename();
        if (p.status().type() == fs::file_type::regular)
        {
            rec.length = fs::file_size(p);
        }

        indexBlock.push_back(rec);
    }

    indexQueue.push(indexBlock);
    ProcessIndexBlock(indexBlock);
}

void Archiver::ProcessIndexBlock(const IndexBlock &block)
{
    IndexRecord header = block[0];
    for (const auto &r : block)
    {
        if (static_cast<fs::file_type>(r.type) == fs::file_type::directory)
            ListFiles(fs::path(header.name) / fs::path(r.name));

        if (static_cast<fs::file_type>(r.type) == fs::file_type::regular)
        {
            CreateJobs(fs::path(header.name) / fs::path(r.name));
        }
    }
}

void Archiver::CreateJobs(const std::wstring &path)
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
        Job job;
        job.file = path;
        job.no = ++jobNo;
        job.inbuf.resize(bytesToRead);
        std::ifstream is(job.file, std::ios::binary);
        is.seekg(readpos, is.beg);
        is.read(reinterpret_cast<char*>(&job.inbuf[0]), bytesToRead);
        assert(bytesToRead == is.gcount()); // TODO change to run-time exception
        readpos += bytesToRead;
        job.inbuf.resize(is.gcount());
        job.lastJob = is.eof();
        bytesToRead = bytesLeft < CHUNK ? bytesLeft : CHUNK;
        bytesLeft -= bytesToRead;

        {
            std::lock_guard<std::mutex> lock(compQueueMutex);
            compQueue.push(std::move(job));
            // TODO wait the queue to have no more than N jobs
            //...
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
        {
            std::unique_lock<std::mutex> lock(compQueueMutex);
            compQueueHasData.wait(lock, [this] { return done || compQueue.size() > 0; });
            if (done)
                break;

            job = std::move(compQueue.front());
            compQueue.pop();
        }
        if (Z_OK != CompressChunk(job))
        {
            // TODO show error
        }
        {
            std::lock_guard<std::mutex> lock(writerQueueMutex);
            writerQueue.push(std::move(job));
            writerQueueHasData.notify_one();
        }
    } while (!done);
}

// writer thread
void Archiver::Write()
{
    while (!done)
    {
        Job job;
        // TODO MUST use priority queue by job#!
        {
            std::unique_lock<std::mutex> lock(writerQueueMutex);
            writerQueueHasData.wait(lock, [this] { return done || writerQueue.size() > 0; });
            if (done)
                break;

            job = std::move(writerQueue.front());
            writerQueue.pop();
        }

        std::ofstream os(destination, std::ios::binary | std::ios::app);
        os.write(reinterpret_cast<char*>(&job.no), sizeof(job.no));
        uint64_t len = job.outbuf.size();
        os.write(reinterpret_cast<char*>(&len), sizeof(len));
        os.write(reinterpret_cast<char*>(&job.outbuf[0]), len);
        os.close();

        ++jobsWriten;
        writerQueueFinished.notify_one();
    }
}

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int Archiver::CompressChunk(Job &job)
{
    int ret, flush;
    unsigned have;
    z_stream strm;

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, compressionLevel);
    if (ret != Z_OK)
        return ret;

    // TODO use fixed read buffers
    strm.avail_in = static_cast<uInt>(job.inbuf.size());
    //flush = last ? Z_FINISH : Z_NO_FLUSH;
    flush = Z_FINISH;
    strm.next_in = &job.inbuf[0];

    std::size_t prev_out_size = 0;
    job.outbuf.resize(CHUNK);
    strm.next_out = &job.outbuf[0];
    do {
        strm.avail_out = CHUNK;
        ret = deflate(&strm, flush);    /* no bad return value */
        assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
        have = CHUNK - strm.avail_out;
        
        if (strm.avail_out == 0)
        {
            prev_out_size += CHUNK;
            job.outbuf.resize(prev_out_size + CHUNK);
            strm.next_out = &job.outbuf[prev_out_size];
        }
    } while (strm.avail_out == 0);
    assert(strm.avail_in == 0);     /* all input will be used */

    job.outbuf.resize(prev_out_size + have);
    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}

/* report a zlib or i/o error */
void zerr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}