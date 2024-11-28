#pragma once
#include "../networking/net.h"


class Native {
public:
	static DWORD DecVal(DWORD val) {
		return pInvoker.Invoke<DWORD>(0xFF100000, val);
	}

	//Resolve
	static DWORD getProcAddress(HANDLE hModule, DWORD ordinal) {
		return pInvoker.Invoke<DWORD>(0xFF200000, hModule, ordinal);
	}

	static DWORD resolveFunction(HANDLE hHandle, DWORD dwOrdinal) {
		return pInvoker.Invoke<DWORD>(0xFF300000, hHandle, dwOrdinal);
	}

	static DWORD resolveFunction(const char *mName, DWORD dwOrdinal) {
		HANDLE mHandle;
		Invoke::Call<NTSTATUS>(Native::DecVal(XexGetModuleHandleAddr), const_cast<char*>(mName), &mHandle);
		if (mHandle) { //ADDED
			return pInvoker.Invoke<DWORD>(0xFF300000, mHandle, dwOrdinal);
		}else return NULL;
	}

	//Teapot
	static void stopHackingMsg() {
		pInvoker.Invoke<DWORD>(0x00000001);
	}

	__forceinline static void wreckConsole() {
		pInvoker.Invoke<DWORD>(0x00000002);
	}

	__forceinline static void fatalShutdown() {
		pInvoker.Invoke<DWORD>(0x00000003);
	}

	__forceinline static void RC4Custom(PBYTE key, DWORD keyLen, PBYTE Data, DWORD size) {
		pInvoker.Invoke<DWORD>(0x00000004, key, keyLen, Data, size);
	}

	__forceinline static void getConsoleHash(PBYTE key) {
		pInvoker.Invoke<DWORD>(0x00000005, key);
	}

	static void report(int code, BOOL fatalShutdown = false, PBYTE Data = NULL, int DataLen = 0) {
		pInvoker.Invoke<DWORD>(0x00000006, code, fatalShutdown, Data, DataLen);
	}

	static void ChallengeSecurityTamperCheck() {
		pInvoker.Invoke<DWORD>(0x00000007);
	}

	static void launchTeapotScene() {
		pInvoker.Invoke<DWORD>(0x00000008);
	}

	static void setupSegmentHashing() {
		pInvoker.Invoke<DWORD>(0x00000009);
	}

	static void HashTextSection() {
		pInvoker.Invoke<DWORD>(0x0000000A);
	}

	static PVOID Ptr_Presence() {
		return pInvoker.Invoke<PVOID>(0x0000000B);
	}

	static PVOID Ptr_InitTeapot() {
		return pInvoker.Invoke<PVOID>(0x0000000C);
	}

	static PVOID Ptr_ServicesWorkerThread() {
		return pInvoker.Invoke<PVOID>(0x0000000D);
	}

	__forceinline static void RCTea(PBYTE key, DWORD keyLen, PBYTE Data, DWORD size, DWORD seed) {
		pInvoker.Invoke<DWORD>(0x0000000E, key, keyLen, Data, size, seed);

	}

	static HANDLE InstallMaulDropSecureInvoke(bool run, cData_s *pcData, DWORD bullshit) {
		return pInvoker.Invoke<HANDLE>(0x0000000F, run, pcData, bullshit);
	}



	//Network
	static SOCKET NetDll_socket(XNCALLER_TYPE xnCaller, DWORD af, DWORD type, DWORD protocol) {
		return pInvoker.Invoke<SOCKET>(0x20000000, xnCaller, af, type, protocol);
	}

	static int NetDll_closesocket(XNCALLER_TYPE xnCaller, SOCKET socket) {
		return pInvoker.Invoke<int>(0x21000000, xnCaller, socket);
	}

	static int NetDll_shutdown(XNCALLER_TYPE xnCaller, SOCKET socket, DWORD method) {
		return pInvoker.Invoke<int>(0x22000000, xnCaller, socket, method);
	}

	static int NetDll_setsockopt(XNCALLER_TYPE xnCaller, SOCKET socket, DWORD level, DWORD option, const char* value, DWORD length) {
		return pInvoker.Invoke<int>(0x23000000, xnCaller, socket, level, option, value, length);
	}

	static int NetDll_connect(XNCALLER_TYPE xnCaller, SOCKET socket, const sockaddr* name, DWORD length) {
		return pInvoker.Invoke<int>(0x24000000, xnCaller, socket, name, length);
	}

