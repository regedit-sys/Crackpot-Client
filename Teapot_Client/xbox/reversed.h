#pragma once
#include "stdafx.h"

namespace reversed{
	NTSTATUS XexLoadImageFromMemory(BYTE* szXexBuffer, DWORD dwSize, const char* pXexName, DWORD dwFlags, DWORD dwMinVersion, HANDLE* pOutModuleHandle);
};