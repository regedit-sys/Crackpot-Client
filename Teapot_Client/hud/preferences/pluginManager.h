#pragma once

namespace hud {
	namespace pluginManager {
		struct PluginList_s {
			wchar_t Plugin1[50];
			wchar_t Plugin2[50];
			wchar_t Plugin3[50];
			wchar_t Plugin4[50];
			wchar_t Plugin5[50];
		}; extern PluginList_s PluginList;

		struct QuickLaunchPluginList_s {
			wchar_t Plugin1[50];
			wchar_t Plugin2[50];
			wchar_t Plugin3[50];
		}; extern QuickLaunchPluginList_s qPluginList;

		void ParseQuicklaunchINI();
		void WrteQuickLaunchINI();
		void ParseLaunchINI();
		void WriteLaunchINI(int plugin);
		void SetPlugin(int pluginid);
		void InjectNewPlugin();
		void LaunchPlugin(int passed);

		class scnTabPlugins : CXuiSceneImpl {
		public:

			CXuiControl btnPlugin1, btnPlugin2, btnPlugin3, btnPlugin4, btnPlugin5, btnInjectPlugin, btnQLaunch1, btnQLaunch2, btnQLaunch3, btnResetXBDM;
			XUI_IMPLEMENT_CLASS(scnTabPlugins, L"scnTabPlugins", XUI_CLASS_SCENE)
				XUI_BEGIN_MSG_MAP()
				XUI_ON_XM_INIT(OnInit)
				XUI_ON_XM_TIMER(OnTimer)
				XUI_ON_XM_NOTIFY_PRESS(OnNotifyPress)
				XUI_END_MSG_MAP()

			HRESULT OnInit(XUIMessageInit* pInitData, BOOL& bHandled);
			HRESULT OnNotifyPress(HXUIOBJ hObjPressed, BOOL& bHandled);
			HRESULT OnTimer(XUIMessageTimer* pTimer, BOOL& bHandled);
			void fillPluginSlots();
		};
	}
}