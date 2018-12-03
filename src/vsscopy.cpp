// Volume Shadow Copy Service facade
// 

#include "pch.h"
#include "VssCopy.h"

inline void throw_if_fail(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw _com_error(hr);
    }
}

VSS_ID VssCopy::CreateSnapshot(std::wstring volume)
{
    throw_if_fail(CreateVssBackupComponents(&backupComponents));
    throw_if_fail(backupComponents->InitializeForBackup());
    throw_if_fail(backupComponents->SetBackupState(FALSE, FALSE, VSS_BT_FULL));
    throw_if_fail(backupComponents->SetContext(VSS_CTX_FILE_SHARE_BACKUP));

    VSS_ID snapshotSetId;
    throw_if_fail(backupComponents->StartSnapshotSet(&snapshotSetId));

    VSS_ID snapshotId = GUID_NULL;
    throw_if_fail(backupComponents->AddToSnapshotSet(&volume[0], GUID_NULL, &snapshotId));

    CComPtr<IVssAsync> async;
    throw_if_fail(backupComponents->DoSnapshotSet(&async));
    throw_if_fail(async->Wait());

    return snapshotId;
}

bool VssCopy::CopySnapshotFile(const VSS_ID snapshotId, const WCHAR sourceFilePath[MAX_PATH],
    const WCHAR newFileLocation[MAX_PATH])
{
    VSS_SNAPSHOT_PROP snapshotProp;
    throw_if_fail(backupComponents->GetSnapshotProperties(snapshotId, &snapshotProp));

    TCHAR existingFileLocation[MAX_PATH];
    wcscpy_s(existingFileLocation, snapshotProp.m_pwszSnapshotDeviceObject);
    wcscat_s(existingFileLocation, sourceFilePath);
    //VssFreeSnapshotProperties(&snapshotProp);
    return CopyFile(existingFileLocation, newFileLocation, false);
}

// Not completed, this is part of an example how to enumerate shadow copies
void VssCopy::EnumSnapshots()
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
