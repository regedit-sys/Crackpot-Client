#include "stdafx.h"
#include "DashXUI.h"
#include "dash.h"

//using namespace dashboard;

HRESULT(*dashboard::XUI::XuiElementGetId)(HXUIOBJ hObj, LPCWSTR* pszId);
HRESULT(*dashboard::XUI::XuiElementGetParent)(HXUIOBJ hObj, HXUIOBJ* phParent);
HRESULT(*dashboard::XUI::XuiFigureSetFill)(HXUIOBJ hObj, XUI_FILL_TYPE nFillType, DWORD dwFillColor, XUIGradientStop* pStops, int nNumStops, float fGradientAngle, const D3DXVECTOR2* pvScale, const D3DXVECTOR2* pvTrans);
HRESULT(*dashboard::XUI::XuiObjectGetProperty)(HXUIOBJ hObj, DWORD dwPropId, DWORD dwIndex, XUIStructures::XUIElementPropVal* pVal);
HRESULT(*dashboard::XUI::XuiObjectSetProperty)(HXUIOBJ hObj, DWORD dwPropId, DWORD dwIndex, const XUIStructures::XUIElementPropVal* pVal);
HRESULT(*dashboard::XUI::XuiObjectGetPropertyId)(HXUIOBJ hObj, LPCWSTR szPropName, DWORD* pdwId);

void dashboard::XUI::resolveOrdinals(HANDLE hDash) {
	dashboard::XUI::XuiElementGetId = (HRESULT(*)(HXUIOBJ, LPCWSTR*))(DWORD)Native::resolveFunction(hDash, 10176);
	dashboard::XUI::XuiElementGetParent = (HRESULT(*)(HXUIOBJ, HXUIOBJ*))(DWORD)Native::resolveFunction(hDash, 10157);
	dashboard::XUI::XuiFigureSetFill = (HRESULT(*)(HXUIOBJ, XUI_FILL_TYPE, DWORD, XUIGradientStop*, int, float, const D3DXVECTOR2*, const D3DXVECTOR2*))Native::resolveFunction(hDash, 10210);
	dashboard::XUI::XuiObjectGetProperty = (HRESULT(*)(HXUIOBJ, DWORD, DWORD, XUIStructures::XUIElementPropVal*))Native::resolveFunction(hDash, 10047);
	dashboard::XUI::XuiObjectSetProperty = (HRESULT(*)(HXUIOBJ, DWORD, DWORD, const XUIStructures::XUIElementPropVal*))Native::resolveFunction(hDash, 10048);
	dashboard::XUI::XuiObjectGetPropertyId = (HRESULT(*)(HXUIOBJ, LPCWSTR, DWORD*))Native::resolveFunction(hDash, 10044);
}

HRESULT dashboard::XUI::SetFill(HXUIOBJ hObj, DWORD Color, bool DoBackgroundHacks) {
	XUI_FILL_TYPE fillType = DoBackgroundHacks ? XUI_FILL_RADIAL_GRADIENT : XUI_FILL_SOLID;
	D3DXVECTOR2 pvScalingFactor(1.0f, 1.0f), pTrnas(0.0f, 0.0f);
	XUIGradientStop gs[2];
	gs[0].dwColor = Color;
	gs[0].fPos = 0.1f;
	gs[1].dwColor = DoBackgroundHacks ? 0xFF1b1c1a : Color;
	gs[1].fPos = 1.0f;
	return dashboard::XUI::XuiFigureSetFill(hObj, dashboard::cUI.bBackGradient? XUI_FILL_RADIAL_GRADIENT : XUI_FILL_SOLID, Color, gs, 2, 360, &pvScalingFactor, &pTrnas);
}

HRESULT dashboard::XUI::SetProperty(HXUIOBJ Obj, LPCWSTR szPropName, XUIStructures::XUIElementPropVal* propVal) {
	HRESULT result = E_FAIL;
	DWORD propID = 0;
	if (NT_SUCCESS(result = XuiObjectGetPropertyId(Obj, szPropName, &propID)))
		result = XuiObjectSetProperty(Obj, propID, 0, propVal);
	return result;
}

HRESULT dashboard::XUI::GetProperty(HXUIOBJ Obj, LPCWSTR szPropName, XUIStructures::XUIElementPropVal* propVal) {
	HRESULT result = E_FAIL;
	DWORD propID = 0;
	if (NT_SUCCESS(result = XuiObjectGetPropertyId(Obj, szPropName, &propID)))
		result = XuiObjectGetProperty(Obj, propID, 0, propVal);
	return result;
}