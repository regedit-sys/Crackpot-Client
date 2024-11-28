#pragma once

namespace NtFile{
	class HandleCheck {
	public:
		static HRESULT install();
		static void uninstall();
		static bool tamperCheck();

	private:
		static QWORD NtOpenASM;
		static QWORD NtCreateASM;
		static detour<NTSTATUS> HandleCheck::NtOpenFileTour;
		static detour<NTSTATUS> HandleCheck::NtCreateFileTour;
		static bool NtValidateOpeningFile(DWORD linkReg, POBJECT_ATTRIBUTES objectAttributes);
		static NTSTATUS NtCreateFileHook(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize, DWORD FileAttributes, DWORD ShareAccess, DWORD CreateDisposition, DWORD CreateOptions);
		static NTSTATUS NtOpenFileHook(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, DWORD ShareAccess, DWORD OpenOptions);
	};
}