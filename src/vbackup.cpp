// vbackup.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "VssCopy.h"
#include "CommandLineOptions.h"
#include "Archiver.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

#include <zlib.h>
#include <fcntl.h>
#include <io.h>
#include <codecvt>

extern "C" int def(FILE *source, FILE *dest, int level);
extern "C" int inf(FILE *source, FILE *dest);

int wmain(int argc, wchar_t* argv[])
{
  // TODO streams - imbue utf8 / binary
  //std::wofstream os(L"data.dat", std::ios::binary);
  //os.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));

  // TODO Error handling
  CommandLineOptions options;
  options.Parse(argc, argv);

  Archiver arc;
  arc.loadDirBlocks(options.source);
  
  FILE *source, *dest;
  _wfopen_s(&source, options.source.c_str(), L"r");
  _wfopen_s(&dest, options.destination.c_str(), L"w");
  _setmode(_fileno(source), _O_BINARY);
  _setmode(_fileno(dest), _O_BINARY);
  
  int rv = (options.command == Command::Archive) ? def(source, dest, -1) :
	inf(source, dest);

  fclose(source);
  fclose(dest);

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