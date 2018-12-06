#include "pch.h"

#include "zlib_intf.h"
#include "zlib.h"

#include <vector>
#include <assert.h>
#include <stdio.h>

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int deflate(int compressionLevel, std::vector<uint8_t> &inbuf, std::vector<uint8_t> &outbuf)
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

	strm.avail_in = static_cast<uInt>(inbuf.size());
	//flush = last ? Z_FINISH : Z_NO_FLUSH;
	flush = Z_FINISH;
	strm.next_in = &inbuf[0];

	std::size_t prev_out_size = 0;
	outbuf.resize(CHUNK);
	strm.next_out = &outbuf[0];
	do {
		strm.avail_out = CHUNK;
		ret = deflate(&strm, flush);    /* no bad return value */
		assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
		have = CHUNK - strm.avail_out;

		if (strm.avail_out == 0)
		{
			prev_out_size += CHUNK;
			outbuf.resize(prev_out_size + CHUNK);
			strm.next_out = &outbuf[prev_out_size];
		}
	} while (strm.avail_out == 0);
	assert(strm.avail_in == 0);     /* all input will be used */

	outbuf.resize(prev_out_size + have);
	/* clean up and return */
	(void)deflateEnd(&strm);
	return Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int inflate(const std::vector<uint8_t> &inbuf, std::vector<uint8_t> &outbuf)
{
	int ret;
	unsigned have;
	z_stream strm;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return ret;

	do {
		strm.avail_in = static_cast<uInt>(inbuf.size());
		if (strm.avail_in == 0)
			break;

		strm.next_in = const_cast<Byte*>(&inbuf[0]);
		do {
			strm.avail_out = CHUNK;
			strm.next_out = &outbuf[0];
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;     /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
			}
			have = CHUNK - strm.avail_out;
			// TODO handle output buffer flush?
		} while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerr(int ret)
{
	fputs("zlib: ", stderr);
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
