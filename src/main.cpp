// vbackup.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "VssClient.h"
#include "CommandLineOptions.h"
#include "Archiver.h"
#include "Unarchiver.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

int wmain(int argc, wchar_t* argv[])
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

	//CoInitialize(NULL);
	//try
	//{
	//	VssCopy vssClient;
	//	// Make sure volume name has a trailing backslash!
	//	VSS_ID snapshotId = vssClient.CreateSnapshot(L"C:\\");
	//	vssClient.CopySnapshotFile(snapshotId, L"\\Users\\valyo\\NTUSER.DAT", L"c:/NTUSER-copy.DAT");
	//	//vss.RestoreSnapshot();
	//}
	//catch (const _com_error &err)
	//{
	//	std::wstring error = err.ErrorMessage();
	//	std::wcout << error;
	//}

	//CoUninitialize();
}