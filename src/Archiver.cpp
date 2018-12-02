// Archivator - create archive for a volume
//

#include "pch.h"
#include "Archiver.h"

#include <fstream>
#include <filesystem>
#include <queue>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <io.h>
#include "zlib.h"

#define CHUNK 16384
int def(std::ifstream &is, FILE *dest, int level, unsigned char in[16384],
  unsigned char out[16384]);

void Archiver::Run(const std::wstring &src, const std::wstring &dest)
{
  source = src;
  destination = dest;
  QueueIndexBlocks(source);
}

namespace fs = std::experimental::filesystem;

void Archiver::QueueIndexBlocks(const std::wstring &path)
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
	  QueueIndexBlocks(fs::path(header.name) / fs::path(r.name));

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
  Task t = taskQueue.front();
  taskQueue.pop();

  // jobs[t.jobNo].file
  t.bufferCompressed.resize(CHUNK);
  t.bufferDecompressed.resize(CHUNK);

  {
	std::ifstream is(jobs[t.jobNo].file, std::ios::binary);
	is.read(reinterpret_cast<char*>(&t.bufferDecompressed[0]), CHUNK);
	t.srcPos = is.gcount();

	FILE *destFile;
	_wfopen_s(&destFile, destination.c_str(), L"w");
	_setmode(_fileno(destFile), _O_BINARY);

	def(is, destFile, -1, &t.bufferDecompressed[0], &t.bufferCompressed[0]);
	fclose(destFile);
  }

  t.status = TaskStatus::Processed;
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
int def(std::ifstream &is, FILE *dest, int level, unsigned char in[CHUNK],
  unsigned char out[CHUNK])
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
	strm.avail_in = static_cast<uInt>(is.gcount());
	//strm.avail_in = fread(in, 1, CHUNK, source);
	//if (ferror(source)) {
	//  (void)deflateEnd(&strm);
	//  return Z_ERRNO;
	//}

	flush = is.eof() ? Z_FINISH : Z_NO_FLUSH;
	strm.next_in = in;

	/* run deflate() on input until output buffer not full, finish
	   compression if all of source has been read in */
	do {
	  strm.avail_out = CHUNK;
	  strm.next_out = out;
	  ret = deflate(&strm, flush);    /* no bad return value */
	  assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
	  have = CHUNK - strm.avail_out;
	  if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
		(void)deflateEnd(&strm);
		return Z_ERRNO;
	  }
	} while (strm.avail_out == 0);
	assert(strm.avail_in == 0);     /* all input will be used */

	/* done when last data in file processed */
  } while (flush != Z_FINISH);
  assert(ret == Z_STREAM_END);        /* stream will be complete */

  /* clean up and return */
  (void)deflateEnd(&strm);
  return Z_OK;
}