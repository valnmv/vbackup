#pragma once

#include <string>

class Unarchiver
{
private:
    std::wstring source;
    std::wstring destination;

    void ReadBlock();
    void Decompress();

public:
    void Run(const std::wstring & src, const std::wstring & dest);
};

