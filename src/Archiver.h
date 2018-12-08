#pragma once

#include <string>

class Archiver
{
private:
    std::wstring MakeRootPath(const std::wstring &path);
public:
    void Run(const std::wstring &src, const std::wstring &dest);
};