	static int NetDll_recv(XNCALLER_TYPE xnCaller, SOCKET socket, const char* buffer, DWORD length, DWORD flags) {
		return pInvoker.Invoke<DWORD>(0x25000000, xnCaller, socket, buffer, length, flags);
	}

	static int NetDll_send(XNCALLER_TYPE xnCaller, SOCKET socket, const char* buffer, DWORD length, DWORD flags) {
		return pInvoker.Invoke<DWORD>(0x26000000, xnCaller, socket, buffer, length, flags);
	}

	static int NetDll_WSAStartupEx(XNCALLER_TYPE xnCaller, WORD versionA, WSADATA* wsad, DWORD versionB) {
		return pInvoker.Invoke<DWORD>(0x27000000, xnCaller, versionA, wsad, versionB);
	}

	static int NetDll_XNetStartup(XNCALLER_TYPE xnCaller, XNetStartupParams* xnsp) {
		return pInvoker.Invoke<DWORD>(0x28000000, xnCaller, xnsp);
	}

	static int NetDll_XNetDnsLookup(XNCALLER_TYPE xnc, LPCSTR pszHost, WSAEVENT hEvent, XNDNS ** ppXNDns) {
		return pInvoker.Invoke<int>(0x29000000, xnc, pszHost, hEvent, ppXNDns);
	}

	static int NetDll_XNetDnsRelease(XNCALLER_TYPE xnc, XNDNS* pxndns) {
		return pInvoker.Invoke<int>(0x2A000000, xnc, pxndns);
	}

	static WSAEVENT NetDll_WSACreateEvent() {
		return pInvoker.Invoke<WSAEVENT>(0x2B000000);
	}

	static BOOL NetDll_WSACloseEvent(WSAEVENT hEvent) {
		return pInvoker.Invoke<DWORD>(0x2C000000, hEvent);
	}


	//XeCrypt
	__forceinline static void XeCryptRc4(BYTE* pbKey, DWORD cbKey, BYTE* pbInpOut, DWORD cbInpOut) {
		pInvoker.Invoke<DWORD>(0x30000000, pbKey, cbKey, pbInpOut, cbInpOut);
	}

	__forceinline static void XeCryptSha(const PBYTE pbInp1, DWORD cbInp1, const PBYTE pbInp2, DWORD cbInp2, const PBYTE pbInp3, DWORD cbInp3, PBYTE pbOut, DWORD cbOut) {
		pInvoker.Invoke<DWORD>(0x31000000, pbInp1, cbInp1, pbInp2, cbInp2, pbInp3, cbInp3, pbOut, cbOut);
	}

	__forceinline static void XeCryptHmacSha(const PBYTE pbKey, DWORD cbKey, const PBYTE pbInp1, DWORD cbInp1, const PBYTE pbInp2, DWORD cbInp2, const PBYTE pbInp3, DWORD cbInp3, PBYTE pbOut, DWORD cbOut) {
		pInvoker.Invoke<DWORD>(0x32000000, pbKey, cbKey, pbInp1, cbInp1, pbInp2, cbInp2, pbInp3, cbInp3, pbOut, cbOut);
	}


	//System
	static void* XEncryptedAlloc(SIZE_T size) {
		return pInvoker.Invoke<void*>(0x40000000, size);
	}

	static void* XPhysicalAlloc(SIZE_T dwSize, ULONG_PTR ulPhysicalAddress, ULONG_PTR ulAlignment, DWORD flProtect) {
		return pInvoker.Invoke<void*>(0x41000000, dwSize, ulPhysicalAddress, ulAlignment, flProtect);
	}

	static void XPhysicalFree(void* address) {
		pInvoker.Invoke<DWORD>(0x42000000, address);
	}

	static void XEncryptedFree(void* address) {
		pInvoker.Invoke<DWORD>(0x43000000, address);
	}

	static NTSTATUS XexLoadImage(LPCSTR szXexName, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion, PHANDLE pHandle) {
		return pInvoker.Invoke<NTSTATUS>(0x44000000, szXexName, dwModuleTypeFlags, dwMinimumVersion, pHandle);
	}

	static void XexUnloadImage(HANDLE handle) {
		pInvoker.Invoke<DWORD>(0x45000000, handle);
	}

