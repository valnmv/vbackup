#pragma once

#include <string>
#include <filesystem>

enum class Command { Unknown, Archive, Restore };

class CommandLineOptions
{
public:
  Command command{ Command::Unknown };
  std::filesystem::path source;
  std::filesystem::path destination;
  void Parse(int argc, char *argv[]);
};

