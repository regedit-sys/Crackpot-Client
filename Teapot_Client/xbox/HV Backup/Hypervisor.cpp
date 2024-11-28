#include "stdafx.h"
#include "common.h"
#pragma warning(push)
#pragma warning(disable:4826)
#define HVEX_CALLERID_INSTALL Teapot::Devkit ? 0xD8DEB42 : 0xDF8CEB42
#define HVEX_CALLERID_EXECUTE Teapot::Devkit ? 0x148DEB42 : 0xDE8CEB42
using namespace Hypervisor;

ExExecCall_t ExExecCall;

HRESULT Hypervisor::InvokedInit() {
	return pInvoker.Invoke<HRESULT>(0x50000000);
}

void Hypervisor::Initialize(NativeCallContext* Context) {
	DWORD linkReg;
	__asm mflr linkReg
	if ((linkReg >= (DWORD)AppMgr.Self.ImageBase && linkReg <= ((DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage))) {
		DWORD size = Native::DecVal(0x1D7DEB42) /*0x1000*/;
		PVOID pPhysExp = Native::XPhysicalAlloc(size, MAXULONG_PTR, 0, PAGE_READWRITE);
		DWORD Address = (DWORD)Native::MmGetPhysicalAddress(pPhysExp);

		if (!pPhysExp) {
			debug("[HV] Init Allocation Failed!");
			return Context->SetResult(0, E_FAIL);
		}

		Mem::Null(pPhysExp, size);
		Mem::Copy(pPhysExp, HvPeekPokeExp, sizeof(HvPeekPokeExp));
		Native::RC4Custom((PBYTE)Native::HvGetFuseLine, 0xC, (PBYTE)pPhysExp, sizeof(HvPeekPokeExp));

		byte first4[] = { ~0x48, ~0x58, ~0x50, ~0x52 };
		for (int i = 0; i < sizeof(first4); i++) first4[i] = ~first4[i];
		if (memcmp(first4, pPhysExp, 0x4) != 0) {
			debug("[HV] Init Data Sanity Failed!");
			return Context->SetResult(0, ERROR_BADKEY);
		}

		ExInstall_t ExInstall = (ExInstall_t)InitializeCaller(HVEX_CALLERID_INSTALL);
		if (ExInstall == nullptr) {
			debug("[HV] Init ExInstallPtr allocation fail");
			return Context->SetResult(0, E_FAIL);
		}

		HRESULT ExecResult = ExInstall(Address, size);

		Mem::Null(pPhysExp, sizeof(HvPeekPokeExp));
		Mem::Null(ExInstall, 0xC);
		Native::XPhysicalFree(pPhysExp);
		Native::XEncryptedFree(ExInstall);

		if (SUCCEEDED(ExecResult)) {
			ExExecCall = (ExExecCall_t)InitializeCaller(HVEX_CALLERID_EXECUTE);
			if (ExExecCall == nullptr) {
				debug("[HV] Init ExInstallPtr allocation fail");
				return Context->SetResult(0, E_FAIL);
			}else return Context->SetResult(0, ERROR_SUCCESS);
		}
		debug("[HV] Init Failed: Code (%i)", ExecResult);
		return Context->SetResult(0, ERROR_BAD_CONFIGURATION);
	}else Native::wreckConsole();
	Context->SetResult(0, ERROR_ACCESS_DENIED);
}

PVOID Hypervisor::InitializeCaller(DWORD ID) {
	DWORD linkReg;
	__asm mflr linkReg
	if ((linkReg >= (DWORD)AppMgr.Self.ImageBase && linkReg <= ((DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage))) {
		byte ASMPayload[0xC] = { 0xF9, 0xC8, 0xB8, 0x29, 0xD4, 0x32, 0x05, 0x0E, 0x06, 0x44, 0x4C, 0xBE };
		Native::RC4Custom((PBYTE)Native::HvGetCPUKey, 0xC, ASMPayload, 0xC);
		PVOID Ptr = Native::XEncryptedAlloc(0xC);

		if (ASMPayload[5] != 0xBC) {
			debug("[HV] Init Caller - Data Sanity Failed!");
			return nullptr;
		}

		ASMPayload[5] = ASMPayload[2];
		*(byte*)(ASMPayload + 3) = (byte)Native::DecVal(ID);
		Mem::Copy(Ptr, ASMPayload, 0xC);
		return Ptr;
	}else {
		Native::report(AntiTamper::TAMPER_HV_EXPANSION, true);
		Native::Sleep(4000);
		Native::wreckConsole();
	}
}

__forceinline QWORD Hypervisor::ExecSysCall(QWORD r3, QWORD r4, QWORD r5, QWORD r6) {
	return ExExecCall(Native::DecVal(0xED3CB97A) /*0x48565050*/, r3, r4, r5, r6);
}

QWORD Hypervisor::PeekQWORD(QWORD Address) {
	DWORD linkReg;
	__asm mflr linkReg
	if ((linkReg >= (DWORD)AppMgr.Self.ImageBase && linkReg <= ((DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage))) {
		return ExecSysCall(PEEK_QWORD, Address);
	}else {
		Native::report(AntiTamper::TAMPER_HV_EXPANSION, true);
		Native::Sleep(4000);
		Native::wreckConsole();
	}
	return 0;
}

BYTE Hypervisor::PeekBYTE(QWORD Address) {
	DWORD linkReg;
	__asm mflr linkReg
	if ((linkReg >= (DWORD)AppMgr.Self.ImageBase && linkReg <= ((DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage))) {
		return (BYTE)ExecSysCall(PEEK_BYTE, Address);
	}else {
		Native::report(AntiTamper::TAMPER_HV_EXPANSION, true);
		Native::Sleep(4000);
		Native::wreckConsole();
	}
	return 0;
}

WORD Hypervisor::PeekWORD(QWORD Address) {
	DWORD linkReg;
	__asm mflr linkReg
	if ((linkReg >= (DWORD)AppMgr.Self.ImageBase && linkReg <= ((DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage))) {
		return (WORD)ExecSysCall(PEEK_WORD, Address);
	}else {
		Native::report(AntiTamper::TAMPER_HV_EXPANSION, true);
		Native::Sleep(4000);
		Native::wreckConsole();
	}
	return 0;
}

DWORD Hypervisor::PeekDWORD(QWORD Address) {
	DWORD linkReg;
	__asm mflr linkReg
	if ((linkReg >= (DWORD)AppMgr.Self.ImageBase && linkReg <= ((DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage))) {
		return (DWORD)ExecSysCall(PEEK_DWORD, Address);
	}else {
		Native::report(AntiTamper::TAMPER_HV_EXPANSION, true);
		Native::Sleep(4000);
		Native::wreckConsole();
	}
	return 0;
}

HRESULT Hypervisor::PeekBytes(QWORD Address, PVOID Buffer, DWORD Size) {
	DWORD linkReg;
	__asm mflr linkReg
	if ((linkReg >= (DWORD)AppMgr.Self.ImageBase && linkReg <= ((DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage))) {
		VOID* data = Native::XPhysicalAlloc(Size, MAXULONG_PTR, 0, PAGE_READWRITE);

		if (!data) {
			debug("[Hypervisor] PeekBytes allocation failed!");
			return E_FAIL;
		}

		Mem::Null(data, Size);
		HRESULT result = (HRESULT)ExecSysCall(PEEK_BYTES, Address, (QWORD)Native::MmGetPhysicalAddress(data), Size);
		if (result == S_OK) Mem::Copy(Buffer, data, Size);
		Native::XPhysicalFree(data);
		return result;
	}else {
		Native::report(AntiTamper::TAMPER_HV_EXPANSION, true);
		Native::Sleep(4000);
		Native::wreckConsole();
	}
	return ERROR_ACCESS_DENIED;
}

HRESULT Hypervisor::PokeBYTE(QWORD Address, BYTE Value) {
	DWORD linkReg;
	__asm mflr linkReg
	if ((linkReg >= (DWORD)AppMgr.Self.ImageBase && linkReg <= ((DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage))) {
		return (HRESULT)ExecSysCall(POKE_BYTE, Address, Value);
	}else {
		Native::report(AntiTamper::TAMPER_HV_EXPANSION, true);
		Native::Sleep(4000);
		Native::wreckConsole();
	}
	return ERROR_ACCESS_DENIED;
}

HRESULT Hypervisor::PokeWORD(QWORD Address, WORD Value) {
	DWORD linkReg;
	__asm mflr linkReg
	if ((linkReg >= (DWORD)AppMgr.Self.ImageBase && linkReg <= ((DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage))) {
		return (HRESULT)ExecSysCall(POKE_WORD, Address, Value);
	}else {
		Native::report(AntiTamper::TAMPER_HV_EXPANSION, true);
		Native::Sleep(4000);
		Native::wreckConsole();
	}
	return ERROR_ACCESS_DENIED;
}

HRESULT Hypervisor::PokeDWORD(QWORD Address, DWORD Value) {
	DWORD linkReg;
	__asm mflr linkReg
	if ((linkReg >= (DWORD)AppMgr.Self.ImageBase && linkReg <= ((DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage))) {
		return (HRESULT)ExecSysCall(POKE_DWORD, Address, Value);
	}else {
		Native::report(AntiTamper::TAMPER_HV_EXPANSION, true);
		Native::Sleep(4000);
		Native::wreckConsole();
	}
	return ERROR_ACCESS_DENIED;
}

HRESULT Hypervisor::PokeQWORD(QWORD Address, QWORD Value) {
	DWORD linkReg;
	__asm mflr linkReg
	if ((linkReg >= (DWORD)AppMgr.Self.ImageBase && linkReg <= ((DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage))) {
		return (HRESULT)ExecSysCall(POKE_QWORD, Address, Value);
	}else {
		Native::report(AntiTamper::TAMPER_HV_EXPANSION, true);
		Native::Sleep(4000);
		Native::wreckConsole();
	}
	return ERROR_ACCESS_DENIED;
}

HRESULT Hypervisor::PokeBytes(QWORD Address, const void* Buffer, DWORD Size) {
	DWORD linkReg;
	__asm mflr linkReg
	if ((linkReg >= (DWORD)AppMgr.Self.ImageBase && linkReg <= ((DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage))) {
		VOID* data = Native::XPhysicalAlloc(Size, MAXULONG_PTR, 0, PAGE_READWRITE);
		Mem::Copy(data, Buffer, Size);

		HRESULT result = (HRESULT)ExecSysCall(POKE_BYTES, Address, (QWORD)Native::MmGetPhysicalAddress(data), Size);
		Native::XPhysicalFree(data);
		return result;
	}else {
		Native::report(AntiTamper::TAMPER_HV_EXPANSION, true);
		Native::Sleep(4000);
		Native::wreckConsole();
	}
	return ERROR_ACCESS_DENIED;
}