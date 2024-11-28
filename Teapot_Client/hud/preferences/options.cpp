#include "stdafx.h"
#include "common.h"
#include "options.h"
#include "../xui.h"

using namespace hud;
AsyncEvent SceneNetworkEvent;
options::scnTabOptions *Tab;
int defaultFanSpeedValue = 55;
int fanSpeedValue = 55;

void SetTextColor(HXUIOBJ Obj, DWORD Color) {
	XUIElementPropVal Element;
	XUIElementPropVal_SetUint(&Element, Color);
	SetProperty(Obj, L"TextColor", &Element);
}

void ToggleElements(options::scnTabOptions *Tab, bool state) {
	Tab->cb_notify_welcome.SetEnable(state);
	Tab->cb_notify_hvc.SetEnable(state);
	Tab->cb_notify_xosc.SetEnable(state);
	Tab->cb_offhost_mw2.SetEnable(state);
	Tab->cb_offhost_mw3.SetEnable(state);
	Tab->cb_offhost_bo.SetEnable(state);
	Tab->cb_offhost_bo2.SetEnable(state);
	Tab->cb_offhost_bo3.SetEnable(state);
	Tab->cb_offhost_aw.SetEnable(state);
	Tab->cb_offhost_ghosts.SetEnable(state);
	Tab->cb_offhost_waw.SetEnable(state);
	Tab->cb_offhost_cod4.SetEnable(state);
	Tab->cb_onHost_mw2.SetEnable(state);
	Tab->cb_gtav.SetEnable(state);
	Tab->cb_nokvmode.SetEnable(state);
	Tab->sldFanSpeed.SetEnable(state);
}

void AsyncPopulate() {
	wchar_t kvinfoStr[32];
	wchar_t wcharBuffer[80];

	if (SUCCEEDED(NET::PREFERENCES_GET(cData.preferences))) {
		if (Tab->IsValidObject()) {
			SetElementVisability(Tab->imgLoading, false);
			ToggleElements(Tab, true);

			swprintf(kvinfoStr, L"%id %ih %im", cData.preferences.kvinfo.days, cData.preferences.kvinfo.hours, cData.preferences.kvinfo.minutes);
			if (cData.preferences.kvinfo.status != -1) {
				if (ini::NoKVMode) swprintf_s(kvinfoStr, 20, cData.preferences.kvinfo.status > 0 ? L"Unbanned" : L"Banned");
				AddText(&Tab->m_hObj, cData.preferences.kvinfo.status > 0 ? RGBA_GREEN : RGBA_RED, 10.0, 511.00, 328.00, kvinfoStr);
			}

			Tab->cb_nokvmode.SetCheck(ini::NoKVMode);
			Tab->cb_notify_welcome.SetCheck(cData.preferences.xnotify.bwelcome);
			Tab->cb_notify_hvc.SetCheck(cData.preferences.xnotify.bxam);
			Tab->cb_notify_xosc.SetCheck(cData.preferences.xnotify.bxosc);
			Tab->cb_offhost_mw2.SetCheck(cData.preferences.cb_offhost_mw2);
			Tab->cb_offhost_mw3.SetCheck(cData.preferences.cb_offhost_mw3);
			Tab->cb_offhost_bo.SetCheck(cData.preferences.cb_offhost_bo);
			Tab->cb_offhost_bo2.SetCheck(cData.preferences.cb_offhost_bo2);
			Tab->cb_offhost_bo3.SetCheck(cData.preferences.cb_offhost_bo3);
			Tab->cb_offhost_aw.SetCheck(cData.preferences.cb_offhost_aw);
			Tab->cb_offhost_ghosts.SetCheck(cData.preferences.cb_offhost_ghosts);
			Tab->cb_offhost_waw.SetCheck(cData.preferences.cb_offhost_waw);
			Tab->cb_offhost_cod4.SetCheck(cData.preferences.cb_offhost_cod4);
			Tab->cb_onHost_mw2.SetCheck(ini::onHost_MW2);
			Tab->cb_gtav.SetCheck(ini::CE_GTAV);

			Tab->txt_email.SetText(charToWChar(cData.email, wcharBuffer));
			Tab->txt_username.SetText(charToWChar(cData.name, wcharBuffer));
			Tab->txt_kvstats.SetText(L"");
			Tab->btnSave.SetText(L"Save Settings");

			Tab->sldFanSpeed.SetValue(fanSpeedValue);
			if (fanSpeedValue != defaultFanSpeedValue) {
				Tab->txFanSpeedAuto.SetText(L"");
			}
		}else debug("[INFO] [HUD] The Options scene object is not active");
	}else FNotify(L"Could not load Preferences - Connection Failed!").error();
	SceneNetworkEvent.Reset();
}

