#include "stdafx.h"
#include "xui.h"
#include "dashboard/dash.h"

DWORD RGBA_WHITE = D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 255);
DWORD RGBA_GREEN = D3DCOLOR_RGBA(50, 205, 50, 255);
DWORD RGBA_ORANGE = D3DCOLOR_RGBA(0xFF, 0x8C, 0x00, 255);
DWORD RGBA_RED = D3DCOLOR_RGBA(0xFF, 0x00, 0x00, 255);

void propSetString(HXUIOBJ hscene, wchar_t *elementID, wchar_t *newText) {
	HXUIOBJ child; DWORD id;
	if (XuiElementGetChildById(hscene, elementID, &child) == 0) {
		XUIElementPropVal prop;
		XUIElementPropVal_Construct(&prop);
		XUIElementPropVal_SetString(&prop, newText);
		if (XuiObjectGetPropertyId(child, L"Text", &id) == 0) XuiObjectSetProperty(child, id, NULL, &prop);
		XUIElementPropVal_Destruct(&prop);
	}
}

HRESULT GetProperty(HXUIOBJ Obj, LPCWSTR szPropName, XUIElementPropVal* propVal) {
	HRESULT result = E_FAIL;
	DWORD propID = 0;
	if (NT_SUCCESS(result = XuiObjectGetPropertyId(Obj, szPropName, &propID)))
		result = XuiObjectGetProperty(Obj, propID, 0, propVal);
	return result;
}

HRESULT SetProperty(HXUIOBJ Obj, LPCWSTR szPropName, XUIElementPropVal* propVal) {
	HRESULT result = E_FAIL;

	DWORD propID = 0;
	if (NT_SUCCESS(result = XuiObjectGetPropertyId(Obj, szPropName, &propID)))
		result = XuiObjectSetProperty(Obj, propID, 0, propVal);
	return result;
}

HRESULT SetFill(HXUIOBJ hObj, DWORD Color) {
	//XUI_FILL_TYPE fillType = XUI_FILL_TYPE::XUI_FILL_SOLID;
	
	D3DXVECTOR2 pvScalingFactor(1.0f, 1.0f), pTrnas(0.0f, 0.0f);
	XUIGradientStop gs[2];
	gs[0].dwColor = Color;
	gs[0].fPos = 0.0f;
	gs[1].dwColor = Color;
	gs[1].fPos = 1.0f;
	return XuiFigureSetFill(hObj, dashboard::cUI.bTileGradient ? XUI_FILL_RADIAL_GRADIENT  : XUI_FILL_SOLID, Color, gs, 2, 0, &pvScalingFactor, &pTrnas);
}

void SetElementVisability(HXUIOBJ hObj, BOOL hide) {
	XUIElementPropVal Element;
	XUIElementPropVal_SetBool(&Element, hide);
	SetProperty(hObj, L"Show", &Element);
}

HRESULT AddText(HXUIOBJ* ObjToAttatchTo, DWORD Color, FLOAT PointSize, FLOAT x, FLOAT y, LPCWSTR Text) {
	D3DXVECTOR3 pos;
	XUIElementPropVal propVal;
	HXUIOBJ obj;


	XuiCreateObject(XUI_CLASS_TEXT, &obj);

	XuiElementSetBounds(obj, 255.0, 28.0);

	pos.x = x; pos.y = y; pos.z = 0;
	XuiElementSetPosition(obj, &pos);

	propVal.SetVal(PointSize);
	SetProperty(obj, L"PointSize", &propVal);

	propVal.SetColorVal(Color);
	SetProperty(obj, L"TextColor", &propVal);

	XuiTextElementSetText(obj, Text);
	Native::XuiElementAddChild(*ObjToAttatchTo, obj);

	return ERROR_SUCCESS;
}

HRESULT AddButton(HXUIOBJ ObjToAttatchTo, HXUIOBJ* Obj, LPCWSTR Id, LPCWSTR Text) {
	XUIElementPropVal propVall;
	D3DXVECTOR3 pos;
	HXUIOBJ hBtnLastChild;
	LPCWSTR LastButtonId;
	XuiElementGetLastChild(ObjToAttatchTo, &hBtnLastChild);
	XuiElementGetId(hBtnLastChild, &LastButtonId);
	propVall.SetVal(Id);
	SetProperty(hBtnLastChild, L"NavDown", &propVall);
	XuiElementGetPosition(hBtnLastChild, &pos);
	XUIElementPropVal propVal;
	XuiCreateObject(XUI_CLASS_BUTTON, Obj);
	XuiElementSetBounds(*Obj, 323.0, 28.0);
	pos.y += 28;
	XuiElementSetPosition(*Obj, &pos);
	propVal.SetVal(Id);
	SetProperty(*Obj, L"Id", &propVal);
	propVal.SetVal(Text);
	SetProperty(*Obj, L"Text", &propVal);
	propVal.SetVal(L"XuiButtonGuide");
	SetProperty(*Obj, L"Visual", &propVal);
	propVal.SetVal(LastButtonId);
	SetProperty(*Obj, L"NavUp", &propVal);
	Native::XuiElementAddChild(ObjToAttatchTo, *Obj);

	return ERROR_SUCCESS;
}