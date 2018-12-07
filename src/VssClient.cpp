// Volume Shadow Copy Service facade
// 

#include "pch.h"
#include "VssClient.h"

#include <string>
#include <iostream>

inline void throw_if_fail(HRESULT hr)
{
    if (FAILED(hr))
    {
        throw _com_error(hr);
    }
}

std::string GetLastErrorText(DWORD errorCode)
{
	PCHAR pwszBuffer = NULL;
	DWORD dwRet = ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&pwszBuffer, 0, NULL);

	if (dwRet == 0)
		return std::string("<Unknown error code>");

	std::string errorText(pwszBuffer);
	LocalFree(pwszBuffer);
	return errorText;
}

inline void check_last_error(bool result)
{
	if (!result)
	{
		DWORD errorCode = GetLastError();		
		throw std::runtime_error(GetLastErrorText(errorCode));
	}
}

VSS_ID VssCopy::CreateSnapshot(std::wstring path)
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

    return snapshotId;
}

bool VssCopy::CopySnapshotFile(const VSS_ID snapshotId, const std::wstring &sourcePath,
    const std::wstring &newPath)
{
    VSS_SNAPSHOT_PROP snapshotProp;
    throw_if_fail(backupComponents->GetSnapshotProperties(snapshotId, &snapshotProp));

    std::wstring fileLocation = snapshotProp.m_pwszSnapshotDeviceObject + sourcePath;
    //VssFreeSnapshotProperties(&snapshotProp);
    std::wstring s;
	check_last_error(CopyFile(fileLocation.c_str(), newPath.c_str(), false));
	return true;
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
