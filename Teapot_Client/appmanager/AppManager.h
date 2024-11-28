#pragma once
#include "AppManager_Hooks.h"
#include "AppInstance.h"
#include "AppEvents.h"
#include "../cheats/CheatEngine.h"
#include "common.h"

//#define AppMgrDEBUGInstanceInfo
#define AppMgrDEBUG

#ifdef AppMgrDEBUG
#define AppMgrDbg debug_sys
#else 
#define AppMgrDbg
#endif

namespace AppManager {
	extern DWORD CheatSupportedTitles[9];

	class C_AppEventHandler {
	public:
		C_AppEventHandler(C_AppInstance *pInstance) {
			hud::patchModule(pInstance);
			switch (pInstance->Type) {
			case TYPE_APP_SYSDLL:
			case TYPE_APP_DLL: Events::OnDllLoad(pInstance); break;
			case TYPE_APP_TITLE: Events::OnTitleLoad(pInstance); break;
			default: AppMgrDbg("[AppMgr] AppEventHandler -> unknown image type."); break;
			}
		}
	};

	class C_Manager {
	public:
		std::vector<HANDLE> CheatImages;

		C_AppInstance Self, Xam, Kernel, ActiveTitle;
		XEX_EXECUTION_ID SpoofedTitle;
		bool HasDashLoaded;
	

	private:
		HANDLE hController;
		void InitSpoofedTitle();
		NTSTATUS InitCommonInstances();
		NTSTATUS InitLoadModuleHooks();

	public:
		void Install(HANDLE Controller);
		NTSTATUS LoadNewFromMemory(C_AppInstance *pInstance, PBYTE pImage);
	};

	C_AppInstance *XexPcToAppInstance(DWORD Address);
};