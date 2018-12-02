#pragma once

#include <string>
#include <filesystem>

enum class Command { Unknown, Archive, Restore };

class CommandLineOptions
{
public:
  Command command{ Command::Unknown };
  //std::filesystem::path source;
  //std::filesystem::path destination;
  std::wstring source;
  std::wstring destination;
  void Parse(int argc, wchar_t *argv[]);
};

