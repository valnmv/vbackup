#pragma once

#include <string>

class Archiver
{
private:
    int compressorCount = 1; // number of compress threads
public:
    void Run(const std::wstring &src, const std::wstring &dest);
};

