#pragma once

namespace hud {
	namespace options {
		class scnTabOptions : public CXuiSceneImpl {
		public:
			CXuiControl btnSave;
			CXuiCheckbox cb_notify_xosc, cb_notify_hvc, cb_notify_welcome, cb_nokvmode;
			CXuiCheckbox cb_offhost_mw2, cb_offhost_mw3, cb_offhost_bo, cb_offhost_bo2, cb_offhost_bo3, cb_offhost_aw, cb_offhost_ghosts, cb_offhost_waw, cb_offhost_cod4;
			CXuiCheckbox cb_onHost_mw2, cb_gtav;
			CXuiTextElement txt_email, txt_username, txt_kvstats, txt_cpukey, txt_ipaddr, txFanSpeedAuto;
			CXuiTabScene Tab1;
			CXuiSlider sldFanSpeed;
			CXuiImageElement imgLoading;

			XUI_IMPLEMENT_CLASS(scnTabOptions, L"scnTabOptions", XUI_CLASS_SCENE)
			XUI_BEGIN_MSG_MAP()
			XUI_ON_XM_INIT(OnInit)
			XUI_ON_XM_NOTIFY_PRESS(OnNotifyPress)
			XUI_ON_XM_NOTIFY_VALUE_CHANGED(OnNotifyValueChanged)
			XUI_END_MSG_MAP()
				
			HRESULT OnNotifyValueChanged(HXUIOBJ hObjSource, XUINotifyValueChanged* pNotifyValueChangedData, BOOL& bHandled);
			HRESULT OnInit(XUIMessageInit* pInitData, BOOL& bHandled);
			HRESULT OnNotifyPress(HXUIOBJ hObjPressed, BOOL& bHandled);
		};
	}
}