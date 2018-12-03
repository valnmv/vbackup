// Archivator - create archive for a volume
//

#include "pch.h"
#include "Archiver.h"

#include <cassert>
#include <fstream>
#include <filesystem>
#include <queue>

#include "zlib.h"
void zerr(int ret);
const int CHUNK = 128*1024;

void Archiver::Run(const std::wstring &src, const std::wstring &dest)
{
    source = src;
    destination = dest;
    ListFiles(source);
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
    uintmax_t bytesLeft = fs::file_size(path);
    Job job;
    job.file = path;
    job.no = ++jobNo;

    uintmax_t bytesToRead = bytesLeft < CHUNK ? bytesLeft : CHUNK;
    bytesLeft -= bytesToRead;
    while (bytesToRead > 0)
    {
        // TODO use fixed number of read buffers!
        job.inbuf.resize(bytesToRead);
        std::ifstream is(job.file, std::ios::binary);
        is.seekg(job.inpos, is.beg);
        is.read(reinterpret_cast<char*>(&job.inbuf[0]), bytesToRead);
        assert(bytesToRead == is.gcount()); // TODO change to run-time exception
        job.inbuf.resize(is.gcount());
        job.lastJob = is.eof();
        bytesToRead = bytesLeft < CHUNK ? bytesLeft : CHUNK;
        bytesLeft -= bytesToRead;

        // TODO sync
        jobQueue.push(job);
        // TODO wait the queue to have no more than N jobs...
    }

    ProcessJob();
}

void Archiver::ProcessJob()
{
    // TODO sync
    Job job = jobQueue.front();
    jobQueue.pop();

    if (Z_OK != CompressChunk(job))
    {
        // TODO handle error
    }

    writerQueue.push(job);

    // writer thread
    {
        Job job = writerQueue.front();
        writerQueue.pop();
        std::ofstream os(destination, std::ios::binary);
        os.write(reinterpret_cast<char*>(&job.no), sizeof(job.no));
        uint64_t len = job.outbuf.size();
        os.write(reinterpret_cast<char*>(len), sizeof(len));
        os.write(reinterpret_cast<char*>(&job.outbuf[0]), len);
        os.close();
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

    do {
        std::size_t prev_out_size = job.outbuf.size();
        job.outbuf.resize(prev_out_size + CHUNK);
        strm.avail_out = CHUNK;
        strm.next_out = &job.outbuf[prev_out_size];
        ret = deflate(&strm, flush);    /* no bad return value */
        assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
        have = CHUNK - strm.avail_out;
    } while (strm.avail_out == 0);
    assert(strm.avail_in == 0);     /* all input will be used */

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