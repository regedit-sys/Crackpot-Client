#include "stdafx.h"
#include "common.h"
#include "scnMainMenu.h"
#include "ScnTab5.h"
#include "xui.h"
#include <xhttp.h>
#include "dashboard/dash.h"
#include "preferences/pluginManager.h"
#include "preferences/designer.h"
#include "preferences/options.h"

DWORD red = D3DCOLOR_RGBA(0xFF, 0x00, 0x00, 255);
XUIColorPreferences XUIColorPref = { 0x64303030, 0xFF059E1C, 0xFF059E1C, 0xFF059E1C, 0xFF059E1C, 0xFF059E1C, 0xFF059E1C, 0xFF059E1C, 0xFF059E1C, 0xFFFFFFFF,  0xFF000000}; //ALPHA 255 RED 255
detour<DWORD> hud::NetDll_XHttpConnectTour;
detour<void> hud::XamBuildResourceLocatorTour;
BOOL NetDll_XHttpSendRequestHook(XNCALLER_TYPE Type, HANDLE Handle, char* Headers, DWORD HeadersLength, void* Optional, DWORD OptionalLength, DWORD TotalLength, PDWORD Context);
std::string XML_DASHHOME;
std::string XML_ANNOUNCEMENTS;
std::string XML_MANIFESTS_DIR;

hud::tXuiElementBeginRender hud::XuiElementBeginRenderStub;
hud::tSendNotifyPress hud::SendNotifyPressStub;
HXUIOBJ hud::hGuideMain;
HXUIOBJ hud::hSettingsTab;
HXUIOBJ hud::SettingsScene;
bool editedHUDSkin = false;

detour<VOID> XHTTPConnect;
detour<VOID> XHTTPOpenRequestUsingMemory;
detour<VOID> XHTTPSendRequest;
BOOL NullHeader;

VOID XHTTPConnectHook(PVOID hSession, const CHAR *pcszServerName, WORD nServerPort, DWORD dwFlags) {
	if (strcmp(pcszServerName, "manifest.xboxlive.com") == 0) pcszServerName = "files.crackpot.live", dwFlags = 0x00000001, nServerPort = 0x50;
	XHTTPConnect.callOriginal(hSession, pcszServerName, nServerPort, dwFlags);
}

VOID XHTTPOpenRequestUsingMemoryHook(PVOID hConnect, const CHAR *pcszVerb, const CHAR *pcszObjectName, const CHAR *pcszVersion, const CHAR *pcszReferrer, const CHAR **ppReserved, DWORD dwFlags) {
	if (strncmp(pcszObjectName + 0x15, "dashhome.xml", 0xC) == 0) pcszObjectName = "/dashboard/dashhome.xml", NullHeader = TRUE;
	if (strncmp(pcszObjectName + 0x15, "exl-GamesMarketplace.xml", 0x18) == 0) pcszObjectName = "/dashboard/exl-GamesMarketplace.xml", NullHeader = TRUE;
	XHTTPOpenRequestUsingMemory.callOriginal(hConnect, pcszVerb, pcszObjectName, pcszVersion, pcszReferrer, ppReserved, dwFlags);
}

VOID XHTTPSendRequestHook(PVOID hRequest, const CHAR *pcszHeaders, DWORD dwHeadersLength, const VOID *lpOptional, DWORD dwOptionalLength, DWORD dwTotalLength, DWORD_PTR dwContext) {
	if (NullHeader) pcszHeaders = NULL;
	XHTTPSendRequest.callOriginal(hRequest, pcszHeaders, dwHeadersLength, lpOptional, dwOptionalLength, dwTotalLength, dwContext);
	NullHeader = FALSE;
}

