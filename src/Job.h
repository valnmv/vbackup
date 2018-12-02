#pragma once

#include <string>
#include <queue>
#include <vector>

class Task;

class Job
{
public:
  uint64_t no;
  Job(const std::wstring &file_) : file(file_) {}
  ~Job() {}

  const std::wstring file;
  std::queue<Task> taskQueue;
};

enum class TaskStatus { Pending, InProgress, Done };

class Task
{
public:
  uint64_t no;
  std::vector<uint8_t> bufferCompressed;
  std::vector<uint8_t> bufferDecompressed;
  TaskStatus status;
};



