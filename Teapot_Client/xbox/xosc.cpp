#include "stdafx.h"
#include "xosc.h"
#include "kv.h"
#include "HvPeekPoke.h"
#include "strings.h"
#include "io.h"

void xosc::computeSecurityHash(PBYTE out) {
	int xConfigDevFlags = 0;
	WORD settingSize = 6;
	byte unknownBuffer[0x10] = { 0 };
	byte securityDigest[0x14];
	byte macAddress[0x6];
	byte smcVersion[0x5];

	PLDR_DATA_TABLE_ENTRY hxam = (PLDR_DATA_TABLE_ENTRY)GetModuleHandle(getStringByIndex(STR_XAM));
	PLDR_DATA_TABLE_ENTRY hkrnl = (PLDR_DATA_TABLE_ENTRY)GetModuleHandle(getStringByIndex(STR_KERNEL));
	PLDR_DATA_TABLE_ENTRY hcurrent = (PLDR_DATA_TABLE_ENTRY)GetModuleHandle(0);
	memcpy(securityDigest, (PBYTE)0x8E03AA40, 0x14);
	unknownBuffer[0xE] &= 0xF8;

	if (hxam && hkrnl && hcurrent) {
		IMAGE_XEX_HEADER* xamHeader = (IMAGE_XEX_HEADER*)(hxam->XexHeaderBase);
		IMAGE_XEX_HEADER* krnlHeader = (IMAGE_XEX_HEADER*)(hkrnl->XexHeaderBase);
		IMAGE_XEX_HEADER* currentHeader = (IMAGE_XEX_HEADER*)(hcurrent->XexHeaderBase);

		if (xamHeader) {
			XEX_SECURITY_INFO* securityInfo = (XEX_SECURITY_INFO*)(xamHeader->SecurityInfo);
			DWORD size = ((xamHeader->SizeOfHeaders - (DWORD)&securityInfo->AllowedMediaTypes) + (DWORD)xamHeader);
			XeCryptSha((PBYTE)&securityInfo->AllowedMediaTypes, size, securityDigest, 0x14, unknownBuffer, 0x10, securityDigest, 0x14); //0x3800026C
			xConfigDevFlags = XCONFIG_DEVKIT_USBD_ROOT_HUB_PORT_DISABLE_MASK;
		}

		if (krnlHeader) {
			if (NT_SUCCESS(ExGetXConfigSetting(XCONFIG_SECURED_CATEGORY, XCONFIG_SECURED_MAC_ADDRESS, macAddress, 0x6, &settingSize))) {
				XEX_SECURITY_INFO* securityInfo = (XEX_SECURITY_INFO*)(krnlHeader->SecurityInfo);
				DWORD size = ((krnlHeader->SizeOfHeaders - (DWORD)&securityInfo->AllowedMediaTypes) + (DWORD)krnlHeader);
				XeCryptSha((PBYTE)&securityInfo->AllowedMediaTypes, size, securityDigest, 0x14, macAddress, 0x6, securityDigest, 0x14); //0x80040D6C
				xConfigDevFlags |= (XCONFIG_DEVKIT_XAM_FEATURE_ENABLE_DISABLE_MASK & 0xFF);
			}
		}

		if (currentHeader) {
			getSMCVersion(smcVersion);
			XEX_SECURITY_INFO* securityInfo = (XEX_SECURITY_INFO*)(currentHeader->SecurityInfo);
			DWORD size = ((currentHeader->SizeOfHeaders - (DWORD)&securityInfo->AllowedMediaTypes) + (DWORD)currentHeader);
			XeCryptSha((PBYTE)&securityInfo->AllowedMediaTypes, size, securityDigest, 0x14, smcVersion, 0x5, securityDigest, 0x14); //0x8E050214
			xConfigDevFlags |= (XCONFIG_DEVKIT_MAX & 0xFF);
		}
	}

	XeCryptSha((PBYTE)0x900101A3, 0x8E59, (PBYTE)securityDigest, 0x14, 0, 0, (PBYTE)securityDigest, 0x14);
	securityDigest[0] = xConfigDevFlags;
	memcpy(out, securityDigest, 0x14);
}

void xosc::execute(PBYTE resp){
	byte securityHash[0x14];
	computeSecurityHash(securityHash);
	memcpy(resp + 0x60, securityHash, 0x10);
}

void xosc::getSMCVersion(LPVOID version) {
	memset(version, 0x00, 0x10);
	BYTE falconSmc[] = { 0x12, 0x31, 0x01, 0x06, 0x00 };
	BYTE coronaSmc[] = { 0x12, 0x62, 0x02, 0x05, 0x00 };
	BYTE trinitySmc[] = { 0x12, 0x51, 0x03, 0x01, 0x00 };
	BYTE jasperSmc[] = { 0x12, 0x41, 0x02, 0x03, 0x00 };
	BYTE zephyrSmc[] = { 0x12, 0x21, 0x01, 0x0A, 0x00 };

	switch (KV::getConsoleType()) {
	case 0x1: memcpy(version, zephyrSmc, 5); break;
	case 0x2: memcpy(version, falconSmc, 5); break;
	case 0x3: memcpy(version, jasperSmc, 5); break;
	case 0x4: memcpy(version, trinitySmc, 5); break;
	case 0x5: memcpy(version, coronaSmc, 5); break;
	}
}