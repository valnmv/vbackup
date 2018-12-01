// vbackup.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "VssCopy.h"
#include "CommandLineOptions.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
  // TODO Error handling, test unicode paths
  CommandLineOptions options;
  options.Parse(argc, argv);

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