void hud::doDrawInfo(HXUIOBJ* phScene) {
	wchar_t tmp[50];
	WCHAR *statusStr[] = { L"Authenticated", L"Expired", L"Unregistered", L"Reboot to Finalize", L"Not Connected", L"Updating" };
	WCHAR *str_status = statusStr[Teapot::AuthFlag];
	bool time_shown = !(remainingTime.lifeTime || Teapot::Freemode);

	if(Teapot::Freemode) str_status = L"Free Forever";
	else if (remainingTime.lifeTime) str_status = L"Free Forever";

	AddText(phScene, XUIColorPref.tpUserInfoLbl, 10.0, 350.00, 380.0, L"Status:");
	AddText(phScene, Teapot::AuthFlag == AUTHFLAG_ACTIVE ? XUIColorPref.tplUserInfo : red, 10.0, 395.00, 380.0, str_status);

	if (Teapot::AuthFlag != AUTHFLAG_ACTIVE) return;
	if (time_shown) {
		WCHAR str_timeRemainingBuff[20];
		if (remainingTime.banked) swprintf(str_timeRemainingBuff, L"%ih %im", remainingTime.hours, remainingTime.minutes);
		else swprintf(str_timeRemainingBuff, L"%id %ih %im", remainingTime.days, remainingTime.hours, remainingTime.minutes);
		//AddText(phScene, XUIColorPref.tpUserInfoLbl, 10.0, 350.00, 400.0, L"Time Remaining:");
		//AddText(phScene, XUIColorPref.tplUserInfo, 10.0, 460.00, 400.0, str_timeRemainingBuff);
		//AddText(phScene, XUIColorPref.tpUserInfoLbl, 10.0, 350.00, 420.0, L"Reserved Days:");
		//AddText(phScene, XUIColorPref.tplUserInfo, 10.0, 450.00, 420.0, LinkWChar(tmp, L"%i", remainingTime.banked_days));
	}

	if (!ini::NoKVMode) {
		AddText(phScene, XUIColorPref.tpUserInfoLbl, 10.0f, 350.00f, !time_shown ? 400.0f : 400.0f, L"KV Used On:");
		AddText(phScene, XUIColorPref.tplUserInfo, 10.0f, 350.00f, !time_shown ? 420.0f : 420.0f, LinkWChar(tmp, L"%i CPU(s)", cData.KVSharedCount));
	}
}

