#pragma once

#include <string>

class Unarchiver
{
private:
    std::wstring source;
    std::wstring destination;

public:
    Unarchiver();
    ~Unarchiver();
    void Run(const std::wstring & src, const std::wstring & dest);
};

