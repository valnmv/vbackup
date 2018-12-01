#pragma once

#include <atlbase.h>
#include <comdef.h>

#include <Vss.h>
#include <VsWriter.h>
#include <VsBackup.h>

#include <string>

class VSSCopy
{
public:
  VSS_ID CreateSnapshot(std::wstring volume);
  void EnumSnapshots();
  bool CopySnapshotFile(const VSS_ID snapshotId, const WCHAR sourceFilePath[MAX_PATH],
	const WCHAR newLocation[MAX_PATH]);

private:
  CComPtr<IVssBackupComponents> backupComponents;
};