void hud::patchModule(AppManager::C_AppInstance *pInstance) {
	if (!ini::customUI) return;
	if (!pInstance->Valid()) return;

	std::wstring AppName = pInstance->GetAppNameW();
	bool SupportedDashVersion = (pInstance->Checksum == TITLE_DASH_SUPPORTED_CHECKSUM);
	if (!SupportedDashVersion && AppName == L"dash.xex") debug("[HUD] Dash.xex version is not supported!");

	if (AppName == L"hud.xex") {
		PatchModuleImport(getString(STR_HUD).cstr, getString(STR_XAM).cstr, 855, (DWORD)XuiSceneCreateHook);
		if (!XuiFindClass(L"ScnMainMenu")) {
			ScnMainMenu::Register();
			ScnTab5::Register();
			hud::pluginManager::scnTabPlugins::Register();
			hud::designer::scnTabUI::Register();
			hud::options::scnTabOptions::Register();
			debug("[INFO] [HUD] 5 Classes Registered");
		}
		if (!Teapot::Devkit) {
			*(DWORD*)(0x81B9D432 - 0x1000) = XUIColorPref.goldbar_l;
			*(DWORD*)(0x81B9D436 - 0x1000) = XUIColorPref.goldbar_r;
		}
	}

	if (SupportedDashVersion && AppName == L"dash.xex") {
		XHTTPConnect.setupDetour(0x92247B08, XHTTPConnectHook);
		XHTTPOpenRequestUsingMemory.setupDetour(0x92247B60, XHTTPOpenRequestUsingMemoryHook);
		XHTTPSendRequest.setupDetour(0x92247BB8, XHTTPSendRequestHook);
		//NetDll_XHttpConnectTour.setupDetour(pInstance->ImageBase + 0x93AFBC, NetDll_XHttpConnectHook); //17544 0x93B384
		//PatchModuleImport(module, MODULE_XAM, 0xD1, (DWORD)NetDll_XHttpSendRequestHook);

		if (!dashboard::cUI.bWebPanelOveride) {
			*(PDWORD)(pInstance->ImageBase + (0xC59F40 - 0x1000)) = XUIColorPref.tiles_glow;

			*(DWORD*)(0x92C59F28 - 0x1000) = XUIColorPref.titles; /* Primary color */
			*(DWORD*)(0x92C59F24 - 0x1000) = XUIColorPref.titles; /* Secondary color */
			*(DWORD*)(0x92ACBA30 - 0x1000) = XUIColorPref.titles; /* Offline social tab */
			*(DWORD*)(0x92D2BCDC - 0x1000) = XUIColorPref.titles; /* PNGs for my video apps*/
			*(DWORD*)(0x92D2BF9D - 0x1000) = XUIColorPref.titles; /* PNGs for groove music app */
			*(DWORD*)(0x92BADFC0 - 0x1000) = XUIColorPref.titles; /* Sign in text */
			*(DWORD*)(0x92C73B73 - 0x1000) = XUIColorPref.network; /* Solid netowrk color */

			memset((PVOID)(0x92B1A236 - 0x1000), 0, 0x13); /* Enable Aura default value "61 75 72 61 5F 62 61 63 6B 67 72 6F 75 6E 64 2E 6A 70 67" */
			*(DWORD*)(0x92B50D97 - 0x1000) = XUIColorPref.background; /* Left */
			*(DWORD*)(0x92B50DB3 - 0x1000) = XUIColorPref.background; /* Bottom */
			*(DWORD*)(0x92B50E0B - 0x1000) = XUIColorPref.background; /* Top half */
			*(DWORD*)(0x92B50E03 - 0x1000) = XUIColorPref.background; /* Lower half */
			*(DWORD*)(0x92B50DA7 - 0x1000) = XUIColorPref.background; /* Center gay ass blade backgrnd */
			*(DWORD*)(0x92B50DAF - 0x1000) = XUIColorPref.background; /* center stupid cock shit */

			if (!editedHUDSkin && Teapot::AuthFlag == AUTHFLAG_ACTIVE) {
				HANDLE hHudSkin;
				Native::XexLoadImage(getString(STR_PATH_HUDSKIN).cstr, 8, 0, &hHudSkin);

				PVOID pSectionData; DWORD dwSectionSize;
				if (Native::XGetModuleSection(hHudSkin, getString(STR_SKIN).cstr, &pSectionData, &dwSectionSize)) {
					*(PDWORD)((DWORD)pSectionData + 0x72DC) = XUIColorPref.xuibutton;
					*(PDWORD)((DWORD)pSectionData + 0x730C) = XUIColorPref.xuibutton_press;
					XuiFreeVisuals(NULL);
					wchar_t tmp[50];
					if (NT_SUCCESS(XuiLoadVisualFromBinary(LinkWChar(tmp, L"section://%X,skin#skin.xur", hHudSkin), 0))) {
						Native::XuiVisualSetBasePath(L"skin://", 0);
					}
					*(WORD*)((INT)(hHudSkin)+0x40) = 1;
					Native::XexUnloadImage(hHudSkin);
					editedHUDSkin = TRUE;
				}
			}
		}
	}

	if (SupportedDashVersion && !dashboard::cUI.bWebPanelOveride) {
		if (AppName == L"dash.social.lex") *(DWORD*)0x9AFC4876 = XUIColorPref.titles;
		if (AppName == L"Dash.OnlineCommon.lex") {
			*(DWORD*)(0x9BB3896A - 0x1000) = XUIColorPref.titles;
			*(DWORD*)(0x9BB38996 - 0x1000) = XUIColorPref.titles;
		}

		if (AppName == L"dash.search.lex") {
			*(DWORD*)(0x9B06B948 - 0x1000) = XUIColorPref.bingsearch;
		}

		if (Teapot::Devkit) return;
		if (AppName == L"Title.NewLiveSignup.xex") *(DWORD*)0x92291ED6 = XUIColorPref.titles;
		if (AppName == L"guide.beacons.xex" || AppName == L"Guide.Beacons.xex") {
			*(DWORD*)(0x9016269D - 0x1000) = XUIColorPref.titles; /* Primary */
			*(DWORD*)(0x901626B5 - 0x1000) = XUIColorPref.titles; /* Secondary */
		}
	}
}


