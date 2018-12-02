#pragma once

#include "pch.h"

#include <iosfwd>
#include <string>
#include <vector>

// There are index files and data files, each of them comprised of consecutive blocks.
//
// Index files
// -----------
// Index file = [directory-block] ...
// Directory block = [header record] [file or directory record] ...
// Directory block record = [<rec.type> <length> <file#> <position> <name>]
//
// An index file contains directory blocks, one for each directory. Each directory block 
// starts with a header record and contains up to a 1000 records for files and subdirectories.
//
// Directory block record = [rec.type|length|name|file#|position]
// <rec.type> H=header, F=file, D=directory
// <length> for header - current dir.block size, for file - file length, for directory - 0
// <file#> and <position>:
// - in header record they point to a next directory block when there are more than 
//   a 1000 directory entries
// - in file records these fields define file# and position to the file data in data file
// - in directory records they define file# and position of directory data in index files
//
// Data files
// ----------
// Data file = [data-block] ...
// Data block = [data-length|next-block-position|data...]
//
// A data file is made up of data blocks containing compressed file data. Each data block 
// starts with <length> and <position> fields and then compressed data from a file. 
// <length> is the length of the data block, <position> is the offset from start of the 
// next data block for the same file, or zero if there are no more data blocks for this file


struct IndexRecord
{
  short type; // 0=header, 1=file, 2=directory
  uint64_t length;
  short fileNo;
  uint64_t position;
  std::wstring name;

  void print(std::wostream& os) const;
  void read(std::wistream& is);
};

using DirectoryBlock = std::vector<IndexRecord>;

struct DataBlock
{
  uint64_t length;
  uint64_t position;
  std::vector<uint8_t> data;
  std::vector<uint8_t> dataCompressed;
};

std::wostream& operator<<(std::wostream &os, const IndexRecord &rec);
std::wistream& operator>>(std::wistream &is, IndexRecord &rec);
