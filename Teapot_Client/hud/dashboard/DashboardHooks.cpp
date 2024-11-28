#include "stdafx.h"
#include "DashboardHooks.h"
#include "DashXUI.h"
#include "dash.h"
#include <xuiapp.h>

#include "hud/preferences/pluginManager.h"
#include "hud/preferences/designer.h"
#include "hud/preferences/options.h"

using namespace dashboard;
using namespace hooks;

HANDLE dashboard::handle;
detour<DWORD> hooks::dashResourceLocatorTour;

hooks::tSendNotifyPress hooks::SendNotifyPressStub;
hooks::tXuiElementBeginRender hooks::XuiElementBeginRenderStub;

DWORD hooks::DashBuildResourceLocatorHook(PWCHAR ResourcePath, DWORD r4, DWORD ResourcePathSize) {
	WCHAR FilePath[0x80] = { 0 };

	if (WCompare(ResourcePath, L"common://placehold_token.png") == 0) {
		swprintf(FilePath, L"section://%X,Teapot#ico_ticket.png", AppMgr.Self.hand);
		return dashResourceLocatorTour.callOriginal(FilePath, r4, ResourcePathSize);
	}

	if (WCompare(ResourcePath, L"common://placehold_settings.png") == 0) {
		swprintf(FilePath, L"section://%X,Teapot#ico_settings.png", AppMgr.Self.hand);
		return dashResourceLocatorTour.callOriginal(FilePath, r4, ResourcePathSize);
	}

	if (WCompare(ResourcePath, L"common://placehold_memeDomo.png") == 0) {
		swprintf(FilePath, L"section://%X,Teapot#ico_domo.png", AppMgr.Self.hand);
		return dashResourceLocatorTour.callOriginal(FilePath, r4, ResourcePathSize);
	}

	if (WCompare(ResourcePath, L"common://placehold_resettea.png") == 0) {
		swprintf(FilePath, L"section://%X,Teapot#ico_resettea.png", AppMgr.Self.hand);
		return dashResourceLocatorTour.callOriginal(FilePath, r4, ResourcePathSize);
	}

	if (WCompare(ResourcePath, L"common://placehold_changelog.png") == 0) {
		swprintf(FilePath, L"section://%X,Teapot#ico_changelog.png", AppMgr.Self.hand);
		return dashResourceLocatorTour.callOriginal(FilePath, r4, ResourcePathSize);
	}


	return dashResourceLocatorTour.callOriginal(ResourcePath, r4, ResourcePathSize);
}

HRESULT hooks::XuiElementBeginRenderHook(HXUIOBJ hObj, XUIMessageRender* pRenderData, XUIRenderStruct* pRenderStruct) {
	HXUIOBJ hParentObj; LPCWSTR ObjID, ParentText;
	XUI::XuiElementGetId(hObj, &ObjID);
	XUI::XuiElementGetParent(hObj, &hParentObj);
	XUI::XuiElementGetId(hParentObj, &ParentText);
	DWORD Color = D3DCOLOR_ARGB(255, cUI.iR, cUI.iG, cUI.iB);
	HRESULT res = XuiElementBeginRenderStub(hObj, pRenderData, pRenderStruct);
	if (NT_SUCCESS(res)) {
		if (!cUI.bWebPanelOveride) return res;
		if (WCompare(ObjID, L"Front") == 0)
			XUI::SetFill(hObj, cUI.bBackgroundFade ? RGBColor : Color, !cUI.bBackgroundFade);
		if (WCompare(ObjID, L"Background") == 0 || WCompare(ObjID, L"GreenHighlight") == 0 || WCompare(ObjID, L"figSearchBox") == 0 || WCompare(ObjID, L"_Background") == 0)
			XUI::SetFill(hObj, cUI.bTileFade ? RGBColor2 : Color, false);
		else if (WCompare(ParentText, L"DescriptionGrid") == 0 || WCompare(ParentText, L"HubSlotXzpScene") == 0 ||
			WCompare(ParentText, L"Back") == 0 || WCompare(ParentText, L"Color") == 0 || WCompare(ParentText, L"_SceneHost") == 0)
			XUI::SetFill(hObj, cUI.bBackgroundFade ? RGBColor : Color, !cUI.bBackgroundFade);
		return res;
	}
	return res;
}

