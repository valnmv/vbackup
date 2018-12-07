#pragma once

#include <atlbase.h>
#include <comdef.h>

#include <Vss.h>
#include <VsWriter.h>
#include <VsBackup.h>

#include <string>

class VssCopy
{
public:
    VSS_ID CreateSnapshot(std::wstring volume);
    void EnumSnapshots();
    bool CopySnapshotFile(const VSS_ID snapshotId, const std::wstring &sourcePath, const std::wstring &newPath);

private:
    CComPtr<IVssBackupComponents> backupComponents;
};