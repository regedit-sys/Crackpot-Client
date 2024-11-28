#include "stdafx.h"
#include "io.h"

HRESULT CreateSymbolicLink(const char *szDrive, const std::string szDeviceName, BOOL System) {
	CHAR szDestinationDrive[MAX_PATH];
	ANSI_STRING linkname, devicename;
	sprintf_s(szDestinationDrive, MAX_PATH, System ? "\\System??\\%s" : "\\??\\%s", szDrive);
	RtlInitAnsiString(&linkname, szDestinationDrive);
	RtlInitAnsiString(&devicename, szDeviceName.c_str());

	if(DirectoryExists(szDrive)) return S_OK;

	NTSTATUS status = ObCreateSymbolicLink(&linkname, &devicename);
	return (status >= 0) ? S_OK : S_FALSE;
}

HRESULT deleteSymbolicLink(const char *szDrive, BOOL System) {
	CHAR szDestinationDrive[MAX_PATH];
	ANSI_STRING linkname;
	sprintf_s(szDestinationDrive, MAX_PATH, System ? "\\System??\\%s" : "\\??\\%s", szDrive);
	RtlInitAnsiString(&linkname, szDestinationDrive);
	NTSTATUS status = ObDeleteSymbolicLink(&linkname);
	return (status >= 0) ? S_OK : S_FALSE;
}

BOOL cReadFile(const char * FileName, MemoryBuffer &pBuffer) {
	HANDLE hFile; DWORD dwFileSize, dwNumberOfBytesRead;
	hFile = Native::CreateFileEx(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) {
		debug("[WRN] CReadFile - CreateFile failed %s", FileName);
		return FALSE;
	}
	dwFileSize = GetFileSize(hFile, NULL);
	PBYTE lpBuffer = (BYTE*)Native::Malloc(dwFileSize);
	if(lpBuffer == NULL) {
		Native::CloseHandle(hFile);
		debug("[WRN] CReadFile - malloc failed");
		return FALSE;
	}

	if(ReadFile(hFile, lpBuffer, dwFileSize, &dwNumberOfBytesRead, NULL) == FALSE) {
		Native::Free(lpBuffer);
		Native::CloseHandle(hFile);
		debug("[WRN] CReadFile - ReadFile failed");
		return FALSE;
	}
	else if (dwNumberOfBytesRead != dwFileSize) {
		Native::Free(lpBuffer);
		Native::CloseHandle(hFile);
		debug("[WRN] CReadFile - Failed to read all the bytes");
		return FALSE;
	}
	Native::CloseHandle(hFile);
	pBuffer.Add(lpBuffer, dwFileSize);
	Native::Free(lpBuffer);
	return TRUE;
}

bool CWriteFileLockInit2 = false;
RTL_CRITICAL_SECTION CSLockCWriteFile2;
BOOL cWriteFile(const char* FilePath, const VOID* Data, DWORD Size) {
	if (CWriteFileLockInit2 == false) {
		InitializeCriticalSection(&CSLockCWriteFile2);
		CWriteFileLockInit2 = true;
	}

	EnterCriticalSection(&CSLockCWriteFile2);
	HANDLE fHandle = Native::CreateFileEx(FilePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (fHandle == INVALID_HANDLE_VALUE) {
		debug("[WRN] CWriteFile - CreateFile failed %s", FilePath);
		return FALSE;
	}

	DWORD writeSize = Size;
	if (WriteFile(fHandle, Data, writeSize, &writeSize, NULL) != TRUE) {
		debug("[WRN] CWriteFile - WriteFile failed");
		return FALSE;
	}
	Native::CloseHandle(fHandle);
	LeaveCriticalSection(&CSLockCWriteFile2);
	return TRUE;
}

BOOL DirectoryExists(LPCSTR lpDirectoryName) {
	if (GetFileAttributes(lpDirectoryName) == -1) {
		DWORD lastError = GetLastError();
		if (lastError == ERROR_FILE_NOT_FOUND || lastError == ERROR_PATH_NOT_FOUND)
			return FALSE;
	}
	return TRUE;
}

BOOL fileExists(LPCSTR lpFileName) {
	if (FILE *file = fopen(lpFileName, "r")) {
		fclose(file);
		return TRUE;
	}else return FALSE;
}

DWORD getDeviceSize(const std::string &device){
	HANDLE file;
	OBJECT_ATTRIBUTES attr;
	IO_STATUS_BLOCK ioStatus;
	FILE_FS_SIZE_INFORMATION sizeInfo;

	ACCESS_MASK desiredAccess = SYNCHRONIZE | 1;
	DWORD shareAccess = FILE_SHARE_READ;
	DWORD openOptions = FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | 0x800000;

	ANSI_STRING deviceName;
	RtlInitAnsiString(&deviceName, device.c_str());
	InitializeObjectAttributes(&attr, &deviceName, OBJ_CASE_INSENSITIVE, NULL);
	NTSTATUS res = NtOpenFile(&file, desiredAccess, &attr, &ioStatus, shareAccess, openOptions);
	if (res != STATUS_SUCCESS)
		return 0;

	NtQueryVolumeInformationFile(file, &ioStatus, &sizeInfo, sizeof(sizeInfo), FileFsSizeInformation);
	DWORD ret = sizeInfo.TotalAllocationUnits.LowPart;
	NtClose(file);
	return ret;
}

NTSTATUS getLowPartAllocationUnits(const char* device, PDWORD dest){
	NTSTATUS ret = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES oab;
	IO_STATUS_BLOCK iosb;
	STRING lstr;
	HANDLE fhand;
	*dest = 0;
	RtlInitAnsiString(&lstr, device);
	oab.RootDirectory = NULL;
	oab.Attributes = 0x40;
	oab.ObjectName = &lstr;
	if (NT_SUCCESS(NtOpenFile(&fhand, 0x100001, &oab, &iosb, 1, 0x800021)))
	{
		FILE_FS_SIZE_INFORMATION fsinfo;
		if (NT_SUCCESS(ret = NtQueryVolumeInformationFile(fhand, &iosb, &fsinfo, sizeof(FILE_FS_SIZE_INFORMATION), FileFsSizeInformation))) {
			*dest = fsinfo.TotalAllocationUnits.LowPart;
		}
		NtClose(fhand);
	}
	return ret;
}