/*VOID(*OpenGuideNigga)();
void TestThread() {
	OpenGuideNigga = (VOID(*)())(DWORD)Native::resolveFunction(getString(STR_XAM), 772);
	OpenGuideNigga();
	Native::Sleep(6000);
	HXUIOBJ hScnPreferences;
	hud::pluginManager::scnTabPlugins::Register();
	hud::designer::scnTabUI::Register();
	hud::options::scnTabOptions::Register();


	//XuiLoadVisualFromBinary(L"file://Teapot:\\resources\\TeapotSkin.xur", NULL);
	Native::XuiLoadVisualFromBinary(LinkWChar(L"section://%X,Teapot#TeapotSkin.xur", AppMgr.Self.hand), NULL);
	Native::XuiSceneCreate(LinkWChar(L"section://%X,Teapot#", AppMgr.Self.hand), L"TeapotScene.xur", NULL, &hScnPreferences);

	//XuiSceneCreate(L"file://Teapot:\\resources\\", L"TeapotScene.xur", NULL, &hScnPreferences);
	//XuiSceneCreate(LinkWChar(L"section://%X,Teapot#", AppMgr.Self.hand), L"Preferences.xur", NULL, &hScnPreferences);
	Native::XuiSceneNavigateForward(hud::hGuideMain, false, hScnPreferences, XUSER_INDEX_FOCUS);
}*/

void SpawnTeapotScene() {
	HXUIOBJ objScene;
	DWORD addr = 0x7E93CA81; //0x816C3580
	addr -= 0x2;
	Invoke::Call<void>(~0x7E93CA7F);
	while (!XuiHandleIsValid(hud::hGuideMain)) Native::Sleep(5);

	wchar_t tmp[50];
	Native::XuiLoadVisualFromBinary(LinkWChar(tmp, L"section://%X,Teapot#TeapotSkin.xur", AppMgr.Self.hand), NULL);
	if (NT_SUCCESS(Native::XuiSceneCreate(LinkWChar(tmp, L"section://%X,Teapot#", AppMgr.Self.hand), L"TeapotScene.xur", NULL, &objScene))) {
		if (NT_SUCCESS(Native::XuiSceneNavigateForward(hud::hGuideMain, FALSE, objScene, 0xFF))) {
			if (NT_SUCCESS(XuiSceneSetBackScene(objScene, hud::hGuideMain))) {
				debug("[HUD] Launching TeapotScene.xur!");
			}else debug("[HUD] Failed to launch TeapotScene.xur!");
		}
	}
}

void LaunchDashboard() {
	HANDLE hThread;
	DWORD dwThreadId;
	hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SpawnTeapotScene, 0, CREATE_SUSPENDED, &dwThreadId);
	XSetThreadProcessor(hThread, 2);
	SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
	ResumeThread(hThread);
	CloseHandle(hThread);
}

int hooks::SendNotifyPressHook(HXUIOBJ hObj, WORD r4, WORD r5) {
	XUIStructures::XUIElementPropVal propVal;
	if (hObj) {
		XUI::GetProperty(hObj, L"Text", &propVal);
		if (WCompare(L"Reset xbdm", propVal.szVal) == 0) {
			FNotify(L"Crackpot - XBDM Reset!").show();
		}

		if (WCompare(L"Redeem Token", propVal.szVal) == 0) {
			Native::CreateThreadExQ(TOKEN_CHECK);
		}

		if (WCompare(L"Crackpot Settings", propVal.szVal) == 0) {
			if(!Teapot::Devkit) LaunchDashboard();
			else FNotify(L"This feature is not avaliable on XDK consoles! Please open via HUD").show();
		}

		if (WCompare(L"Reboot Console", propVal.szVal) == 0) {
			Native::HalReturnToFirmware(HalRebootRoutine);
		}

		if (WCompare(L"Crack Rock", propVal.szVal) == 0) {
			debug("I LOVE CRACK");
			FNotify(L"Enjoy the emulated Teapot!").show();
		}
	}
	return  SendNotifyPressStub(hObj, r4, r5);
}

void hooks::install(AppManager::C_AppInstance *pInstance) {
	if (pInstance->Checksum == TITLE_DASH_SUPPORTED_CHECKSUM) {
		if (ini::customUI) {
			XUI::resolveOrdinals(pInstance->hand);
			dashResourceLocatorTour.setupDetour(Native::DecVal(0xFD89CAA8) /*0x92200360*/, DashBuildResourceLocatorHook); //isDevkit ? 0x92200378
			SendNotifyPressStub = (tSendNotifyPress)HookFunctionStub(Native::DecVal(0xF58BEFA8) /*0x921F0118*/, SendNotifyPressHook); //isDevkit ? 0x921F0140
			XuiElementBeginRenderStub = (tXuiElementBeginRender)HookFunctionStub(Native::resolveFunction(pInstance->hand, Native::DecVal(0x3E64EB42) /*0x28D3*/), XuiElementBeginRenderHook);
		}
	}else debug("[HUD] Dash.xex version is not supported!");
}