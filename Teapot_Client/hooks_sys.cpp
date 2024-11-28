#include "stdafx.h"
#include "common.h"
#include "xbox/challenges.h"
#include "util/XeCrypt.h"
#include "security/XMitSecMsg.h"
#include "xbox/Devkit.h"
#include "NtFile.h"
#include "xbox/XamExecuteChallenge.h"

detour<bool> tourMmDbgReadCheck;
detour<HRESULT> DetourThreadProcServiceSystemTasks;
detour<void> keBugCheckExTour;
AsyncEvent AsyncPresenceEvent;


bool MmDbgReadCheckHook(DWORD address) {
	bool violation = (address >= (DWORD)AppMgr.Self.ImageBase && address <= (DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage);
	for (size_t i = 0; i < AppMgr.CheatImages.size(); ++i) {
		if (AppMgr.CheatImages[i]) {
			PLDR_DATA_TABLE_ENTRY hEngine = (PLDR_DATA_TABLE_ENTRY)AppMgr.CheatImages[i];
			violation = (address >= (DWORD)hEngine->ImageBase && address <= (DWORD)hEngine->ImageBase + hEngine->SizeOfFullImage);
		}
	}

	if (violation) {
		//Native::stopHackingMsg();
		Native::report(AntiTamper::TAMPER_DBGREADWRITE);
		return false;
	}
	return tourMmDbgReadCheck.callOriginal(address);
}

HRESULT HookThreadProcServiceSystemTasks(PVOID pvParam) {
	while (!Teapot::Initialized) {
		if (Teapot::InitError) break;
		Native::Sleep(Native::DecVal(0x328DEB42) /*0x0F*/);
	}

	DetourThreadProcServiceSystemTasks.takeDownDetour();
	return DetourThreadProcServiceSystemTasks.callOriginal(pvParam);
}

void InstallThreadProcServiceSystemTasks() {
	DWORD Address = Teapot::Devkit ? Native::DecVal(0xA50A43C5) /*0x81AB8288*/ : Native::DecVal(0x259F5DC5) /*0x818CEE08*/;
	DetourThreadProcServiceSystemTasks.setupDetour(Address, HookThreadProcServiceSystemTasks);
}

HRESULT SystemHooks_init() {
	string name_krnl = getString(STR_KERNEL);

	Native::PatchInJump((PDWORD)XamExecuteChallengeAddr, (DWORD)Xbox::Challenges::Xam::XamExecuteChallengeHook); //changed DIS
	tourMmDbgReadCheck.setupDetour(name_krnl.cstr, Native::DecVal(0x968BEB42) /*0x1AB*/, MmDbgReadCheckHook);
	NtFile::HandleCheck::install();
	hud::installHooks();
	PatchSpoofGold();

	//stack protection
	if (!Teapot::Devkit) {
		Hypervisor::PokeDWORD(DecQWORD(0x2A34FDFFFBFF0A89) /*0x800001040002C764*/, Native::DecVal(0x248D8B0A) /*0x38600001*/); //jamies old shit
		*(PQWORD)Native::DecVal(0x45E7E2C2) /*0x8007A628*/ = DecQWORD(0x460F80B1FEFFAB21) /*0x386000014E800020*/;
	}

	//fix controller problems
	if (ini::NoKVMode) {
		if (Teapot::Devkit) *(PINT)Native::DecVal(0xDD05FCC2) /*0x80128740*/ = Native::DecVal(0x1D8DEB62) /*0x60000000*/;
		else *(PINT)Native::DecVal(0x097FDFC2) /*0x800F0E64*/ = Native::DecVal(0x1D8DEB62) /*0x60000000*/;
	}

	memcpy(Xbox::Challenges::Xam::IntegrityDigest, (PDWORD)XamExecuteChallengeAddr, Native::DecVal(0x318DEB42) /*0x0C*/);

	Devkit::InstallHooks();
	debug("[SYS] Hooks Initialized");
	return ERROR_SUCCESS;
}

