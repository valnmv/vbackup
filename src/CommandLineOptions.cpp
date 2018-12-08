#include "pch.h"
#include "CommandLineOptions.h"

#include <iostream>

// Parse command line options, simplistic implementation
void CommandLineOptions::Parse(int argc, wchar_t *argv[])
{
    bool goodArgs = (argc == 4);
    std::wstring cmd;

    if (goodArgs)
    {
        cmd = argv[1];
        goodArgs = (cmd == L"a" || cmd == L"r");
    }

    if (!goodArgs)
    {
        PrintHelp();
        exit(1);
	}

    switch (cmd[0])
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
        "Usage: vbackup <command> <source> <destination> [path]\n"
        "Commands:\n"
        "  a   Archive files from <source> volume to <destination> archive.\n"
        "  r   Restore files from <source> archive to <destination> volume.\n"
        "\n"
        "Source and Destination:\n"
        "  When creating an archive <source> is the volume to make archive from,\n"
        "  <destination> is the name of the archive data file, with extension '.z'.\n"
        "  If not specified in the name, '.z' extension is added to <destination>.\n"
        "  The program creates also an index file with '.i' extension\n"
        "\n"
        "  When restoring an archive, <source> is the name of the archive file,\n"
        "  <destination> is the volume to restore to. The index file must be in the same\n"
        "  folder where the archive data file is.\n"
        "\n"
        "Path:\n"
        "  For debugging and testing purposes the program accepts an optional additional\n"
        "  path argument, which is concatenated to the source volume on archiving or to the\n"
        "  destination volume when restoring. This feature enables testing archive/restore\n"
        "  with non-root path\n"
        "\n";
}