#include "stdafx.h"
#include "AppManager.h"

using namespace AppManager;

DWORD AppManager::CheatSupportedTitles[9] = {
		0x415607E6,  0x415608C3, 0x415608CB, 0x415608FC, 0x4156081C, 0x4156091D, 0x41560817, 0x41560855, 0x41560914
};

void C_Manager::InitSpoofedTitle() {
	DWORD KrnlVersion = ((XboxKrnlVersion->Major & 0xF) << 28) | ((XboxKrnlVersion->Minor & 0xF) << 24) | (XboxKrnlVersion->Build << 8) | (XboxKrnlVersion->Qfe);
	Mem::Null(&this->SpoofedTitle, sizeof(XEX_EXECUTION_ID));
	this->SpoofedTitle.Version = KrnlVersion;
	this->SpoofedTitle.BaseVersion = KrnlVersion;
	this->SpoofedTitle.TitleID = Native::DecVal(0x748510FF) /*0xFFFE07D1*/;
}

NTSTATUS C_Manager::InitCommonInstances() {
	if (NTSTATUS((this->Self = C_AppInstance(this->hController)).iStatus)) {
		debug("[AppMgr] [InitCommon] Controller Failed.");
		return S_FAIL;
	}

	if (NTSTATUS((this->Kernel = C_AppInstance(getString(STR_KERNEL))).iStatus)) {
		debug("[AppMgr] [InitCommon] Kernel Failed.");
		return S_FAIL;
	}

	if (NTSTATUS((this->Xam = C_AppInstance(getString(STR_XAM))).iStatus)) {
		debug("[AppMgr] [InitCommon] Xam Failed.");
		return S_FAIL;
	}

	debug("[AppMgr] [InitCommon] Success!");
	return S_OK;
}

NTSTATUS C_Manager::InitLoadModuleHooks() {
	if (FAILED(PatchKrnlImport(&this->Xam, 0x758BEB42 /*0x198*/, Hooks::XexLoadExecutable))) return S_FAIL;
	if (FAILED(PatchKrnlImport(&this->Xam, 0x7C8BEB42 /*0x199*/, Hooks::XexLoadImage))) return S_FAIL;
	if (FAILED(PatchKrnlImport(&this->Xam, 0x798BEB42 /*0x194*/, Hooks::XexCheckExecutablePrivilegeHook))) return E_FAIL;
	if (FAILED(PatchKrnlImport(&this->Xam, 0x168CEB42 /*0x12B*/, Hooks::RtlImageXexHeaderFieldHook))) return E_FAIL; 
	return S_OK;
}

void C_Manager::Install(HANDLE Controller) {
	this->hController = Controller;
	this->HasDashLoaded = false;

	InitSpoofedTitle();
	if (FAILED(this->InitCommonInstances())) return;
	if (FAILED(this->InitLoadModuleHooks())) {
		debug("[AppMgr] Manager - Failed to Init hooks");
		return;
	}
	debug("[AppMgr] Installed");
}


C_AppInstance *AppManager::XexPcToAppInstance(DWORD Address) {
	PLDR_DATA_TABLE_ENTRY TmpLDR;
	if (Native::XexPcToFileHeader((PVOID)Address, &TmpLDR) != 0) {
		return new C_AppInstance(TmpLDR);
	}
	return nullptr;
}

NTSTATUS C_Manager::LoadNewFromMemory(C_AppInstance *pInstance, PBYTE pImage) {
	return S_OK;
}