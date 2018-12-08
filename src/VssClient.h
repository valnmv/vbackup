#pragma once

#include <atlbase.h>

#include <Vss.h>
#include <VsWriter.h>
#include <VsBackup.h>

#include <string>

class VssClient
{
private:
    CComPtr<IVssBackupComponents> backupComponents;
    VSS_SNAPSHOT_PROP snapshotProp;
public:
    void CreateSnapshot(std::wstring volume);
    std::wstring GetSnapshotDeviceObject() { return snapshotProp.m_pwszSnapshotDeviceObject;  }
    void EnumSnapshots();
};