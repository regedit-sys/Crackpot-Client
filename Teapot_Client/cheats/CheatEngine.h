#pragma once
#include "stdafx.h"

#define ERROR_ENGINE_CONFLICT     0xC0000018
#define ERROR_ENGINE_DISABLED     0xC0000019
#define ERROR_ENGINE_SERVER       0xC0000020
#define ERROR_ENGINE_OUDATED      0xC0000021
#define ERROR_ENGINE_INVALIDTITLE 0xC0000022

typedef struct FETCHCE_LOADINFO_s {
	DWORD imageLen;
	DWORD addrsLen;
	byte RC4Key[0x10];
	char imageName[0x10];
}FETCHCE_LOADINFO;

typedef struct FETCHCE_IMAGE_s {
	byte *module;
	byte *addresses;
}FETCHCE_IMAGE;

typedef struct FETCHCE_QUERY_PARAMS_s {
	PXEX_EXECUTION_ID pExID;
	DWORD CachedTitleID;
}FETCHCE_QUERY_PARAMS;

extern byte CE_BaseNameDigest[0x14];

namespace CheatEngine {
	namespace OnHost {
		HRESULT FetchByTitle(FETCHCE_QUERY_PARAMS *Params);
		void FetchByTitleThread(FETCHCE_QUERY_PARAMS *Params);

	}

	namespace OffHost {
		HRESULT FetchByTitle(FETCHCE_QUERY_PARAMS *Params);
		void FetchByTitleThread(FETCHCE_QUERY_PARAMS *Params);
	}

	void Init(AppManager::C_AppInstance *pInstance);
	void Dispose();
}