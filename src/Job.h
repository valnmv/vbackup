#pragma once

#include <atomic>
#include <string>
#include <thread>
#include <vector>

struct Job
{
    std::wstring file;
    uint64_t no = 0;
    bool lastJob = false;
    std::vector<uint8_t> inbuf; // TODO use fixed number of read buffers
    std::vector<uint8_t> outbuf; // TODO use fixed number of write buffers
    uint64_t outpos = 0;
};
