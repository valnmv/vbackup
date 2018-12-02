#pragma once

#include <atomic>
#include <string>
#include <queue>
#include <thread>
#include <vector>

enum class TaskStatus { Pending, Processed, Done };

class Task
{
public:
  uint64_t jobNo;
  uint64_t no;
  std::thread::id threadNo;
  std::vector<uint8_t> bufferCompressed;
  std::vector<uint8_t> bufferDecompressed;
  uint64_t srcPos;
  uint64_t destPos;
  uint32_t srcBufLength;
  uint32_t destBufLength;
  TaskStatus status;
};

struct Job
{
  uint64_t no;
  std::wstring file;
  uint32_t taskCount;
  bool done;
};
