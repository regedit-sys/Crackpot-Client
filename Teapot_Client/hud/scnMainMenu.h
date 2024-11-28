#pragma once
#ifdef CFG_TWEAKXUI
class ScnMainMenu : CXuiSceneImpl {
public:
	static ScnMainMenu& getInstance() {
		static ScnMainMenu singleton;
		return singleton;
	}

	CXuiControl btn_expired_redeem;
	CXuiControl btn_register_continue;
	CXuiControl btnRedeemToken;
	CXuiControl btnReboot;
	CXuiControl btnPreferences;
	CXuiControl btnXeCheats;
	CXuiControl btnReflashKV;
	CXuiControl btnConnectionError;
	CXuiTextElement info_kvstatus_guest;
	CXuiTextElement info_kvstatus_banned_guest;

	XUI_IMPLEMENT_CLASS(ScnMainMenu, L"ScnMainMenu", XUI_CLASS_SCENE)
		XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_NOTIFY_PRESS(OnNotifyPress)
		XUI_END_MSG_MAP()

		HRESULT OnInit(XUIMessageInit *pInitData, BOOL& bHandled);
	HRESULT OnNotifyPress(HXUIOBJ hObjPressed, BOOL& bHandled);
	HRESULT InitializeChildren();
};
#endif