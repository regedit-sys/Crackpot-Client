#pragma once
#include "../../security/natives.h"

namespace dashboard {
	namespace XUIStructures {

		struct XUIElementPropVal {
			XUI_ELEMENT_PROPERTY_TYPE type;
			union {
				BOOL bVal;
				int nVal;
				unsigned int uVal;
				float fVal;
				WCHAR* szVal;
				D3DVECTOR vecVal;
				XUIQuaternion quatVal;
				IXUIPropObj* pObj;
				struct {
					void* pvCustomData;
					XUICustomPropHandler* pHandler;
				} CustomVal;
			};


#ifdef __cplusplus
			XUIElementPropVal() {
				((void(*)(...))(Native::resolveFunction("dash.xex", 10079)))(this);
			}
			~XUIElementPropVal() {
				((void(*)(...))(Native::resolveFunction("dash.xex", 10080)))(this);
			}
#endif
		};

		struct XUIElementPropDef {
			DWORD dwFlags;
			DWORD dwId;
			DWORD dwOffset;
			DWORD dwExtra;
			LPCWSTR szPropName;
			XUI_ELEMENT_PROPERTY_TYPE Type;
			const XUIElementPropVal* pDefaultVal;
			XUICustomPropHandler* pCustomHandler;
			PFN_GETPROP pfnGetProp;
			PFN_SETPROP pfnSetProp;
			PFN_GETPROPDEF pfnGetPropDef;
			PFN_GETCOUNT pfnGetCount;

#ifndef _XBOX
			LPCWSTR szPropertyEditor;
#endif
		};

		struct XUIClass {
			DWORD cbSize;
			LPCWSTR szClassName;
			LPCWSTR szBaseClassName;
			LPCWSTR szClassDetails;
			XUIObjectMethods Methods;
			const XUIElementPropDef* pPropDefs;
			DWORD dwPropDefCount;
			BOOL bExtensionClass;
		};
	}
	namespace XUI {
		extern HRESULT(*XuiElementGetId)(HXUIOBJ hObj, LPCWSTR* pszId);
		extern HRESULT(*XuiElementGetParent)(HXUIOBJ hObj, HXUIOBJ* phParent);
		extern HRESULT(*XuiFigureSetFill)(HXUIOBJ hObj, XUI_FILL_TYPE nFillType, DWORD dwFillColor, XUIGradientStop* pStops, int nNumStops, float fGradientAngle, const D3DXVECTOR2* pvScale, const D3DXVECTOR2* pvTrans);
		extern HRESULT(*XuiObjectGetProperty)(HXUIOBJ hObj, DWORD dwPropId, DWORD dwIndex, XUIStructures::XUIElementPropVal* pVal);
		extern HRESULT(*XuiObjectSetProperty)(HXUIOBJ hObj, DWORD dwPropId, DWORD dwIndex, const XUIStructures::XUIElementPropVal* pVal);
		extern HRESULT(*XuiObjectGetPropertyId)(HXUIOBJ hObj, LPCWSTR szPropName, DWORD* pdwId);

		void resolveOrdinals(HANDLE hDash);
		HRESULT SetFill(HXUIOBJ hObj, DWORD Color, bool DoBackgroundHacks);
		HRESULT SetProperty(HXUIOBJ Obj, LPCWSTR szPropName, XUIStructures::XUIElementPropVal* propVal);
		HRESULT GetProperty(HXUIOBJ Obj, LPCWSTR szPropName, XUIStructures::XUIElementPropVal* propVal);
	}
}