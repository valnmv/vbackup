#include "pch.h"
#include "CommandLineOptions.h"

#include <iostream>

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
        "  When creating an archive <source> is the volume to archive,\n"
        "  <destination> is the archive file name. The program creates an index\n"
        "  file <destination>.i\n"
        "\n"
        "  When restoring an archive, <source> is the name of the archive file,\n"
        "  <destination> is the volume to restore to. The index file must be in\n"
        "  the same folder where the archive data file is.\n"
        "\n"
        "Note:\n"
        "  For debugging and testing purposes the program accepts an optional\n"
        "  paths to the source volume on archiving or to the destination volume\n"
        "  when restoring. This feature enables testing archive/restore with\n"
        "  non-root path. E.g. make an archive of C:\\test to D:\\archive.z use:\n"
        "\n"
        "    vbackup a C:\\test D:\\archive.z \n"
        "\n"
        "  To restore to C:\\test use:\n"
        "\n"
        "    vbackup r D:\\archive.z C:\\test\n"
        "\n";
}

// Parse command line options, simple implementation
void CommandLineOptions::Parse(int argc, wchar_t *argv[])
{
    bool goodArgs = (argc == 4);

    if (goodArgs)
    {
        switch (argv[1][0])
        {
        case L'a':
            command = Command::Archive;
            break;

        case L'r':
            command = Command::Restore;
            break;

        default:
            goodArgs = false;
        }
    }

    if (!goodArgs)
    {
        PrintHelp();
        exit(1);
    }

    source = argv[2];
    destination = argv[3];
}