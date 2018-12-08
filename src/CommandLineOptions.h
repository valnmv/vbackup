#pragma once

#include <string>
#include <filesystem>

enum class Command { Unknown, Archive, Restore };

class CommandLineOptions
{
public:
    Command command{ Command::Unknown };
    std::wstring source;
    std::wstring destination;
    std::wstring path;
    void Parse(int argc, wchar_t *argv[]);
    void PrintHelp();
};

