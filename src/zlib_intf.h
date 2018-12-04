#pragma once

#include <vector>

const int CHUNK = 512 * 1024;

int deflate(int compressionLevel, std::vector<uint8_t> &inbuf, std::vector<uint8_t> &outbuf);
int inflate(const std::vector<uint8_t> &inbuf, std::vector<uint8_t> &outbuf);
void zerr(int ret);


