#include "stdafx.h"
#include "util/SimpleIni.h"
#include "util/io.h"
#include "dash.h"

dashboard::UISettings_s dashboard::cUI;

bool dashboard::RGBThreadStarted = false;
DWORD dashboard::RGBColor = 0xFFFF0000;
DWORD dashboard::RGBColor2 = 0xFFFF0000;

#pragma optimize("", off)
void dashboard::DoRGB() {
	int Speed = 1;
	int R, G, B;
	R = ((RGBColor >> 16) & 0xFF);
	G = ((RGBColor >> 8) & 0xFF);
	B = ((RGBColor) & 0xFF);
	if (R > 0 && B <= 0) {
		R -= Speed;
		G += Speed;
	}
	if (G > 0 && R <= 0) {
		G -= Speed;
		B += Speed;
	}
	if (B > 0 && G <= 0) {
		R += Speed;
		B -= Speed;
	}
	RGBColor = D3DCOLOR_ARGB(0xFF, R, G, B);
	Native::Sleep(Native::DecVal(0x328DEB42) /*0x0F*/);
}

void dashboard::DoRGB2() {
	int Speed = 1;
	int R, G, B;
	R = ((RGBColor2 >> 16) & 0xFF);
	G = ((RGBColor2 >> 8) & 0xFF);
	B = ((RGBColor2) & 0xFF);
	if (R > 0 && B <= 0) {
		R -= Speed;
		G += Speed;
	}
	if (G > 0 && R <= 0) {
		G -= Speed;
		B += Speed;
	}
	if (B > 0 && G <= 0) {
		R += Speed;
		B -= Speed;
	}
	RGBColor2 = D3DCOLOR_ARGB(0xFF, R, G, B);
}
#pragma optimize("", on)

void setDefaultRGB() {
	dashboard::cUI.iR = 5;
	dashboard::cUI.iG = 158;
	dashboard::cUI.iB = 28;
}

void dashboard::ReadUIConfig() {
	CSimpleIni reader(true, false, true);
	if (reader.LoadFile(DRIVE_TEAPOT "CrackpotCfg\\ui.ini") >= 0 && !reader.IsEmpty()) {
		cUI.bWebPanelOveride = reader.GetBoolValue("TeapotUI", "WebPanelOveride");
		cUI.bCustomNotify = reader.GetBoolValue("TeapotUI", "CustomNotify");
		cUI.bBackgroundFade = reader.GetBoolValue("TeapotUI", "BackgroundFade");
		cUI.bTileFade = reader.GetBoolValue("TeapotUI", "TileFade");
		cUI.bHudFade = reader.GetBoolValue("TeapotUI", "HudFade");
		cUI.bNotifyFade = reader.GetBoolValue("TeapotUI", "NotifyFade");
		cUI.bBackGradient = reader.GetBoolValue("TeapotUI", "BackgroundGradient");
		cUI.bTileGradient = reader.GetBoolValue("TeapotUI", "TileGradient");
		cUI.iR = reader.GetDoubleValue("TeapotUI", "R");
		cUI.iG = reader.GetDoubleValue("TeapotUI", "G");
		cUI.iB = reader.GetDoubleValue("TeapotUI", "B");

		if (!cUI.bWebPanelOveride) {
			setDefaultRGB();
		}
	}else debug("dashboard::ReadUIConfig ERROR!");
}

void dashboard::UpdateUIConfig() {
	CSimpleIni WriteINI(true, false, true);
	WriteINI.SetBoolValue("TeapotUI", "WebPanelOveride", cUI.bWebPanelOveride);
	WriteINI.SetBoolValue("TeapotUI", "CustomNotify", cUI.bCustomNotify);
	WriteINI.SetBoolValue("TeapotUI", "BackgroundFade", cUI.bBackgroundFade);
	WriteINI.SetBoolValue("TeapotUI", "TileFade", cUI.bTileFade);
	WriteINI.SetBoolValue("TeapotUI", "HudFade", cUI.bHudFade);
	WriteINI.SetBoolValue("TeapotUI", "NotifyFade", cUI.bNotifyFade);
	WriteINI.SetBoolValue("TeapotUI", "BackgroundGradient", cUI.bBackGradient);
	WriteINI.SetBoolValue("TeapotUI", "TileGradient", cUI.bTileGradient);
	WriteINI.SetDoubleValue("TeapotUI", "R", cUI.iR);
	WriteINI.SetDoubleValue("TeapotUI", "G", cUI.iG);
	WriteINI.SetDoubleValue("TeapotUI", "B", cUI.iB);
	WriteINI.SaveFile(DRIVE_TEAPOT "CrackpotCfg\\ui.ini");

	if (!cUI.bWebPanelOveride) {
		setDefaultRGB();
	}
}

void dashboard::WriteDefaultUIConfig() {
	CSimpleIni WriteINI(true, false, true);
	WriteINI.SetBoolValue("TeapotUI", "WebPanelOveride", true);
	WriteINI.SetBoolValue("TeapotUI", "CustomNotify", true);
	WriteINI.SetBoolValue("TeapotUI", "BackgroundFade", false);
	WriteINI.SetBoolValue("TeapotUI", "TileFade", false);
	WriteINI.SetBoolValue("TeapotUI", "HudFade", false);
	WriteINI.SetBoolValue("TeapotUI", "NotifyFade", false);
	WriteINI.SetBoolValue("TeapotUI", "BackgroundGradient", true);
	WriteINI.SetBoolValue("TeapotUI", "TileGradient", true);
	WriteINI.SetDoubleValue("TeapotUI", "R", 5);
	WriteINI.SetDoubleValue("TeapotUI", "G", 158);
	WriteINI.SetDoubleValue("TeapotUI", "B", 28);
	WriteINI.SaveFile(DRIVE_TEAPOT "CrackpotCfg\\ui.ini");
}

void dashboard::DoINI() {
	if (!DirectoryExists("Crackpot:\\CrackpotCfg\\"))
		CreateDirectory("Crackpot:\\CrackpotCfg\\", 0);

	if (!fileExists(DRIVE_TEAPOT "CrackpotCfg\\ui.ini"))
		WriteDefaultUIConfig();

	ReadUIConfig();
	debug("[HUD] Config Applied!");
}