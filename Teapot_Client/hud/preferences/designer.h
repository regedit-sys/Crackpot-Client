#pragma once

namespace hud {
	namespace designer {
		class scnTabUI : CXuiSceneImpl {
		public:

			CXuiControl btnSave, btnLoadDefaults;
			CXuiCheckbox chkOverideWeb, chkCustomNotify, chkBackgroundFade, chkTileFade, chkHudFade, chkNotifyFade, chkBackGradient, chkTileGradient;
			CXuiElement ePreview;
			CXuiSlider sldRed, sldGreen, sldBlue;
			CXuiTextElement XuiText1;

			XUI_IMPLEMENT_CLASS(scnTabUI, L"scnTabUI", XUI_CLASS_SCENE)
				XUI_BEGIN_MSG_MAP()
				XUI_ON_XM_INIT(OnInit)
				XUI_ON_XM_TIMER(OnTimer)
				XUI_ON_XM_NOTIFY_PRESS(OnNotifyPress)
				XUI_ON_XM_NOTIFY_VALUE_CHANGED(OnNotifyValueChanged)
				XUI_END_MSG_MAP()

				HRESULT OnInit(XUIMessageInit* pInitData, BOOL& bHandled);
			HRESULT OnNotifyPress(HXUIOBJ hObjPressed, BOOL& bHandled);
			HRESULT OnTimer(XUIMessageTimer* pTimer, BOOL& bHandled);
			HRESULT OnNotifyValueChanged(HXUIOBJ hObjSource, XUINotifyValueChanged* pNotifyValueChangedData, BOOL& bHandled);
		};
	}
}