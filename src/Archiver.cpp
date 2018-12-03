// Archivator - create archive for a volume
//

#include "pch.h"
#include "Archiver.h"

#include <cassert>
#include <fstream>
#include <filesystem>
#include <queue>

#include "zlib.h"

const int CHUNK = 32768;

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
            Job job{ ++jobNo, fs::path(header.name) / fs::path(r.name) };
            jobs[jobNo] = job;
            CreateTask(job);
            ProcessTask();
        }
    }
}

void Archiver::CreateTask(Job &job)
{
    // TODO lock
    Task t{ job.no, job.taskCount };
    ++job.taskCount;
    taskQueue.push(t);
}

void Archiver::ProcessTask()
{
    Task task = taskQueue.front();
    taskQueue.pop();

    // jobs[t.jobNo].file
    task.bufferCompressed.resize(CHUNK);
    task.bufferDecompressed.resize(CHUNK);

    std::ifstream is(jobs[task.jobNo].file, std::ios::binary);
    is.read(reinterpret_cast<char*>(&task.bufferDecompressed[0]), CHUNK);
    task.srcDataLength = is.gcount();
    task.srcPos = is.gcount();
    bool lastTask = is.eof();
    // TODO sync
    if (lastTask)
        jobs[task.jobNo].lastTask = true;

    if (Z_OK != Compress(task, lastTask, -1))
    {
        // TODO handle error
    }

    task.status = TaskStatus::Processed;

    // writer thread
    DataBlock data = dataQueue.front();
    dataQueue.pop();
    std::ofstream os(destination, std::ios::binary);
    os.write(reinterpret_cast<char*>(&data.data[0]), data.length);
    os.close();
}

void Archiver::StartThreads()
{
}

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int Archiver::Compress(Task &task, bool last, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    /* compress until end of file */
    do {
        strm.avail_in = static_cast<uInt>(task.srcDataLength);
        flush = last ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = &task.bufferDecompressed[0];

        do {
            strm.avail_out = CHUNK;
            strm.next_out = &task.bufferCompressed[0];
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;

            DataBlock block;
            block.no = task.no;
            block.length = have;
            block.data.reserve(have);
            std::copy(task.bufferCompressed.begin(), task.bufferCompressed.begin() + have,
                back_inserter(block.data));
            dataQueue.push(block);
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

    } while (flush != Z_FINISH);

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}