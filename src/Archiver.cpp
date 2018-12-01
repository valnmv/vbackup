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


std::queue<std::filesystem::path> fileQueue;

Archiver::Archiver()
{
}


Archiver::~Archiver()
{
}
