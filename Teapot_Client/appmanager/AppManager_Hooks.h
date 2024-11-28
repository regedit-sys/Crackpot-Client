#pragma once

namespace AppManager {
	namespace Hooks {
		namespace Security {
			void __cdecl APCWorker(void* Arg1, void* Arg2, void* Arg3);
			void XSecurityCloseProcessHook();
			DWORD XSecurityCreateProcessHook(DWORD dwHardwareThread);
			DWORD XSecurityVerifyHook(DWORD dwMilliseconds, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
			DWORD XSecurityGetFailureInfoHook(PXSECURITY_FAILURE_INFORMATION pFailureInformation);
			NTSTATUS XexGetProcedureAddressHook(HANDLE hand, DWORD dwOrdinal, PVOID* pvAddress);
		};
		NTSTATUS XexLoadImage(LPCSTR szXexName, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion, PHANDLE pHandle);
		NTSTATUS XexLoadExecutable(PCHAR szXexName, PHANDLE pHandle, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion);
		NTSTATUS XexLoadImageFromMemoryHook(PVOID pvXexBuffer, DWORD dwSize, LPCSTR szXexName, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion, PHANDLE pHandle);
	
		VOID* RtlImageXexHeaderFieldHook(VOID* headerBase, DWORD imageKey);
		BOOL XexCheckExecutablePrivilegeHook(DWORD priv);
	};
};