HRESULT options::scnTabOptions::OnInit(XUIMessageInit* pInitData, BOOL& bHandled) {
	wchar_t cpuStr[45];
	byte CPUKey[0x12];
	Tab = this;

	Native::HvGetCPUKey(CPUKey);
	Mem::Xor(CPUKey, Native::DecVal(0x708CEB42) /*0xC5*/, Native::DecVal(0x2D8DEB42) /*0x10*/);

	GetChildById(L"btnSaveToTeapot", &btnSave);
	GetChildById(L"chkNoKV", &cb_nokvmode);
	GetChildById(L"chkXeKeys", &cb_notify_hvc);
	GetChildById(L"chkWelcome", &cb_notify_welcome);
	GetChildById(L"chkMW2", &cb_offhost_mw2);
	GetChildById(L"chkMW3", &cb_offhost_mw3);
	GetChildById(L"chkBO1", &cb_offhost_bo);
	GetChildById(L"chkBO2", &cb_offhost_bo2);
	GetChildById(L"chkBO3", &cb_offhost_bo3);
	GetChildById(L"chkAW", &cb_offhost_aw);
	GetChildById(L"chkGhosts", &cb_offhost_ghosts);
	GetChildById(L"chkWAW", &cb_offhost_waw);
	GetChildById(L"chkCOD4", &cb_offhost_cod4);
	GetChildById(L"chkOHMW2", &cb_onHost_mw2);
	GetChildById(L"chkGTAV", &cb_gtav);
	GetChildById(L"txFanSpeedAuto", &txFanSpeedAuto);
	GetChildById(L"sldFanspeed", &sldFanSpeed);

	GetChildById(L"txt_email", &txt_email);
	GetChildById(L"txt_username", &txt_username);
	GetChildById(L"txt_cpukey", &txt_cpukey);
	GetChildById(L"txt_kvstats", &txt_kvstats);
	GetChildById(L"txt_intl_ip", &txt_ipaddr);
	GetChildById(L"imgLoading", &imgLoading);
	XuiControlSetEnable(btnSave, false);
	ToggleElements(this, false);

	btnSave.SetText(L"Connecting...");
	cb_onHost_mw2.SetText(L"Modern Warfare 2 (On Host)");
	txt_cpukey.SetText(charToWChar(BytesToHexStr(CPUKey, 0x10).cstr, cpuStr));

	txt_email.SetText(L"Loading...");
	txt_username.SetText(L"Loading...");
	txt_kvstats.SetText(L"Loading...");
	txt_email.SetText(L"Loading...");
	txt_kvstats.SetText(L"Loading...");

	if (Teapot::IPAddrIntrnl.ina.S_un.S_addr) {
		txt_ipaddr.SetText(ParseIPAddrW(Teapot::IPAddrIntrnl.ina, false).cstr);
	}

	if (!SceneNetworkEvent.Busy()) {
		SceneNetworkEvent.Create(0x1BB);
		Native::CreateThreadExQ(AsyncPopulate);
	} 

	bHandled = true;
	return ERROR_SUCCESS;
}

HRESULT options::scnTabOptions::OnNotifyValueChanged(HXUIOBJ hObjSource, XUINotifyValueChanged* pNotifyValueChangedData, BOOL& bHandled) {
	if (hObjSource == sldFanSpeed) {
		SetFanSpeed(pNotifyValueChangedData->nValue);
		fanSpeedValue = pNotifyValueChangedData->nValue;
		if(fanSpeedValue != defaultFanSpeedValue) txFanSpeedAuto.SetText(L"");
	}

	bHandled = true;
	return ERROR_SUCCESS;
}

HRESULT options::scnTabOptions::OnNotifyPress(HXUIOBJ hObjPressed, BOOL& bHandled) {
	XuiControlSetEnable(btnSave, true);
	if (hObjPressed == btnSave) {
		NET_PREFERENCES preferences;
		Mem::Null(&preferences.kvinfo, sizeof NET_RESP_FETCHCDATA_KVINFO_s);
		preferences.xnotify.bwelcome = cb_notify_welcome.IsChecked();
		preferences.xnotify.bxam = cb_notify_hvc.IsChecked();
		preferences.xnotify.bxosc = cb_notify_xosc.IsChecked();
		preferences.cb_offhost_mw2 = cb_offhost_mw2.IsChecked();
		preferences.cb_offhost_mw3 = cb_offhost_mw3.IsChecked();
		preferences.cb_offhost_bo = cb_offhost_bo.IsChecked();
		preferences.cb_offhost_bo2 = cb_offhost_bo2.IsChecked();
		preferences.cb_offhost_bo3 = cb_offhost_bo3.IsChecked();
		preferences.cb_offhost_aw = cb_offhost_aw.IsChecked();
		preferences.cb_offhost_ghosts = cb_offhost_ghosts.IsChecked();
		preferences.cb_offhost_waw = cb_offhost_waw.IsChecked();
		preferences.cb_offhost_cod4 = cb_offhost_cod4.IsChecked();

		bool CachedNoKVMode = ini::NoKVMode;
		if ((bool)cb_nokvmode.IsChecked() != CachedNoKVMode) {
			ini::NoKVMode = cb_nokvmode.IsChecked();
			if (Config::ModifyINIBoolValue(getString(STR_INI_SETTINGS).cstr, "NoKVMode", cb_nokvmode.IsChecked())) {
				FNotify(L"Crackpot - Rebooting to finalize KV mode changes.").show(true);
			}else FNotify(L"Failed to write 'NoKVMode' to Crackpot.ini").error();
		}

		if (Config::ModifyINIBoolValue(getString(STR_INI_SETTINGS).cstr, "Mw2Onhost", cb_onHost_mw2.IsChecked())) {
			ini::onHost_MW2 =  cb_onHost_mw2.IsChecked();
		}else FNotify(L"Failed to write 'Mw2Onhost' to Crackpot.ini").error();

		if (Config::ModifyINIBoolValue(getString(STR_INI_SETTINGS).cstr, "GTAV", cb_gtav.IsChecked())) {
			ini::CE_GTAV = cb_gtav.IsChecked();
		}else FNotify(L"Failed to write 'GTAV' to Crackpot.ini").error();

		if (!FAILED(NET::PREFERENCES_SET(preferences))) {
			XuiControlSetEnable(btnSave, false);
			FNotify(L"Preferences Saved!").show();
		}else FNotify(L"Error contacting server, your preferences have not been saved!").error();
	}

	bHandled = true;
	return ERROR_SUCCESS;
}