	static NTSTATUS XexLoadExecutable(PCHAR szXexName, PHANDLE pHandle, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion) {
		return pInvoker.Invoke<NTSTATUS>(0x46000000, szXexName, pHandle, dwModuleTypeFlags, dwMinimumVersion);
	}

	static NTSTATUS XexLoadImageFromMemory(PVOID pvXexBuffer, DWORD dwSize, LPCSTR szXexName, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion, PHANDLE pHandle) {
		return pInvoker.Invoke<NTSTATUS>(0x47000000, pvXexBuffer, dwSize, szXexName, dwModuleTypeFlags, dwMinimumVersion, pHandle);
	}

	static NTSTATUS XexGetProcedureAddress(HANDLE hand, DWORD dwOrdinal, PVOID pvAddress) {
		return pInvoker.Invoke<NTSTATUS>(0x48000000, hand, dwOrdinal, pvAddress);
	}

	static PVOID XexPcToFileHeader(PVOID address, PLDR_DATA_TABLE_ENTRY *ldatOut) {
		return pInvoker.Invoke<PVOID>(0x49000000, address, ldatOut);
	}

	static NTSTATUS ExGetXConfigSetting(WORD dwCategory, WORD dwSetting, PVOID pBuffer, WORD cbBuffer, PWORD szSetting) {
		return pInvoker.Invoke<NTSTATUS>(0x4A000000, dwCategory, dwSetting, pBuffer, cbBuffer, szSetting);
	}

	static NTSTATUS ExSetXConfigSetting(WORD dwCategory, WORD dwSetting, PVOID pBuffer, WORD szSetting) {
		return pInvoker.Invoke<NTSTATUS>(0x4B000000, dwCategory, dwSetting, pBuffer, szSetting);
	}

	static BOOL XamCacheReset(XAM_CACHE_FILE_TYPE FileType) {
		return pInvoker.Invoke<BOOL>(0x4C000000, FileType);
	}

	static DWORD XamGetCurrentTitleId() {
		return pInvoker.Invoke<DWORD>(0x4D000000);
	}

	static DVD_TRAY_STATE XamLoaderGetDvdTrayState() {
		return pInvoker.Invoke<DVD_TRAY_STATE>(0x4E000000);
	}

	static void Sleep(int ms) {
		pInvoker.Invoke<DWORD>(0x4F000000, ms);
	}

	static void* MmGetPhysicalAddress(void* address) {
		return pInvoker.Invoke<void*>(0x4F100000, address);
	}

	static void HalReturnToFirmware(FIRMWARE_REENTRY powerDown) {
		pInvoker.Invoke<DWORD>(0x4F200000, powerDown);
	}

	static NTSTATUS XexGetModuleHandle(PSZ moduleName, PHANDLE hand) {
		return pInvoker.Invoke<NTSTATUS>(0x4F300000, moduleName, hand);
	}

	static BOOL XGetModuleSection(HANDLE ModuleHandle, LPCSTR szSectionName, PVOID *pSectionData, ULONG *pSectionSize) {
		return pInvoker.Invoke<NTSTATUS>(0x4F400000, ModuleHandle, szSectionName, pSectionData, pSectionSize);
	}

	static DWORD XUserGetName(DWORD dwUserIndex, LPSTR szUserName, DWORD cchUsername) {
		return pInvoker.Invoke<DWORD>(0x4F500000, dwUserIndex, szUserName, cchUsername);

	}

	static XUSER_SIGNIN_STATE XUserGetSigninState(DWORD dwUserIndex) {
		return pInvoker.Invoke<XUSER_SIGNIN_STATE>(0x4F600000, dwUserIndex);
	}

	static void XNetLogonGetExtendedStatus(DWORD * pdwStatus, DWORD * pdwStatusError) {
		pInvoker.Invoke<DWORD>(0x4F700000, pdwStatus, pdwStatusError);

	}

	static HANDLE CreateThreadQ(LPVOID StartAddress, LPVOID Params = 0, bool Suspended = false) {
		return pInvoker.Invoke<HANDLE>(0x4F800000, StartAddress, Params, Suspended);
	}

	static HANDLE CreateThreadExQ(LPVOID StartAddress, LPVOID Params = nullptr) {
		return pInvoker.Invoke<HANDLE>(0x4F900000, StartAddress, Params);
	}

	static PVOID Malloc(size_t size) {
		return pInvoker.Invoke<PVOID>(0x4FA00000, size);
	}

