#include "pch.h"
#include "Unarchiver.h"

#include <fcntl.h>
#include <io.h>
extern "C" int inf(FILE *source, FILE *dest);

Unarchiver::Unarchiver()
{
}

Unarchiver::~Unarchiver()
{
}

void Unarchiver::Run(const std::wstring &src, const std::wstring &dest)
{
  source = src;
  destination = dest;
  // ---

  FILE *sourceFile, *destFile;
  _wfopen_s(&sourceFile, source.c_str(), L"r");
  _wfopen_s(&destFile, destination.c_str(), L"w");
  _setmode(_fileno(sourceFile), _O_BINARY);
  _setmode(_fileno(destFile), _O_BINARY);
  inf(sourceFile, destFile);
  fclose(sourceFile);
  fclose(destFile);
}