void hud::XamBuildResourceLocatorHook(HANDLE hModule, PWCHAR wModuleName, PWCHAR const cdModule, PWCHAR pBuffer, ULONG ddSize) {
	wchar_t tmp[50];
	PCWSTR edits[4] = { L"notify.xur", L"Blade_grey.png", L"Blade_dark.png", L"loadingRing.png" };
	XamBuildResourceLocatorTour.callOriginal(hModule, wModuleName, cdModule, pBuffer, ddSize);
	if (!Teapot::Devkit) {
		for (int i = 0; i < (int)ARRAYSIZE(edits) - Teapot::Devkit ? 3 : 0; i++) {
			if (!lstrcmpW(cdModule, edits[i])) {
				if (!dashboard::cUI.bCustomNotify && WCompare(cdModule, L"notify.xur")==0) continue;
				if (!lstrcmpW(edits[i], L"notify.xur"))  Native::XuiLoadVisualFromBinary(LinkWChar(tmp, L"section://%X,Teapot#Teapot.xur", AppMgr.Self.hand), NULL);
				swprintf(pBuffer, L"section://%X,Teapot#%ws", AppMgr.Self.hand, cdModule);
			}
		}
	}else {
		if (!lstrcmpW(cdModule, L"notify.xur")) {
			Native::XuiLoadVisualFromBinary(LinkWChar(tmp, L"section://%X,Teapot#Teapot.xur", AppMgr.Self.hand), NULL);
			swprintf(pBuffer, L"section://%X,Teapot#%ws", AppMgr.Self.hand, cdModule);
		}
	}
}

HRESULT hud::XuiElementBeginRenderHook(HXUIOBJ hObj, XUIMessageRender* pRenderData, XUIRenderStruct* pRenderStruct) {
	using namespace dashboard;

	HXUIOBJ hParentObj; LPCWSTR ObjID, ParentText;
	XuiElementGetId(hObj, &ObjID);
	XuiElementGetParent(hObj, &hParentObj);
	XuiElementGetId(hParentObj, &ParentText);

	DWORD Color = D3DCOLOR_ARGB(255, cUI.iR, cUI.iG, cUI.iB);

	if (cUI.bCustomNotify && lstrcmpW(ObjID, L"logoback") == 0 || lstrcmpW(ObjID, L"SquareOutlineTop") == 0 || lstrcmpW(ObjID, L"SquareOutlineLeft") == 0 || lstrcmpW(ObjID, L"SquareOutlineRight") == 0 || lstrcmpW(ObjID, L"SquareOutlineBottom") == 0 || lstrcmpW(ObjID, L"Square") == 0 || lstrcmpW(ObjID, L"bgEdge1") == 0 || lstrcmpW(ObjID, L"bgEdge2") == 0 || lstrcmpW(ObjID, L"bgBase") == 0 || lstrcmpW(ObjID, L"bgOutlineTop") == 0 || lstrcmpW(ObjID, L"bgOutlineBottom") == 0 || lstrcmpW(ObjID, L"bgOutlineEdge") == 0)
		SetFill(hObj, cUI.bNotifyFade ? RGBColor2 : Color);

	if (lstrcmpW(ObjID, L"ePreview") == 0)
		SetFill(hObj, Color);

	if (lstrcmpW(ObjID, L"background") == 0 || lstrcmpW(ObjID, L"GreenHighlight") == 0 || lstrcmpW(ObjID, L"GreenHighlight1") == 0)
		SetFill(hObj, cUI.bHudFade ? RGBColor : Color);


	HRESULT res = XuiElementBeginRenderStub(hObj, pRenderData, pRenderStruct);
	if (NT_SUCCESS(res)) {
		if (lstrcmpW(ParentText, L"Tabscene") == 0) {
			XuiElementGetParent(hParentObj, &hGuideMain);
			XuiTabSceneGetCurrentTab(hParentObj, &hSettingsTab);
			XuiElementGetFirstChild(hSettingsTab, &hSettingsTab);
			XuiElementGetNext(hSettingsTab, &hSettingsTab);
			XuiElementGetId(hSettingsTab, &ParentText);
		}
		return res;
	}
	return res;
}

HRESULT hud::XuiSceneCreateHook(LPCWSTR szBasePath, LPCWSTR szScenePath, PVOID pvInitData, HXUIOBJ* phScene) {
	wchar_t guideLabel[21];
	wchar_t tmp[70];
	BOOL isGuideMain = (lstrcmpW(szScenePath, L"GuideMain.xur") == 0);
	PWCHAR TeapotXZP = LinkWChar(tmp, L"section://%X,Teapot#", AppMgr.Self.hand);
	HRESULT Ret = XuiSceneCreate(isGuideMain ? TeapotXZP : L"section://@0,hud#", szScenePath, pvInitData, phScene);
	if (isGuideMain && NT_SUCCESS(Ret)) {
		propSetString(*phScene, L"Header", charToWChar(sData.guideLabel, guideLabel));
		doDrawInfo(phScene);
	}
	XuiElementSetBasePath(*phScene, L"section://@0,hud#");
	return Ret;
}

