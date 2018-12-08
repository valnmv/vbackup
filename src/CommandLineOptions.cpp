#include "pch.h"
#include "CommandLineOptions.h"

#include <iostream>

// Parse command line options, simplistic implementation
void CommandLineOptions::Parse(int argc, wchar_t *argv[])
{
	if (argc != 4 || (argc == 4 && argv[1] != L"a" && argv[1] != L"r"))
	{
        PrintHelp();
        exit(1);
	}

    std::wstring s(argv[1]);
    switch (s[0])
    {
    case L'a':
        command = Command::Archive;
        break;

    case L'r':
        command = Command::Restore;
    }

    source = argv[2];
    destination = argv[3];
}

void CommandLineOptions::PrintHelp()
{
    std::cout <<
        "Creates or restores a file archive.\n"
        "\n"
        "Usage: vbackup <command> <source> <destination>\n"
        "Commands:\n"
        "  a   Archive files from <source> volume to <destination> archive.\n"
        "  r   Restore files from <source> archive to <destination> volume.\n"
        "\n"
        "Source and destination:\n"
        "  When creating an archive <source> is the volume to make archive from,\n"
        "  <destination> is the name of the archive data file, with extension '.z'.\n"
        "  If not specified in the name, '.z' extension is added to <destination>.\n"
        "  The program creates also an index file with '.i' extension\n"
        "\n"
        "  When restoring an archive, <source> is the name of the archive file,\n"
        "  <destination> is the path to restore to. The index file must be in the same\n"
        "  folder where the archive data file is.\n"
        "\n";        
}