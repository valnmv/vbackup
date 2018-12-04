#include "pch.h"
#include "FileBlocks.h"
#include "Unarchiver.h"

#include <assert.h>
#include "zlib_intf.h"

void Unarchiver::Run(const std::wstring &src, const std::wstring &dest)
{
    source = src;
    destination = dest;
	sourceStream.open(source, std::ios::binary);
	std::ofstream os(dest, std::ios::binary);

	DataBlock block;
	while (ReadBlock(block))
	{
		DecompressChunk(block);
		os.write(reinterpret_cast<const char*>(&inflateBuffer[0]), block.origLength);
	}
}

bool Unarchiver::ReadBlock(DataBlock &block)
{
	if (sourceStream.eof() || EOF == sourceStream.peek())
		return false;

	block.read(sourceStream);
	return true;
}

int Unarchiver::DecompressChunk(const DataBlock &block)
{
	if (inflateBuffer.size() < block.origLength)
		inflateBuffer.resize(block.origLength);

	return inflate(block.data, inflateBuffer);
}

