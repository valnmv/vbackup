#pragma once

#include "pch.h"

#include <string>
#include <vector>

// There are index files and data files, each of them comprised of consecutive blocks.
//
// An index file contains directory blocks. A directory block consists of index records.
// An index record is either header, file or directory record.
// The directory block starts with a header index record and contains up to a 1000 records.
//
// <length> in header record is equal to the directory block size.
//
// <fileNo> and <position>:
// - for file records these fields define file# and position to the file data in data file
// - for directory records they define file# and position of directory data in index files
// - for the header record they point to next directory block when there are more than 
//   a 1000 directory entries.
//
// A data file is made up of data blocks, where each data block starts with <length> and <position>
// fields and contains then compressed data from a file. <length> is the length of the data block,
// <position> is the offset from start of next data block for the same file, or zero if there are 
// no more data blocks.

struct IndexRecord
{
  char type; // H-header, F-file, D-directory
  uint64_t length; // for header - block size, for file - length, for directory - 0
  short nameLength;
  std::wstring name;
  short fileNo; // for header - fileNo and position of next directory block for the same directory
  uint64_t position;
};

using DirectoryBlock = std::vector<IndexRecord>;