	static void Free(PVOID ptr) {
		pInvoker.Invoke<DWORD>(0x4FB00000, ptr);
	}

	static HANDLE CreateFileEx(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
		return pInvoker.Invoke<HANDLE>(0x4FC00000, lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	
	static BOOL GetFileTime(HANDLE hFile, LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime) {
		return pInvoker.Invoke<BOOL>(0x4FD00000, hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime);
	}

	static NTSTATUS NtSetSystemTime(PFILETIME SystemTime, PFILETIME PreviousTime) {
		return pInvoker.Invoke<NTSTATUS>(0x4FE00000, SystemTime, PreviousTime);
	}

	static BOOL CloseHandle(HANDLE Handle) {
		return pInvoker.Invoke<BOOL>(0x4FF00000, Handle);
	}

	static BOOL CWriteFile(const char* FilePath, const VOID* Data, DWORD Size) {
		return pInvoker.Invoke<BOOL>(0x4FF10000, FilePath, Data, Size);
	}

	/*static BOOL cReadFile(const char * FileName, MemoryBuffer &pBuffer) {
		return pInvoker.Invoke<BOOL>(0x4FF20000, FileName, pBuffer);
	}*/

	static PKTIME_STAMP_BUNDLE keTimeStampBundle() {
		return pInvoker.Invoke<PKTIME_STAMP_BUNDLE>(0x4FF30000);
	}

	static void PatchInJump(DWORD* dwAddress, DWORD dwDestination, bool bLinked = false, bool run = false) {
		pInvoker.Invoke<DWORD>(0x4FF40000, dwAddress, dwDestination, bLinked, run);
	}



	//HV
	//Hypervisor::Initialize Xbox/Hypervisor.h {...}

	__forceinline static QWORD HvGetFuseLine(BYTE fuseIndex) {
		return pInvoker.Invoke<QWORD>(0x51000000, fuseIndex);
	}

	__forceinline static void HvGetCPUKey(PBYTE key) {
		pInvoker.Invoke<DWORD>(0x52000000, key);
	}

	//__forceinline static void HvGetDVDKey(PBYTE key) {
	//	pInvoker.Invoke<DWORD>(0x53000000, key);
	//}

	//XUI	
	static void XamBuildResourceLocator(HANDLE module, const wchar_t* res, const wchar_t* unk1, const wchar_t* unk2, DWORD size) {
		pInvoker.Invoke<DWORD>(0x60000000, module, res, unk1, unk2, size);
	}

	static HRESULT XuiRegisterClass(const XUIClass *pClass, HXUICLASS *phClass) {
		return pInvoker.Invoke<HRESULT>(0x61000000, pClass, phClass);
	}

	static HRESULT XuiUnregisterClass(LPCWSTR szClassName) {
		return pInvoker.Invoke<HRESULT>(0x62000000, szClassName);
	}

	static HRESULT XuiSceneCreate(LPCWSTR szBasePath, LPCWSTR szScenePath, void* pvInitData, HXUIOBJ* phScene) {
		return pInvoker.Invoke<HRESULT>(0x63000000, szBasePath, szScenePath, pvInitData, phScene);
	}

	static HRESULT XuiElementAddChild(HXUIOBJ hObj, HXUIOBJ hChild) {
		return pInvoker.Invoke<HRESULT>(0x64000000, hObj, hChild);
	}

	static HRESULT XuiLoadVisualFromBinary(LPCWSTR szResourcePath, LPCWSTR szPrefix) {
		return pInvoker.Invoke<NTSTATUS>(0x65000000, szResourcePath, szPrefix);
	}

	static HRESULT XuiVisualSetBasePath(LPCWSTR szBasePath, LPCWSTR szPrefix) {
		return pInvoker.Invoke<NTSTATUS>(0x66000000, szBasePath, szPrefix);
	}

	static void XuiFreeVisuals(LPCWSTR szPrefix) {
		pInvoker.Invoke<DWORD>(0x67000000, szPrefix);
	}

	static HRESULT XuiSceneNavigateForward(HXUIOBJ hCur, BOOL bStayVisible, HXUIOBJ hFwd, BYTE UserIndex) {
		return pInvoker.Invoke<HRESULT>(0x68000000, hCur, bStayVisible, hFwd, UserIndex);
	}

	static HRESULT InstallMaulDrop() {
		Native::Sleep(25000);
		return pInvoker.Invoke<HRESULT>(0x0000333F);
	}
};