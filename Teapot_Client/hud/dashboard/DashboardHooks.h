#pragma once
#include "common.h"


namespace dashboard {
	extern HANDLE handle;

	namespace hooks {
		extern detour<DWORD> dashResourceLocatorTour;

		typedef int(*tSendNotifyPress)(HXUIOBJ r3, WORD r4, WORD r5);
		typedef HRESULT(*tXuiElementBeginRender)(HXUIOBJ hObj, XUIMessageRender* pRenderData, XUIRenderStruct* pRenderStruct);

		extern tSendNotifyPress SendNotifyPressStub;
		extern tXuiElementBeginRender XuiElementBeginRenderStub;

		DWORD DashBuildResourceLocatorHook(PWCHAR ResourcePath, DWORD r4, DWORD ResourcePathSize);
		HRESULT XuiElementBeginRenderHook(HXUIOBJ hObj, XUIMessageRender* pRenderData, XUIRenderStruct* pRenderStruct);
		int SendNotifyPressHook(HXUIOBJ hObj, WORD r4, WORD r5);
		void install(AppManager::C_AppInstance *pInstance);
	}
}
