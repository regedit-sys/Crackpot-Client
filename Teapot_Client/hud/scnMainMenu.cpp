#include "stdafx.h"
#include "common.h"
#include "scnMainMenu.h"
#include "preferences/pluginManager.h"
#include "preferences/designer.h"
#include "preferences/options.h"
#include "ScnTab5.h"

#ifdef CFG_TWEAKXUI
HRESULT ScnMainMenu::OnInit(XUIMessageInit *pInitData, BOOL& bHandled) {
	InitializeChildren();

	wchar_t kvinfoStr[20];
	swprintf(kvinfoStr, L"%id %ih %im", cData.kvinfo.days, cData.kvinfo.hours, cData.kvinfo.minutes);
	if (ini::NoKVMode) swprintf_s(kvinfoStr, 20, cData.kvinfo.status > 0 ? L"Unbanned" : L"Banned");
	info_kvstatus_guest.SetText(kvinfoStr);

	if (cData.kvinfo.status != -1) {
		if (cData.kvinfo.status > 0) {
			info_kvstatus_guest.SetText(kvinfoStr);
			info_kvstatus_banned_guest.SetText(L'');
		}
		if (cData.kvinfo.status < 1) {
			info_kvstatus_banned_guest.SetText(kvinfoStr);
		}
	}
	return ERROR_SUCCESS;
}

HRESULT ScnMainMenu::OnNotifyPress(HXUIOBJ hObjPressed, BOOL& bHandled) {
	if (hObjPressed == btn_register_continue) Native::CreateThreadExQ(REGISTER_EMAIL);
	else if (hObjPressed == btn_expired_redeem) Native::CreateThreadExQ(TOKEN_CHECK);
	else if (hObjPressed == btnRedeemToken) Native::CreateThreadExQ(TOKEN_CHECK);
	else if (hObjPressed == btnPreferences) {
		wchar_t tmp[50];
		HXUIOBJ hParent; //use file://
		HXUIOBJ hScnPreferences;
		XuiElementGetParent(m_hObj, &hParent);
		for (int i = 0; i < 2; i++) XuiElementGetParent(hParent, &hParent);
		Native::XuiLoadVisualFromBinary(LinkWChar(tmp, L"section://%X,Teapot#TeapotSkin.xur", AppMgr.Self.hand), NULL);
		Native::XuiSceneCreate(LinkWChar(tmp, L"section://%X,Teapot#", AppMgr.Self.hand), L"TeapotScene.xur", NULL, &hScnPreferences);
		Native::XuiSceneNavigateForward(hParent, false, hScnPreferences, XUSER_INDEX_FOCUS);
	}else if(hObjPressed == btnReboot) Native::HalReturnToFirmware(HalRebootRoutine);
	else if (hObjPressed == btnConnectionError) Native::HalReturnToFirmware(HalFatalErrorRebootRoutine);
	return ERROR_SUCCESS;
}

HRESULT ScnMainMenu::InitializeChildren() {
	GetChildById(L"btn_expired_redeem", &btn_expired_redeem);
	GetChildById(L"btn_register_continue", &btn_register_continue);
	GetChildById(L"btnRedeem", &btnRedeemToken);
	GetChildById(L"btnReboot", &btnReboot);
	GetChildById(L"btnPreferences", &btnPreferences);
	GetChildById(L"btn_notConnected_reboot", &btnConnectionError);
	GetChildById(L"info_kvstatus_guest", &info_kvstatus_guest);
	GetChildById(L"info_kvstatus_banned_guest", &info_kvstatus_banned_guest);
	return ERROR_SUCCESS;
}
#endif