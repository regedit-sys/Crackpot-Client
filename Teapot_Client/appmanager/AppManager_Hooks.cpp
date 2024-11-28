#include "stdafx.h"
#include "AppManager.h"

using namespace AppManager;

void __cdecl Hooks::Security::APCWorker(void* Arg1, void* Arg2, void* Arg3) {
	if (Arg2) ((LPOVERLAPPED_COMPLETION_ROUTINE)Arg2)((DWORD)Arg3, 0, (LPOVERLAPPED)Arg1);
}

void Hooks::Security::XSecurityCloseProcessHook() {
	return;
}

static DWORD Hooks::Security::XSecurityCreateProcessHook(DWORD dwHardwareThread) {
	return ERROR_SUCCESS;
}

DWORD Hooks::Security::XSecurityVerifyHook(DWORD dwMilliseconds, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
	if (lpCompletionRoutine) NtQueueApcThread((HANDLE)-2, (PIO_APC_ROUTINE)APCWorker, lpOverlapped, (PIO_STATUS_BLOCK)lpCompletionRoutine, 0);
	return ERROR_SUCCESS;
}

DWORD Hooks::Security::XSecurityGetFailureInfoHook(PXSECURITY_FAILURE_INFORMATION pFailureInformation) {
	if (pFailureInformation->dwSize == 0x14) {
		pFailureInformation->dwBlocksChecked = 0x64;
		pFailureInformation->dwFailedHashes = 0;
		pFailureInformation->dwFailedReads = 0;
		pFailureInformation->dwTotalBlocks = 0x64;
	}else if (pFailureInformation->dwSize == sizeof(XSECURITY_FAILURE_INFORMATION)) {
		pFailureInformation->dwBlocksChecked = 0x100;
		pFailureInformation->dwFailedHashes = 0;
		pFailureInformation->dwFailedReads = 0;
		pFailureInformation->dwTotalBlocks = 0x100;
		pFailureInformation->fComplete = TRUE;
	}else return ERROR_NOT_ENOUGH_MEMORY;
	return ERROR_SUCCESS;
}

NTSTATUS Hooks::Security::XexGetProcedureAddressHook(HANDLE hand, DWORD dwOrdinal, PVOID* pvAddress) {
	if (hand == AppMgr.Xam.hand) {
		switch (dwOrdinal) {
		case 0x9BB: *pvAddress = XSecurityCreateProcessHook; return 0;
		case 0x9BC: *pvAddress = XSecurityCloseProcessHook; return 0;
		case 0x9BD: *pvAddress = XSecurityVerifyHook; return 0;
		case 0x9BE: *pvAddress = XSecurityGetFailureInfoHook; return 0;
		}
	}
	return XexGetProcedureAddress(hand, dwOrdinal, pvAddress);
}

NTSTATUS Hooks::XexLoadImage(LPCSTR szXexName, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion, PHANDLE pHandle) {
	HANDLE mHandle = 0;
	NTSTATUS result = Native::XexLoadImage(szXexName, dwModuleTypeFlags, dwMinimumVersion, &mHandle);
	if (pHandle != NULL) *pHandle = mHandle;
	if (NT_SUCCESS(result)) {
		C_AppInstance Instance = C_AppInstance(mHandle);
		if (SUCCEEDED(Instance.iStatus)) Instance.Init();
	}
	return result;
}

NTSTATUS Hooks::XexLoadExecutable(PCHAR szXexName, PHANDLE pHandle, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion) {
	HANDLE mHandle = 0;
	NTSTATUS result = Native::XexLoadExecutable(szXexName, pHandle, dwModuleTypeFlags, dwMinimumVersion);
	if (pHandle != NULL) *pHandle = mHandle;
	if (NT_SUCCESS(result)) {
		C_AppInstance Instance = C_AppInstance(*XexExecutableModuleHandle);
		if (SUCCEEDED(Instance.iStatus)) Instance.Init();
	}
	return result;
}

NTSTATUS Hooks::XexLoadImageFromMemoryHook(PVOID pvXexBuffer, DWORD dwSize, LPCSTR szXexName, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion, PHANDLE pHandle) {
	//if (strstr(szXexName, "xosc") != NULL) {
	//	dumpBuffer(szXexName, pvXexBuffer, dwSize);
	//}
	NTSTATUS result = XexLoadImageFromMemory(pvXexBuffer, dwSize, szXexName, dwModuleTypeFlags, dwMinimumVersion, pHandle);
	return result;
}

VOID* Hooks::RtlImageXexHeaderFieldHook(VOID* headerBase, DWORD imageKey) {
	VOID* retVal = RtlImageXexHeaderField(headerBase, imageKey);
	if (imageKey == Native::DecVal(0x2B8DE742) /*0x40006*/) {
		if (retVal) {
			DWORD titleID = ((XEX_EXECUTION_ID*)retVal)->TitleID;
			if (isBadTitle(titleID)) retVal = &AppMgr.SpoofedTitle;
		}else retVal = &AppMgr.SpoofedTitle;
	}
	return retVal;
}

BOOL Hooks::XexCheckExecutablePrivilegeHook(DWORD priv) {
	if (priv == XEX_PRIVILEGE_INSECURE_SOCKETS) return TRUE;
	return XexCheckExecutablePrivilege(priv);
}