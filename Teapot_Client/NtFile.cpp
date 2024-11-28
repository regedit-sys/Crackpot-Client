#include "stdafx.h"
#include "common.h"
#include "NtFile.h"
#include <list>
#include <set>

using NtFile::HandleCheck;

struct NTFileConfirmParams {
	PLDR_DATA_TABLE_ENTRY table;
	bool decision;
};

QWORD HandleCheck::NtCreateASM = 0;
detour<NTSTATUS> HandleCheck::NtCreateFileTour;
std::vector<wstring> moduleWhitelist;
AsyncEvent eNtValidate;

MESSAGEBOX_RESULT MsgBoxResult;
XOVERLAPPED MsgBoxOverlapped;

bool WhiteListedApp(PLDR_DATA_TABLE_ENTRY table) {
	AppManager::C_AppInstance Process = AppManager::C_AppInstance(table);
	if (NT_SUCCESS(Process.iStatus)) {
		return Process.IsWhiteListedApp();
	}
	return false;
}

void NtOpenFileConfirmWindow(NTFileConfirmParams &Params) {
	LPCWSTR buttons[2] = { L"Yes", L"No" };
	wstring msg(L"Something is attempting to read or modify your KV.bin!\n\n\n\n\n");
	msg.append(L"Application in question:\n'");
	msg.append(Params.table->BaseDllName.Buffer);
	msg.append(L"'\n\n\nWould you like to allow this action?\n(You may see this prompt multiple times)");
	XShowMessageBoxUI(0, L"Crackpot KV Protection", msg.c_str(), 2, buttons, 0, XMB_WARNINGICON, &MsgBoxResult, &MsgBoxOverlapped);
	while (!XHasOverlappedIoCompleted(&MsgBoxOverlapped)) 
		Native::Sleep(10);

	Native::Sleep(700);
	Params.decision = !MsgBoxResult.dwButtonPressed;
	eNtValidate.Signal(AsyncEvent::STATE_COMPLETE);
}

bool HandleCheck::NtValidateOpeningFile(DWORD linkReg, POBJECT_ATTRIBUTES objectAttributes) {
	string fileName(objectAttributes->ObjectName->Buffer);
	transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);

	if (fileName.find(getString(STR_KV)) != string::npos) {
		NTFileConfirmParams Params;
		Native::XexPcToFileHeader((PVOID)linkReg, &Params.table);
		if (Params.table && AppMgr.HasDashLoaded) {
			if (WhiteListedApp(Params.table)) return false;
			eNtValidate.WaitForReset();
			eNtValidate.Create(0xAAF);

			Params.decision = false;
			if (KeGetCurrentProcessType() != PROC_USER)
				CreateWorkerExQ(NtOpenFileConfirmWindow, &Params);
			else NtOpenFileConfirmWindow(Params);
			eNtValidate.WaitForCompletion(INFINITE);
			eNtValidate.Reset();
			return Params.decision;
		}else return Params.table? Params.table->CheckSum == AppMgr.Self.Checksum ? true : false : false;
	}
	return true;
}

NTSTATUS HandleCheck::NtCreateFileHook(PHANDLE FileHandle, ACCESS_MASK DesiredAccess, POBJECT_ATTRIBUTES ObjectAttributes, PIO_STATUS_BLOCK IoStatusBlock, PLARGE_INTEGER AllocationSize, DWORD FileAttributes, DWORD ShareAccess, DWORD CreateDisposition, DWORD CreateOptions) {
	DWORD linkReg;
	__asm mflr linkReg

	if (ini::KVProtection && !NtValidateOpeningFile(linkReg, ObjectAttributes)) {
		*FileHandle = INVALID_HANDLE_VALUE;
		return 0xc000000f;
	}
	return NtCreateFileTour.callOriginal(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions);
}

HRESULT HandleCheck::install() {
	std::string krnlTxt = getString(STR_KERNEL);
	NtCreateFileTour.setupDetour(krnlTxt.cstr, 210, NtCreateFileHook);
	NtCreateASM = *(PQWORD)Native::resolveFunction(krnlTxt.cstr, 210);
	return ERROR_SUCCESS;
}

bool HandleCheck::tamperCheck() {
	if (*(PQWORD)Native::resolveFunction(getString(STR_KERNEL).cstr, 210) != NtCreateASM) return false;
	return true;
}