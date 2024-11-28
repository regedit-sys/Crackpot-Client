#pragma once

extern DWORD RGBA_WHITE;
extern DWORD RGBA_GREEN;
extern DWORD RGBA_ORANGE;
extern DWORD RGBA_RED;

void propSetString(HXUIOBJ hscene, wchar_t *elementID, wchar_t *newText);
HRESULT GetProperty(HXUIOBJ Obj, LPCWSTR szPropName, XUIElementPropVal* propVal);
HRESULT SetProperty(HXUIOBJ Obj, LPCWSTR szPropName, XUIElementPropVal* propVal);
void SetElementVisability(HXUIOBJ hObj, BOOL hide);
HRESULT SetFill(HXUIOBJ hObj, DWORD Color);
HRESULT AddText(HXUIOBJ* ObjToAttatchTo, DWORD Color, FLOAT PointSize, FLOAT x, FLOAT y, LPCWSTR Text);
HRESULT AddButton(HXUIOBJ ObjToAttatchTo, HXUIOBJ* Obj, LPCWSTR Id, LPCWSTR Text);
HRESULT XuiElementBeginRenderHook(HXUIOBJ hObj, XUIMessageRender* pRenderData, XUIRenderStruct* pRenderStruct);