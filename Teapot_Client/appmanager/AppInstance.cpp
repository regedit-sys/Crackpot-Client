#include "stdafx.h"
#include "AppManager.h"

using namespace AppManager;

#ifdef AppMgrDEBUGInstanceInfo
char Str_E_AppType[5][14] = { "Dashboard", "Title", "Dll", "SysDll", "TITLE_IMPORTS" };
#endif

C_AppInstance::C_AppInstance(std::string ModuleName) {
	HANDLE Handle;
	this->iStatus = E_FAIL;
	this->pLDR = nullptr;
	this->pExID = nullptr;
	if (NT_SUCCESS(Native::XexGetModuleHandle(&ModuleName[0], &Handle))) {
		this->iStatus = InitInstance(Handle);
	}
}

C_AppInstance::C_AppInstance(HANDLE Handle) {
	this->pLDR = nullptr;
	this->pExID = nullptr;
	this->iStatus = InitInstance(Handle);
}

NTSTATUS C_AppInstance::InitInstance(HANDLE Handle) {
	if (!Handle) {
		AppMgrDbg("[AppMgr] Init -> invalid handle");
		return E_FAIL; //S_FAIL's need to be changed to E_FAIL to prevent unhandled behavior.
	}

	if (!(this->pLDR = (PLDR_DATA_TABLE_ENTRY)Handle)) {
		AppMgrDbg("[AppMgr] Init -> pEntry failed");
		return E_FAIL;
	}

	this->hand = Handle;
	this->ImageBase = (DWORD)this->pLDR->ImageBase;
	this->NtHeaderBase = (DWORD)this->pLDR->NtHeadersBase;
	this->Loadflags = this->pLDR->ModuleLoadFlags;
	this->TimeDateStamp = this->pLDR->TimeDateStamp;
	this->Checksum = this->pLDR->CheckSum;
	this->pExID = this->Construct_ExID();
	this->Type = Construct_AppType();
	this->CachedDllNameW = this->GetAppNameW();

	#if defined(DEBUG_PRINT) && defined(AppMgrDEBUGInstanceInfo)
	AppMgrDbg("[AppMgr] ---------------------------------------------------------------------");
	AppMgrDbg("[AppMgr] Instance Created:");
	AppMgrDbg("[AppMgr] Name: %ws", this->pLDR->BaseDllName.Buffer);
	AppMgrDbg("[AppMgr] Path: %ws", this->pLDR->FullDllName.Buffer);
	AppMgrDbg("[AppMgr] Type: %s", this->Type!=-1 ? Str_E_AppType[this->Type] : "Unknown");
	AppMgrDbg("[AppMgr] Load Flags: 0x%08X", this->Loadflags);
	AppMgrDbg("[AppMgr] Flags: 0x%08X", this->pLDR->Flags);
	AppMgrDbg("[AppMgr] Checksum: 0x%08X", this->Checksum);
	AppMgrDbg("[AppMgr] Image Base: 0x%08X", this->pLDR->ImageBase);
	AppMgrDbg("[AppMgr] LDR: 0x%08X\n", this->pLDR);

	if (this->pExID) {
		AppMgrDbg("[AppMgr] pExID Ptr: 0x%08X", (DWORD)pExID);
		AppMgrDbg("[AppMgr] TitleID: 0x%08X", this->TitleID);
		AppMgrDbg("[AppMgr] Version: 0x%08X (%i)", this->Version, this->Version);
	}
	else AppMgrDbg("[AppMgr] Unable to resolve pExID");
	AppMgrDbg("[AppMgr] ---------------------------------------------------------------------\n");
	#endif
	return S_OK;
}

PXEX_EXECUTION_ID C_AppInstance::Construct_ExID() {
	PXEX_EXECUTION_ID pExIDObj = nullptr;
	if (this->pLDR) {
		if (pExIDObj = (PXEX_EXECUTION_ID)RtlImageXexHeaderField(this->pLDR->XexHeaderBase, Native::DecVal(0x2B8DE742) /*0x40006*/)) {
			this->Version = (pExIDObj->Version >> 8) & 0xFF;
			this->TitleID = pExIDObj->TitleID;
		}
	}
	return pExIDObj;
}

E_AppType C_AppInstance::Construct_AppType() {
	switch (this->Loadflags) {
	case XEX_LOADIMG_TYPE_TITLE_DLL:
	case XEX_LOADIMG_FLAG_DLL: return TYPE_APP_DLL;
	case XEX_LOADIMG_FLAG_TITLE_PROCESS: return TYPE_APP_TITLE;
	case XEX_LOADIMG_TYPE_SYSTEM_DLL: return TYPE_APP_SYSDLL;
	case XEX_LOADIMG_FLAG_TITLE_IMPORTS: return TYPE_APP_TITLE_IMPORTS;
	default: return (E_AppType)-1;
	}
}

