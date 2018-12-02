#pragma once

#include "FileBlocks.h"
#include <queue>

using IndexBlockQueue = std::queue<IndexBlock>;
using DataBlockQueue = std::queue<DataBlock>;

class Archiver
{
private:
  IndexBlockQueue indexBlocks;
  DataBlockQueue dataBlocks;

public:
  void QueueIndexBlocks(const std::wstring &path);
  void ProcessIndexBlock(IndexBlock &block);
  Archiver();
  ~Archiver();
};