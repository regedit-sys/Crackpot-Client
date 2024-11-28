#pragma once

extern detour<void> XamBuildResourceLocatorTour;

struct XUIColorPreferences {
	DWORD background, xuibutton, xuibutton_press, goldbar_l, goldbar_r, network, bingsearch, titles, tpUserInfoLbl, tplUserInfo, tpPrefUserInfoLbl, tpPrefUserInfo, tiles_glow;
};

extern XUIColorPreferences XUIColorPref;

#ifdef CFG_TWEAKXUI
void hudMemoryPatches(PLDR_DATA_TABLE_ENTRY module);
VOID XamBuildResourceLocatorHook(HANDLE hModule, PWCHAR wModuleName, PWCHAR CONST cdModule, PWCHAR pBuffer, ULONG ddSize);
HRESULT XuiSceneCreateHook(LPCWSTR szBasePath, LPCWSTR szScenePath, PVOID pvInitData, HXUIOBJ* phScene);


namespace hud {
	extern HXUIOBJ hGuideMain;
	extern HXUIOBJ hSettingsTab;
	extern HXUIOBJ SettingsScene;

	extern detour<DWORD> NetDll_XHttpConnectTour;
	extern detour<void> XamBuildResourceLocatorTour;

	typedef INT(*tSendNotifyPress)(HXUIOBJ r3, WORD r4, WORD r5);
	typedef HRESULT(*tXuiElementBeginRender)(HXUIOBJ hObj, XUIMessageRender* pRenderData, XUIRenderStruct* pRenderStruct);

	extern tXuiElementBeginRender XuiElementBeginRenderStub;
	extern tSendNotifyPress SendNotifyPressStub;

	void doDrawInfo(HXUIOBJ* phScene);
	void patchModule(AppManager::C_AppInstance *pInstance);
	void XamBuildResourceLocatorHook(HANDLE hModule, PWCHAR wModuleName, PWCHAR const cdModule, PWCHAR pBuffer, ULONG ddSize);
	HRESULT XuiElementBeginRenderHook(HXUIOBJ hObj, XUIMessageRender* pRenderData, XUIRenderStruct* pRenderStruct);
	HRESULT XuiSceneCreateHook(LPCWSTR szBasePath, LPCWSTR szScenePath, PVOID pvInitData, HXUIOBJ* phScene);
	DWORD NetDll_XHttpConnectHook(XNCALLER_TYPE caller, SOCKET sock, PCHAR params, WORD port, DWORD dwFlags);

	extern void installHooks();
}
#else
#define InitializeXamBuildResourceLocatorHook()
#define InitializeXuiRegisterHooks()
#endif