HRESULT C_AppInstance::InstallHooks() {
	if (FAILED(PatchKrnlImport(this, 0x7A8BEB42 /*0x197*/, Hooks::Security::XexGetProcedureAddressHook))) return E_FAIL;
	if (FAILED(PatchKrnlImport(this, 0x758BEB42 /*0x198*/, Hooks::XexLoadExecutable))) return E_FAIL;
	if (FAILED(PatchKrnlImport(this, 0x7C8BEB42 /*0x199*/, Hooks::XexLoadImage))) return E_FAIL;
	return ERROR_SUCCESS;
}

HRESULT C_AppInstance::Init() {
	if (this->Valid()) {
		if (FAILED(InstallHooks())) AppMgrDbg("[AppMgr] Failed to install hooks for '%ws'", this->CachedDllNameW.c_str());
		C_AppEventHandler(this);
		return ERROR_SUCCESS;
	}
	AppMgrDbg("[AppMgr] C_AppEventHandler pIstnace or LDR Table is invalid '%ws'", this->CachedDllNameW.c_str());
	return E_FAIL;
}

std::wstring C_AppInstance::GetAppNameW(bool FullPath) {
	std::wstring bufferOut = L"\0";
	if (this->Valid()) {
		UNICODE_STRING UniStr = FullPath ? this->pLDR->FullDllName : this->pLDR->BaseDllName;
		if (UniStr.Length > 0) bufferOut = std::wstring(UniStr.Buffer);
	}
	return bufferOut;
}

std::string C_AppInstance::GetAppName(bool FullPath) {
	std::string bufferOut = "\0";
	if(this->Valid()){
		UNICODE_STRING UniStr = FullPath ? this->pLDR->FullDllName : this->pLDR->BaseDllName;
		if (UniStr.Length > 0) {
			char *buffer = new char[UniStr.Length + 1]; 
			wcstombs(buffer, UniStr.Buffer, UniStr.Length +1); //aded +1
			bufferOut = std::string(buffer);
			delete[] buffer;
		}
	}
	return bufferOut;
}

void C_AppInstance::RegisterActiveTitle() {
	memcpy(&AppMgr.ActiveTitle, this, sizeof AppManager::C_AppInstance);
	AppMgr.ActiveTitle.CachedDllNameW = this->CachedDllNameW;
}

bool C_AppInstance::IsDefaultApp() {
	//Is this dash.xex? 
	if (AppMgrIsTitleRealDash(this)) return true;
	if (AppMgrIsTitleXShell(this)) return true;

	if (!Teapot::Devkit && DLaunch.Vars.bDefaultPathSet) {
		std::string PathStr(this->GetAppName(true));
		if (PathStr.length() > 0) {
			if (PathStr.find(DLaunch.Vars.sDefaultPath) != std::string::npos) {
				return true;
			}
		}
	}
	return false;
}

bool C_AppInstance::IsWhiteListedApp() {
	std::array<WCHAR*, 4> PathDelimiters = { L"fsd", L"freestyle", L"free style", L"dashlaunch" };
	std::array<DWORD, 7> KnownChecksums = {
		0x01024258 /*fsd1*/,
		0x01022EA5 /*fsd2*/,
		0x0105A2F2 /*fsd3*/,
		0x00C160E9 /*aurora*/,
		0x0022545E /*dl319*/,
		0x00229439 /*dl320*/,
		0x00223F1E /*dl321*/
	};

	//Quick checksum lookup, not guaranteed for apps besides Dashlaunch 
	for (size_t i = 0; i < KnownChecksums.size(); ++i) {
		if (KnownChecksums[i] == this->Checksum) return true;
	}

	//checking for an invalid ExID & Image base will make our results more accurate. FSD & Aurora do not have an ExID.
	if (!this->pExID && AppMgrIsTitleApplication(this)) {
		//The only consistent data I could find in all the FSD revisions
		if (strncmp((PCHAR)this->ImageBase + 0x98C, "UTTERANCES2", 11) == 0) {
			return true;
		}

		//Last but not least, let's iterate over our array of keywords and see if the path contains any
		wstring fileName(this->pLDR->FullDllName.Buffer);
		transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);
		for (size_t i = 0; i < PathDelimiters.size(); ++i) {
			if (wcsstr(fileName.c_str(), PathDelimiters[i])) {
				return true;
			}
		}
	}
	return false;
}


bool C_AppInstance::Valid() {
	/*if (this->pLDR) {
		PVOID NtHeaderBase = nullptr;
		PLDR_DATA_TABLE_ENTRY TmpPLDR;
		if ((NtHeaderBase = Native::XexPcToFileHeader((PVOID)ImageBase, &TmpPLDR)) != nullptr) {
			if ((DWORD)NtHeaderBase == this->NtHeaderBase) { 
				return (TmpPLDR->CheckSum == this->Checksum && this->TimeDateStamp == TmpPLDR->TimeDateStamp); //validate the timestamp and checksum
			}
		}
	}else AppMgrDbg("[AppMgr] Instance is NOT valid!");
	return false;*/

	 __try {
		if (!this->pLDR) return false;
		if (!MmIsAddressValid((PVOID)this->pLDR)) return false;
		if (!MmIsAddressValid(&this->pLDR->CheckSum)) return false;
		return (pLDR->CheckSum == this->Checksum);
	}__except (true) {
		return false;
	}
	return false;
}