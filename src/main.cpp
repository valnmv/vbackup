// vbackup.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "CommandLineOptions.h"
#include "Archiver.h"
#include "Unarchiver.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <type_traits>

// currently size_t is used in several places
static_assert(std::is_same<std::size_t, uint64_t>::value, "std::size_t must be uint64_t");

int wmain(int argc, wchar_t* argv[])
{
    try
    {
        CommandLineOptions options;
        options.Parse(argc, argv);

        switch (options.command)
        {
        case Command::Archive:
        {
            Archiver arc;
            arc.Run(options.source, options.destination);
            break;
        }

        case Command::Restore:
        {
            Unarchiver unarc;
            unarc.Run(options.source, options.destination);
            break;
        }
        }
    }
    catch (const std::exception &err)
    {
        std::cout << err.what();
    }
    catch (...)
    {
        std::cout << "Unknown error";
    }
}