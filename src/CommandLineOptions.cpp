#include "pch.h"
#include "CommandLineOptions.h"

#include <iostream>

// Parse command line options, simplistic implementation
void CommandLineOptions::Parse(int argc, wchar_t *argv[])
{
	if (argc != 4)
	{
        std::cout << 
            "Usage: vbackup <command> <source> <destination>\n"
            "Commands:\n"
            "  a   Archive files from <source> volume to <destination> archive.\n"
            "  r   Restore files from <source> archive to <destination> volume.\n";
        exit(1);
	}

    std::wstring s(argv[1]);
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
