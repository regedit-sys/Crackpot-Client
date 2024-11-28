#include "stdafx.h"
#include "common.h"

#include "scnTab5.h"
#include "xui.h"
#include "util/SimpleIni.h"

#ifdef CFG_TWEAKXUI
HXUIOBJ hScene;

HRESULT ScnTab5::OnInit(XUIMessageInit *pInitData, BOOL& bHandled) {
	HXUIOBJ GuideMainMenuScene = NULL;
	WCHAR tmp[58], tmp1[58];
	WCHAR *scene, *base = L"section://%08X,Teapot#%ls";

	if (Teapot::AuthFlag == AUTHFLAG_ACTIVE) scene = L"GuideMenu.xur";
	else if (Teapot::AuthFlag == AUTHFLAG_NOEXIST) scene = L"GuideMenu_unregistered.xur";
	else if (Teapot::AuthFlag == AUTHFLAG_EXPIRED || Teapot::AuthFlag == AUTHFLAG_REBOOTPENDING) scene = L"GuideMenu_expired.xur";
	else if (Teapot::AuthFlag == AUTHFLAG_NOTCONNECTED) scene = L"GuideMenu_notconnected.xur";
	swprintf(tmp, base, AppMgr.Self.hand, scene);
	swprintf(tmp1, base, AppMgr.Self.hand, L"");

	HRESULT ret = Native::XuiSceneCreate(tmp1, tmp, NULL, &GuideMainMenuScene);
	if (SUCCEEDED(ret)) XuiElementSetBasePath(GuideMainMenuScene, tmp1);
	Native::XuiElementAddChild(*this, GuideMainMenuScene);
	XuiSceneNavigateFirst(*this, GuideMainMenuScene, XUSER_INDEX_ANY);
	hScene = GuideMainMenuScene;
	return ERROR_SUCCESS;
}
#endif