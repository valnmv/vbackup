#pragma once

#include <atomic>
#include <string>
#include <thread>
#include <vector>

struct Job
{
    uint64_t no = 0; // seq. job#
	uint64_t fileNo = 0;
	std::size_t indexBlockNo = 0; // index block#
	std::size_t indexRecNo = 0; // record# in index block
	std::vector<uint8_t> inbuf;
    std::vector<uint8_t> outbuf;
    bool lastFileJob = false;
};
