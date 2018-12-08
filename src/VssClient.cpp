// Volume Shadow Copy Service facade
// 

#include "pch.h"
#include "VssClient.h"
#include "util.h"

#include <string>
#include <iostream>

// Create a non-persistent snapshot
void VssClient::CreateSnapshot(std::wstring path)
{
    throw_if_fail(CreateVssBackupComponents(&backupComponents));
    throw_if_fail(backupComponents->InitializeForBackup());
    throw_if_fail(backupComponents->SetBackupState(FALSE, FALSE, VSS_BT_FULL));
    throw_if_fail(backupComponents->SetContext(VSS_CTX_FILE_SHARE_BACKUP));

    VSS_ID snapshotSetId;
    throw_if_fail(backupComponents->StartSnapshotSet(&snapshotSetId));
    
    VSS_ID snapshotId = GUID_NULL;
    throw_if_fail(backupComponents->AddToSnapshotSet(&path[0], GUID_NULL, &snapshotId));

    CComPtr<IVssAsync> async;
    throw_if_fail(backupComponents->DoSnapshotSet(&async));
    throw_if_fail(async->Wait());

    throw_if_fail(backupComponents->GetSnapshotProperties(snapshotId, &snapshotProp));
}

bool VssClient::CopySnapshotFile(const std::wstring &sourcePath,
    const std::wstring &newPath)
{
    std::wstring fileLocation = snapshotProp.m_pwszSnapshotDeviceObject + sourcePath;
	check_last_error(CopyFile(fileLocation.c_str(), newPath.c_str(), false));
	return true;
}

// Not completed, this is part of an example how to enumerate shadow copies
void VssClient::EnumSnapshots()
{
    CComPtr<IVssEnumObject> pIEnumSnapshots;

    throw_if_fail(CreateVssBackupComponents(&backupComponents));
    throw_if_fail(backupComponents->InitializeForBackup());
    throw_if_fail(backupComponents->SetContext(VSS_CTX_ALL));
    throw_if_fail(backupComponents->Query(GUID_NULL, VSS_OBJECT_NONE, VSS_OBJECT_SNAPSHOT, &pIEnumSnapshots));

    // Enumerate all shadow copies.
    while (true)
    {
        VSS_OBJECT_PROP objectProp;

        ULONG ulFetched;
        throw_if_fail(pIEnumSnapshots->Next(1, &objectProp, &ulFetched));
        if (ulFetched == 0)
            break;

        VSS_SNAPSHOT_PROP& snapshotProp = objectProp.Obj.Snap;
        snapshotProp;
        //...
    }
}
