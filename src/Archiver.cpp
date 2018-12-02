// Archivator - create archive for a volume
//
// run path traversal to create file list
// add archive jobs to a queue
// read file data blocks
// write deflated data blocks

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

void Archiver::loadDirBlocks(const std::wstring &path)
{
  DirectoryBlock dirBlock;  
  IndexRecord header{};
  header.name = path;
  dirBlock.push_back(header);

  for (const auto &p : fs::directory_iterator(path))
  {
	IndexRecord rec{};
	rec.type = static_cast<short>(p.status().type());
	rec.name = p.path().filename();
	if (p.status().type() == fs::file_type::regular)
	  rec.length = fs::file_size(p);

	dirBlock.push_back(rec);
  }

  dirBlocks.push(dirBlock);

  for (const auto &r : dirBlock)
  {
	if (static_cast<fs::file_type>(r.type) == fs::file_type::directory)
	  loadDirBlocks(fs::path(header.name) / fs::path(r.name));
  }
}
