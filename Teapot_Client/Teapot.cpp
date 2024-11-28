#include "stdafx.h"
#include "common.h"
#include "iniConfig.h"
#include "exports.h"
#include <unordered_map>
#include "xbox/XamExecuteChallenge.h"
#include "Updater.h"

AppManager::C_Manager AppMgr;
C_DLaunch DLaunch;

PLDR_DATA_TABLE_ENTRY Teapot::handle;
bool Teapot::Initialized = false;
bool Teapot::InitError = false;
bool Teapot::Devkit;
bool Teapot::Freemode;
short Teapot::AuthFlag = AUTHFLAG_NOTCONNECTED;
bool Teapot::UpdateNotified = false;
XNADDR Teapot::IPAddrIntrnl;
byte Teapot::ImageDigest[0x10];
byte Teapot::Session[0x10];
byte Teapot::CachedOriginalDVDKey[0x10];

HRESULT Teapot::MountPath() {
	char *TmpPath = new char[0x180];
	if (CreateSymbolicLink(DRIVE_TEAPOT, SplitFilename(wcharToChar(Teapot::handle->FullDllName.Buffer, TmpPath)), TRUE) != ERROR_SUCCESS) {
		debug("Failed to create symbolic link");
		delete[] TmpPath;
		return E_FAIL;
	}

	delete[] TmpPath;
	CreateSymbolicLink("HDD:\\", "\\Device\\Harddisk0\\Partition1", TRUE);
	CreateSymbolicLink("USB:\\", "\\Device\\Mass0", TRUE);
	debug_newsession();
	return ERROR_SUCCESS;
}

HRESULT Teapot::SetModuleHash() {
	MemoryBuffer mbteapot;
	if (cReadFile(GetSelfPath().cstr, mbteapot) != TRUE) { 
		FNotify(L"Crackpot - Error [0x7CA0] consult admin for fix.\nRestarting console...").error(true);
		debug("Teapot::SetModuleHash() Failed!");
		return E_FAIL;
	}

	Native::XeCryptHmacSha(Teapot::Session, 16, mbteapot.GetData(), mbteapot.GetDataLength(), NULL, 0, NULL, 0, Teapot::ImageDigest, Native::DecVal(0x2D8DEB42) /*0x10*/);
	return ERROR_SUCCESS;
}

HRESULT Teapot::ProcessSecData() {
	DWORD dwSize;
	PVOID RC4Key;
	debug("[SYS] Security Data Initalized!");
	if (!Native::XGetModuleSection(AppMgr.Self.hand, getString(STR_SPFG).cstr, &RC4Key, &dwSize)) return E_FAIL;
	if (Native::XGetModuleSection(AppMgr.Self.hand, getString(STR_SECDATA).cstr, &pSecData, &dwSize)) {
		Native::XeCryptRc4((PBYTE)RC4Key, Native::DecVal(0xF98CEB42) /*0x14*/, (PBYTE)pSecData, SECDATA_SIZE);
		return ERROR_SUCCESS;
	}
	return E_FAIL;
}

HRESULT Teapot::init() {
	DLaunch.SabotageXHTTP();
	Config::Apply();

	//Hypervisor
	if (FAILED(Hypervisor::InvokedInit())) return E_FAIL;

	//System
	if (FAILED(ProcessSecData())) return E_FAIL;
	if (FAILED(SystemHooks_init())) return E_FAIL;
	if (FAILED(KV::Init())) return E_FAIL;

	//Networking
	if (FAILED(NET::CLIENT::Initialize())) return E_FAIL;
	if (FAILED(NET::LOGON())) return E_FAIL;
	Teapot::Initialized = true;

	return ERROR_SUCCESS;
}


enum XLanguagesEx {
	LANGUAGE_ENGLISH = 1,
	LANGUAGE_SPANISH = 5,
	LANGUAGE_BRAZILIAN = 9,
};

//this is some debug dump info for some language pack stuff i was making. really easy to do @seaworld just lazy
//problem is, the whole dashboard is rendered from the website, using an xml manifest, but it's in english. we have many foreign users so it can be an issue
void MiscDebugDump() {
	/*
	DWORD start = offsetof(_KEY_VAULT, PrimaryActivationKey);
	DWORD end = offsetof(_KEY_VAULT, OtherXsm3Device3p2desKey2);
	debug("KV_S::PrimaryActivationKey: 0x%X", start);
	debug("KV_S::OtherXsm3Device3p2desKey2: 0x%X", end);
	debug("Block size between members: 0x%X", end - start);
	debug("[INFO] API Macro Default Local = %i", XGetLocale());
	debug("[INFO] API Macro Default Language = %i", XGetLanguage());
	*/
}


///////////////////////////scrap
	//char buffer[100];
	//int test = _snprintf(buffer, sizeof buffer, DLaunch.GetPluginsList().cstr);
	//debug("Plugin List: %s | %i, %i", buffer, test, strlen(buffer));
	//debug("wtf??");