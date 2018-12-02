#pragma once

#include "FileBlocks.h"
#include "Job.h"

#include <atomic>
#include <map>
#include <queue>

class Archiver
{
private:
  std::wstring source;
  std::wstring destination;
  
  std::atomic<uint64_t> jobNo;
  std::map<uint64_t, Job> jobs;
  std::queue<Task> taskQueue;

  std::queue<IndexBlock> indexQueue;
  std::queue<DataBlock> dataQueue;

  void ListFiles(const std::wstring &path);
  void ProcessIndexBlock(const IndexBlock &block);
  void CreateTask(Job &job);
  void ProcessTask();
  void StartThreads();
  int Compress(Task & task, bool last, int level);
public:
  void Run(const std::wstring &src, const std::wstring &dest);
};