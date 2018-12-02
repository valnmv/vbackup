// Archivator - create archive for a volume
//

#include "pch.h"
#include "Archiver.h"

#include <queue>
#include <filesystem>

Archiver::Archiver()
{
}

Archiver::~Archiver()
{
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

  indexBlocks.push(indexBlock);
}

void Archiver::ProcessIndexBlock(IndexBlock &block)
{
  IndexRecord header = block[0];
  for (const auto &r : block)
  {
	if (static_cast<fs::file_type>(r.type) == fs::file_type::directory)
	  QueueIndexBlocks(fs::path(header.name) / fs::path(r.name));

	if (static_cast<fs::file_type>(r.type) == fs::file_type::regular)
	{
	  DataBlock data{};
	}
  }
}
