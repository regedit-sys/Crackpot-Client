#pragma once

#ifdef CFG_TWEAKXUI
class ScnTab5 : CXuiSceneImpl {
public:
	static ScnTab5& getInstance() {
		static ScnTab5 singleton;
		return singleton;
	}

	XUI_IMPLEMENT_CLASS(ScnTab5, L"scnTab4", XUI_CLASS_SCENE) 
		XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_END_MSG_MAP()

		HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
};


class HUDScenePreferences : CXuiSceneImpl {
public:
	CXuiControl btnSave;
	CXuiCheckbox cb_nokvmode;
	CXuiCheckbox cb_notify_xosc, cb_notify_hvc, cb_notify_welcome;
	CXuiCheckbox cb_offhost_mw2, cb_offhost_mw3, cb_offhost_bo, cb_offhost_bo2, cb_offhost_bo3, cb_offhost_aw, cb_offhost_ghosts, cb_offhost_waw, cb_offhost_cod4;

	XUI_IMPLEMENT_CLASS(HUDScenePreferences, L"HUDScenePreferencesClass", L"HUDScene")
		XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_NOTIFY_PRESS(OnNotifyPress)
	XUI_END_MSG_MAP()

	HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	//HRESULT OnEnterTab(BOOL& bHandled);
	HRESULT OnNotifyPress(HXUIOBJ hObjPressed, BOOL& bHandled);
};
#endif