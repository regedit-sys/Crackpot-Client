#include "stdafx.h"
#include "designer.h"
#include "../xui.h"
#include "../dashboard/dash.h"

using namespace hud;
using namespace dashboard;

void ToggleVisability(designer::scnTabUI *Obj, bool state) {
	SetElementVisability(Obj->btnSave, state);
	SetElementVisability(Obj->btnLoadDefaults, state);
	SetElementVisability(Obj->chkCustomNotify, state);
	SetElementVisability(Obj->chkBackgroundFade, state);
	SetElementVisability(Obj->chkTileFade, state);
	SetElementVisability(Obj->chkHudFade, state);
	SetElementVisability(Obj->chkNotifyFade, state);
	SetElementVisability(Obj->chkBackGradient, state);
	SetElementVisability(Obj->chkTileGradient, state);
	SetElementVisability(Obj->ePreview, state);
	SetElementVisability(Obj->sldRed, state);
	SetElementVisability(Obj->sldGreen, state);
	SetElementVisability(Obj->sldBlue, state);
	SetElementVisability(Obj->XuiText1, state);
}

void PopulateCheckboxes(designer::scnTabUI *Obj) {
	Obj->chkOverideWeb.SetCheck(cUI.bWebPanelOveride);
	Obj->chkCustomNotify.SetCheck(cUI.bCustomNotify);
	Obj->chkBackgroundFade.SetCheck(cUI.bBackgroundFade);
	Obj->chkTileFade.SetCheck(cUI.bTileFade);
	Obj->chkHudFade.SetCheck(cUI.bHudFade);
	Obj->chkNotifyFade.SetCheck(cUI.bNotifyFade);
	Obj->chkBackGradient.SetCheck(cUI.bBackGradient);
	Obj->chkTileGradient.SetCheck(cUI.bTileGradient);
}

HRESULT designer::scnTabUI::OnInit(XUIMessageInit* pInitData, BOOL& bHandled) {
	this->GetChildById(L"btnSave", &btnSave);
	this->GetChildById(L"btnLoadDefaults", &btnLoadDefaults);
	this->GetChildById(L"chkOverideWeb", &chkOverideWeb);
	this->GetChildById(L"chkCustomNotify", &chkCustomNotify);
	this->GetChildById(L"chkBackgroundFade", &chkBackgroundFade);
	this->GetChildById(L"chkTileFade", &chkTileFade);
	this->GetChildById(L"chkHudFade", &chkHudFade);
	this->GetChildById(L"chkNotifyFade", &chkNotifyFade);
	this->GetChildById(L"chkTileGradient", &chkTileGradient);
	this->GetChildById(L"chkBackGradient", &chkBackGradient);
	this->GetChildById(L"ePreview", &ePreview);
	this->GetChildById(L"sldRed", &sldRed);
	this->GetChildById(L"sldGreen", &sldGreen);
	this->GetChildById(L"sldBlue", &sldBlue);
	this->GetChildById(L"XuiText1", &XuiText1);

	this->SetTimer(0, 100);

	PopulateCheckboxes(this);

	sldRed.SetValue(cUI.iR);
	sldGreen.SetValue(cUI.iG);
	sldBlue.SetValue(cUI.iB);

	ToggleVisability(this, cUI.bWebPanelOveride);

	bHandled = TRUE;
	return ERROR_SUCCESS;
}

HRESULT designer::scnTabUI::OnNotifyValueChanged(HXUIOBJ hObjSource, XUINotifyValueChanged* pNotifyValueChangedData, BOOL& bHandled) {
	if (hObjSource == sldRed)
		cUI.iR = pNotifyValueChangedData->nValue;
	if (hObjSource == sldBlue)
		cUI.iB = pNotifyValueChangedData->nValue;
	if (hObjSource == sldGreen)
		cUI.iG = pNotifyValueChangedData->nValue;
	bHandled = TRUE;
	return ERROR_SUCCESS;
}

HRESULT designer::scnTabUI::OnTimer(XUIMessageTimer* pTimer, BOOL& bHandled) {
	if (pTimer->nId == 0) {
		ToggleVisability(this, cUI.bWebPanelOveride);
	}
	bHandled = TRUE;
	return ERROR_SUCCESS;
}

HRESULT designer::scnTabUI::OnNotifyPress(HXUIOBJ hObjPressed, BOOL& bHandled) {
	if (hObjPressed == chkOverideWeb) {
		cUI.bWebPanelOveride = chkOverideWeb.IsChecked();
		dashboard::UpdateUIConfig();
	}

	if (hObjPressed == chkCustomNotify)
		cUI.bCustomNotify = chkCustomNotify.IsChecked();

	if (hObjPressed == chkBackgroundFade)
		cUI.bBackgroundFade = chkBackgroundFade.IsChecked();

	if (hObjPressed == chkTileFade)
		cUI.bTileFade = chkTileFade.IsChecked();

	if (hObjPressed == chkHudFade)
		cUI.bHudFade = chkHudFade.IsChecked();

	if (hObjPressed == chkNotifyFade)
		cUI.bNotifyFade = chkNotifyFade.IsChecked();

	if (hObjPressed == chkBackGradient)
		cUI.bBackGradient = chkBackGradient.IsChecked();

	if (hObjPressed == chkTileGradient)
		cUI.bTileGradient = chkTileGradient.IsChecked();

	if (hObjPressed == btnLoadDefaults) {
		dashboard::WriteDefaultUIConfig();
		dashboard::ReadUIConfig();
		PopulateCheckboxes(this);

		sldRed.SetValue(cUI.iR);
		sldGreen.SetValue(cUI.iG);
		sldBlue.SetValue(cUI.iB);

		ToggleVisability(this, cUI.bWebPanelOveride);
	}
	if (hObjPressed == btnSave) {
		dashboard::UpdateUIConfig();
		FNotify(L"Settings saved to INI!").show();
	}
	bHandled = TRUE;
	return ERROR_SUCCESS;
}