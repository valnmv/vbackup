// BlockWriter - writes compressed data to disk
//

#pragma once

#include "Job.h"

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <future>
#include <queue>

struct CompareJob
{
    bool operator() (const Job &lhs, const Job &rhs) { return lhs.no > rhs.no; }
};

using SetOffsetFunction = std::function<void(std::size_t, std::size_t, uintmax_t)>;
using WriteJobFinishedFunction = std::function<void(const Job &)>;

class BlockWriter
{
private:
    std::ofstream stream; // compressed data stream
    std::priority_queue<Job, std::vector<Job>, CompareJob> queue; // job queue
    std::mutex queueMutex;
    std::condition_variable queueCond;
    std::atomic<uint64_t> jobsWriten = 0;
    std::atomic<uint64_t> fileNoWriting = 0;
	std::future<void> future;
    bool stop = false;
	SetOffsetFunction SetFileOffset; // store the compressed data offset in the index file
    WriteJobFinishedFunction WriteJobFinished; // update progress, save index block
    bool GetJob(Job &job);
public:
    void Start(const std::wstring &filename, const SetOffsetFunction &offsetFunc,
		const WriteJobFinishedFunction &jobFinishedFunction);
    void WriteLoop();
    void Enqueue(Job &job); // push writing job to the own queue
    void Complete(uint64_t jobCount);
};