bool EmptyHeaders = false;
DWORD hud::NetDll_XHttpConnectHook(XNCALLER_TYPE caller, SOCKET sock, PCHAR params, WORD port, DWORD dwFlags) {
	EmptyHeaders = false;
	if (strstr(params + 0x200, XML_DASHHOME.cstr) != NULL || strstr(params + 0x200, XML_ANNOUNCEMENTS.cstr) != NULL) {
		//if (strstr(params + 0x200, XML_DASHHOME.cstr) != NULL) {
		//	strcpy(params, getString(STR_HOST_CDNSERVER).cstr);
		//	sprintf(params + 0x200, getString(STR_URLPATH_XMLDASHHOME).cstr, XML_MANIFESTS_DIR.cstr, XUIColorPref.titles, 0 /*lang ID*/);
		//	//"/%s/dashhome.php?cs=%08X&lang=%i\0"
		//	//dashhome.php
		//}

		//if (strstr(params + 0x200, XML_ANNOUNCEMENTS.cstr) != NULL) {
		//	strcpy(params, getString(STR_HOST_CDNSERVER_CFB).cstr); //CFBypass
		//	sprintf(params + 0x200, getString(STR_URLPATH_XMLUPDATES).cstr, XML_MANIFESTS_DIR.cstr, rand() % 0xFFFFF); //cache busted
		//	debug_sys("[INFO] NetDll_XHttpConnect Intercepted: %s%s", params, params + 0x200);
		//	//update.php
		//}

		//debug_sys("[INFO] NetDll_XHttpConnect Intercepted: %s%s", params, params + 0x200);
		EmptyHeaders = true;
		dwFlags = 0;
		port = 80;
	}


	return NetDll_XHttpConnectTour.callOriginal(caller, sock, params, port, dwFlags);
}

BOOL NetDll_XHttpSendRequestHook(XNCALLER_TYPE Type, HANDLE Handle, char* Headers, DWORD HeadersLength, void* Optional, DWORD OptionalLength, DWORD TotalLength, PDWORD Context) {
	if (EmptyHeaders) Headers = 0;
	return ((BOOL(*)(...))Native::resolveFunction(AppMgr.Xam.hand, 0xD1))(Type, Handle, Headers, HeadersLength, Optional, OptionalLength, TotalLength, Context);
}

void hud::installHooks() {
	//XML_DASHHOME = getString(STR_XML_DASHHOME);
	//XML_ANNOUNCEMENTS = getString(STR_XML_ANNOUNCEMENTS);
	XML_MANIFESTS_DIR = getString(STR_URLDIR_MANIFESTS);
	if (ini::customUI) XamBuildResourceLocatorTour.setupDetour(Native::resolveFunction(getString(STR_XAM).cstr, 0x31B), XamBuildResourceLocatorHook);
	if (ini::customUI) XuiElementBeginRenderStub = (tXuiElementBeginRender)HookFunctionStub(Native::resolveFunction(AppMgr.Xam.hand, 0x3A8), XuiElementBeginRenderHook);
}


/*
//SendNotifyPressStub = (tSendNotifyPress)HookFunctionStub(0x817CA3A0, SendNotifyPressHook);
INT hud::SendNotifyPressHook(HXUIOBJ r3, WORD r4, WORD r5) {
	XUIElementPropVal propVal;
	GetProperty(r3, L"Text", &propVal);
	printf("Button press -> %ws\n", propVal.szVal);
	if (lstrcmpW(propVal.szVal, L"Teapot Settings") == 0) {
		XuiLoadVisualFromBinary(L"file://NotTeapot:\\TeapotResource\\TeapotSkin.xur", NULL);
		XuiVisualSetBasePath(L"skin://", 0);
		//scnTabPlugins::Register();
		//scnTabUI::Register();
		XuiSceneCreate(nullptr, L"file://NotTeapot:\\TeapotResource\\Settings.xur", NULL, &SettingsScene);
		XuiSceneNavigateForward(hGuideMain, FALSE, SettingsScene, 0xFF);
		return 0;
	}


	return SendNotifyPressStub(r3, r4, r5);
}*/