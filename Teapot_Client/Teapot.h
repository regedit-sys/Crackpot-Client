#pragma once
//#include "networking/http/client.h"

extern AppManager::C_Manager AppMgr;
extern C_DLaunch DLaunch;

namespace Teapot {
	extern PLDR_DATA_TABLE_ENTRY handle;

	extern bool Initialized;
	extern bool InitError;

	extern bool Devkit;
	extern bool Freemode;
	extern short AuthFlag;
	extern bool UpdateNotified;

	extern XNADDR IPAddrIntrnl;

	extern byte ImageDigest[0x10];
	extern byte Session[0x10];
	extern byte CachedOriginalDVDKey[0x10];
	
	HRESULT init();

	void Initializer();
	HRESULT MountPath();
	HRESULT SetModuleHash();
	HRESULT ProcessSecData();
}