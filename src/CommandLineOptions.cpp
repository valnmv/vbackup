#include "pch.h"
#include "CommandLineOptions.h"

void CommandLineOptions::Parse(int argc, char *argv[])
{
  std::string s(argv[1]);
  switch (s[0])
  {
  case 'a':
	command = Command::Archive;
	break;

  case 'r':
	command = Command::Restore;
  }

  source = argv[2];
  destination = argv[3];
}
