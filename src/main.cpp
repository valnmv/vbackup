// vbackup.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "VssCopy.h"
#include "CommandLineOptions.h"
#include "Archiver.h"
#include "Unarchiver.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

int wmain(int argc, wchar_t* argv[])
{
    // TODO streams - imbue utf8 / binary
    //std::wofstream os(L"data.dat", std::ios::binary);
    //os.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

    // TODO Error handling
    CommandLineOptions options;
    options.Parse(argc, argv);

    switch (options.command)
    {
    case Command::Archive:
    {
        Archiver arc;
        arc.Run(options.source, options.destination);
    }
    break;

    case Command::Restore:
    {
        Unarchiver unarc;
        unarc.Run(options.source, options.destination);
    }
    break;
    }


    // CoInitialize(NULL);
    // try
    // {
       //VssCopy vss;
       //// Make sure volume name has a trailing backslash!
       //VSS_ID snapshotId = vss.CreateSnapshot(L"C:\\");
       //vss.CopySnapshotFile(snapshotId, L"\\Users\\Valyo\\NTUSER.DAT", L"c:/Users/valyo/NTUSER-copy.DAT");
       ////vss.RestoreSnapshot();
    // }
    // catch (const _com_error &err)
    // {
       //std::wstring error = err.ErrorMessage();
       //std::wcout << error;
    // }

    // CoUninitialize();

}