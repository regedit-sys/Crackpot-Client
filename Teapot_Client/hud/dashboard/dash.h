#pragma once
#include "common.h"

namespace dashboard {
	struct UISettings_s {
		bool bWebPanelOveride;
		bool bCustomNotify;
		bool bBackgroundFade;
		bool bTileFade;
		bool bHudFade;
		bool bNotifyFade;
		bool bBackGradient;
		bool bTileGradient;
		int iR;
		int iG;
		int iB;

	}; extern UISettings_s cUI;

	extern bool RGBThreadStarted;
	extern DWORD RGBColor;
	extern DWORD RGBColor2;

	void DoRGB();
	void DoRGB2();

	void ReadUIConfig();
	void UpdateUIConfig();
	void WriteDefaultUIConfig();
	void DoINI();
}