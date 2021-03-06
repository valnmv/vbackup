#pragma once

#include "pch.h"

#include <iosfwd>
#include <string>
#include <vector>

// There are index files and data files, each of them comprised of consecutive blocks.
//
// Index files
// -----------
// An index file contains directory blocks, one for each directory. Each directory block 
// starts with a header record and contains up to a 1000 records for files and subdirectories.
//
// Index file = [directory-block] ...
// Directory block = [header record] [file or directory record] ...
// Record = [rec.type | length | file# | offset | name]
//
// <rec.type> 0=header, 1=file, 2=directory
// <length> for header - current dir.block size, for file - file length, for directory - 0
// <file#> and <offset>:
// - TODO: in a header record they point to a next directory block when there are more than 
//   a 1000 directory entries
// - in file records these fields define data file# and offset to the file data in a data file
// - in directory records they define index file# and position of directory data in index files
//
// Data files
// ----------
// Data file = [data-block] ...
// Data block = [block-no|orig-length|data-length|data...]
//
// A data file is made up of data blocks containing compressed file data. Each data block 
// starts with <block-no> and <data-length> fields and then compressed data from a file. 
// <block-no> is seq. block number for the file that is being compressed
// <length> is the length of the compressed data block
// <orig-length> is uncompressed data size
//

struct IndexRecord
{
    short type = 0; // 0=header, 1=file, 2=directory
    uint64_t length = 0; // file length for files, otherwise zero
    uint64_t fileNo = 0; // will be used when the archives are split in several volumes, e.g of 4GB
    uint64_t offset = 0; // offset of the first block in the compressed data file
    std::wstring name; // file / directory name
    uint64_t done = false; // not stored, used only during compression, to be removed later

    void print(std::wostream& os) const;
    void read(std::wistream& is);
};

struct IndexBlock
{	
	uint64_t no = 0;
	std::vector<IndexRecord> records;
	void AddRecord(IndexRecord &rec) { records.push_back(std::move(rec)); }

    void print(std::wostream& os) const;
    void read(std::wistream& is);
};

struct DataBlock
{
    uint64_t no = 0; // seq.block# = job#
    uint64_t origLength = 0; // number of data bytes before compression
    uint64_t length = 0; // number of data bytes in the block
    std::vector<uint8_t> data;

    void read(std::istream &is);
    void write(std::ostream &os);    
};

std::wostream& operator<<(std::wostream &os, const IndexRecord &rec);
std::wistream& operator>>(std::wistream &is, IndexRecord &rec);

std::wostream& operator<<(std::wostream &os, const IndexBlock &block);
std::wistream& operator>>(std::wistream &is, IndexBlock &rec);
