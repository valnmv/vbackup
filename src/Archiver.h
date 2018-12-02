#pragma once

#include "FileBlocks.h"
#include <queue>

using DirBlockQueue = std::queue<DirectoryBlock>;
using DataBlockQueue = std::queue<DataBlock>;

class Archiver
{
private:
  DirBlockQueue dirBlocks;
  DataBlockQueue dataBlocks;

public:
  void loadDirBlocks(const std::wstring &path);
  Archiver();
  ~Archiver();
};