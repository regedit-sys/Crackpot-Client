#include "stdafx.h"
#include "AppManager.h"

#include "../hud/dashboard/DashboardHooks.h"
#include "../cheats/CheatEngine.h"
#include "../GTAScratch.h"

void OnDashboardLoad();

/////////////////////////////////ALL GARBAGE BELOW THIS LINE//////////////////////////////////////
detour<void> UnknownCock4;
detour<void> UnknownCock6;
detour<DWORD> Ilikecocks;



bool notified = false;
DWORD test(DWORD r3, DWORD r4, DWORD r5, DWORD r6, DWORD r7, DWORD r8, DWORD r9) {
	if (r3 == 0 && !notified) {
		debug_sys("registers: 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X", r3, r4, r5, r6, r7, r8, r9);
		debug_sys("crash intercepted!");
		notified = true;
		return 0;
	}
	return Ilikecocks.callOriginal(r3, r4, r5, r6, r7, r8, r9);
}

void dumbdumb4(void* r3, unsigned short* r4, bool r5, unsigned char* r6, unsigned long r7) {
	if (!r5) {
		unsigned char pid = r6[0x29];
		if (pid >= 0x00 && pid < 0x10) {
			debug("Retard shit 4: condition notice PID: 0x%08X", pid);
			if (r4[0x1B] < 0 || r4[0x1B] > 13) {
				debug("Retard shit 4: flag would be true here | r4:0x%08X", r4);
				return;
			}
		}
		UnknownCock4.callOriginal(r3, r4, r5, r6, r7);
	}
}

// This removes the block
void dumbdumb6(unsigned char* r3) {
	unsigned char pid = r3[0x29];
	if (pid >= 0x00 && pid < 0x10) {
		debug("Retard shit 4: flag would be true here");
	}
	UnknownCock6.callOriginal(r3);
}

/////////////////////////////////////////////////////////////////////////////////



void AppManager::Events::OnTitleLoad(AppManager::C_AppInstance *pInstance) {
	AppMgrDbg("[AppMgr] Title Loaded -> %ws", pInstance->CachedDllNameW.cstr);

	pInstance->RegisterActiveTitle();
	CheatEngine::Dispose();

	if (pInstance->IsDefaultApp()) {
		if (AppMgrIsTitleRealDash(pInstance)) {
			dashboard::hooks::install(pInstance);
		}
		if (!AppMgr.HasDashLoaded) Native::CreateThreadExQ(OnDashboardLoad);
		AppMgrDbg("[AppMgr] Default app has loaded.");
		return;
	}

	if (Teapot::AuthFlag != AUTHFLAG_ACTIVE) 
		return;
	
	CheatEngine::Init(pInstance);


	if (pInstance->TitleID == GTA_V) {
		debug("GTA Loaded");

		SetupHooks();
		return;
		Ilikecocks.setupDetour(0x835BFA88, (PDWORD)test);
		//UnknownCock4.setupDetour(0x830EA8B8, (PDWORD)dumbdumb4);
		//UnknownCock6.setupDetour(0x82FEA388, (PDWORD)dumbdumb6);
		debug("GTA Protections hook set");
	}
}

void AppManager::Events::OnDllLoad(AppManager::C_AppInstance *pInstance) {
	AppMgrDbg("[AppMgr] Dll Loaded -> %ws", pInstance->CachedDllNameW.cstr);
}

void OnDashboardLoad() {
	Native::Sleep(Native::DecVal(0x8589EB42) /*1000*/);
	AppMgr.HasDashLoaded = true;

	while (!Teapot::Initialized && !Teapot::InitError) Native::Sleep(0);

	switch (Teapot::AuthFlag) {
		case AUTHFLAG_EXPIRED: {
			debug("[INFO] UI_Expired Has Allocated");
			UI_ExpiredMsg(); 
			break;
		}
		case AUTHFLAG_NOEXIST: {
			if (DLaunch.Vars.bDefaultPathSet) {
				debug("Wait 6 seconds ran!");
				Native::Sleep(6000);
			}
			debug("[INFO] UI_REGISTER Has Allocated");
			REGISTER_EMAIL(L"");
			break;
		}
	}
}