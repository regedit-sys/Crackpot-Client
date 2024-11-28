#include "stdafx.h"
#include "common.h"
#include "Anticheat.h"
#include "util/XeCrypt.h"

detour<bool> tourBo2_CL_DispatchConnectionlessPacket;
detour<int> tourLiveAntiCheat_GiveProbation;
detour<bool> tourbdByteBuffer_Write;
detour<bool> tourGhosts_LiveAntiCheat_ProcessChallengeResponse;
detour<bool> tourBo3_LiveAntiCheat_ProcessChallengeResponseHook;
detour<int> TourNetDll_send;

QWORD RandomMachineID;
BYTE RandomMacAddress[6];

bool CURRENT_MULTIPLAYER = false;
DWORD CURRENT_TITLEID = 0;
DWORD CURRENT_TITLE_VERSION = 0;

wchar_t ExceptionMsg[] = L"Crackpot - Anticheat Bypass Failed! Returning to dash for your protection.";

static BYTE RC4Key[32] = {
	0x90, 0x40, 0x72, 0x38, 0x07, 0x23, 0x02, 0x73, 0x97, 0x9D, 0xD3, 0x78,
	0x22, 0x9A, 0x79, 0x97, 0xF2, 0x3A, 0xFA, 0xFF, 0x78, 0x9F, 0x2E, 0xDF,
	0x87, 0x88, 0x92, 0xAA, 0x88, 0x29, 0x28, 0xB0
};

byte fakeRC4Key[10] = { ~0x54, ~0x65, ~0x61, ~0x70, ~0x6F, ~0x74, ~0x4C, ~0x69, ~0x76, ~0x62 };


__forceinline void Decrypt(PBYTE Data, size_t Length) {
	Native::XeCryptRc4(fakeRC4Key, sizeof fakeRC4Key, Data, Length);
}

typedef struct _COD_CHAL_RESP {
	BYTE        bOnlineIPAddress[4];        // 0x00-0x04
	QWORD       qwMachineId;                // 0x04-0x0C
	BYTE        bMacAddress[6];             // 0x0C-0x12
	BYTE        padding1[2];                // 0x12-0x14
	float       fltLongitude;               // 0x14-0x18
	float       fltLatitude;                // 0x18-0x1C
	WORD        PFreeMem;                   // 0x1C-0x1E
	BYTE        bSecurityFlag;              // 0x1E-0x1F 
	char        cConsoleSerialNumber[12];   // 0x1F-0x2B
	BYTE        padding2[1];                // 0x2B-0x2C
	char        cConsoleId[12];             // 0x2C-0x38
	WORD        wKernelVersion;             // 0x38-0x3A
} COD_CHAL_RESP, *PCOD_CHAL_RESP;

typedef struct _BOPS2_CHAL_RESP {
	BYTE        bDebugValue;
	DWORD       dwUnk1;
	BYTE        bUnk;
	BYTE        bOnlineIPAddress[4];
	DWORD       dwTimeStamp;
	BYTE        bMacAddress[8];
	QWORD       qwMachineId;
	QWORD       qwSerialNumber;
	QWORD       qwConsoleID;
	DWORD       dwKernelVersion;
	DWORD       bSecurityValue;
} BOPS2_CHAL_RESP, *PBOPS2_CHAL_RESP;

typedef struct _BO3_CHAL_RESP {
	BYTE        bOnlineIPAddress[4];
	QWORD       qwMachineId;
	BYTE        bMacAddress[6];
	WORD        PFreeMem;
	BYTE        bSecurityFlag;
	char        cConsoleSerialNumber[13];
	char        cConsoleId[12];
	WORD        wKernelVersion;
} BO3_CHAL_RESP, *PBO3_CHAL_RESP;

void __declspec(naked)GTABypass() {
	__asm {
		lis r30, 0x022C
		ori r30, r30, 0xC800
		stw r30, 0x18(r29)

		lis r3, 0xF6D6
		ori r3, r3, 0xAA59

		nop
	}
}

bool HandleProcessError(wchar_t *txt) {
	debug_sys("[INFO] Anticheat: 0x%08X Failed! Returning to dash for safety.", CURRENT_TITLEID);
	FNotify(txt).error();
	ReturnToDash();
	return false;
}

bool Bo2_CL_DispatchConnectionlessPacketHook(int localClientNum, QWORD netadr1, QWORD netadr2, QWORD netadr3, void* msg, int time) {
	//future references msg length: +0x14 || msg data: +0x8 || cmd data: +0xC

	DWORD pCMD_argv = Native::DecVal(0x35D1EEB8) /*0x821BBBD8*/;
	if (!stricmp(Invoke::Call<char*>(pCMD_argv, 0), &getString(STR_BO2_RELAY)[0])) {
		int clientIndex = atoi(Invoke::Call<char*>(pCMD_argv, 1));

		if (clientIndex < 0 || clientIndex >= 18) {
			debug_sys("[BO2_CRASH] Some asshole tried to crash you.");
			return false;
		}
	}

	return tourBo2_CL_DispatchConnectionlessPacket.callOriginal(localClientNum, netadr1, netadr2, netadr3, msg, time);
}

VOID setLogMessageHook(DWORD bdAntiCheatResponses, const char* logMessage, DWORD logMessageSize) {
	debug_sys("[INFO] Anticheat: Spoofing SetLogMessage -> 0x%08X", CURRENT_TITLEID);
	if (CURRENT_TITLEID == COD_AW) {
		if (logMessageSize != 0) {
			PCOD_CHAL_RESP Response = (PCOD_CHAL_RESP)logMessage;

			byte flag[4] = { 0x49, 0x14, 0x60, CURRENT_MULTIPLAYER ? 0x80 : 0x81 };
			Decrypt(flag, 4);
			Response->bSecurityFlag = *(int*)&flag[0];
			Response->qwMachineId = RandomMachineID;
			Mem::Copy(Response->bMacAddress, RandomMacAddress, 0x6);
		}

		if (logMessage == 0 || logMessageSize == 0) {
			*(DWORD*)(bdAntiCheatResponses + 0x64) = 0;
			memset((PVOID)(bdAntiCheatResponses + 0x22), 0, 0x40);
			return;
		}

		Mem::Copy((PVOID)(bdAntiCheatResponses + 0x22), (PVOID)logMessage, (logMessageSize <= 0x40) ? logMessageSize : 0x40);
		*(DWORD*)(bdAntiCheatResponses + 0x64) = (logMessageSize < 0x40) ? logMessageSize : 0x40;
	}

	if (CURRENT_TITLEID == COD_BO3) {
		if (logMessage == 0 || logMessageSize == 0) {
			*(DWORD*)(bdAntiCheatResponses + 0x64) = 0;
			memset((PVOID)(bdAntiCheatResponses + 0x22), 0, 0x40);
			return;
		}
		if (logMessageSize == 0x30) {
			*(BYTE*)(logMessage + 0x14) = CURRENT_MULTIPLAYER ? 0x3 : 0x2;
		}

		Mem::Copy((PVOID)(bdAntiCheatResponses + 0x22), (PVOID)logMessage, (logMessageSize <= 0x40) ? logMessageSize : 0x40);
		*(DWORD*)(bdAntiCheatResponses + 0x64) = (logMessageSize < 0x40) ? logMessageSize : 0x40;
	}

	if (CURRENT_TITLEID == COD_GHOSTS) {
		if (logMessageSize != 0) { // 0x3A
			PCOD_CHAL_RESP Response = (PCOD_CHAL_RESP)logMessage;
			byte flag[4] = { 0x49, 0x14, 0x60, CURRENT_MULTIPLAYER ? 0x80 : 0x81 };
			Decrypt(flag, 4);
			Response->bSecurityFlag = *(int*)&flag[0]; // Server side this
			Response->qwMachineId = RandomMachineID;
			Mem::Copy(Response->bMacAddress, RandomMacAddress, 0x6);
		}

		if (logMessage == 0 || logMessageSize == 0) {
			*(DWORD*)(bdAntiCheatResponses + 0x60) = 0;
			memset((PVOID)(bdAntiCheatResponses + 0x1E), 0, 0x40);
			return;
		}

		Mem::Copy((PVOID)(bdAntiCheatResponses + 0x1E), (PVOID)logMessage, (logMessageSize <= 0x40) ? logMessageSize : 0x40);
		*(DWORD*)(bdAntiCheatResponses + 0x60) = (logMessageSize < 0x40) ? logMessageSize : 0x40;
	}

	if (CURRENT_TITLEID == COD_BLACK_OPS_2) {
		if (logMessageSize != 0) {
			PBOPS2_CHAL_RESP Response = (PBOPS2_CHAL_RESP)logMessage;

			byte BO2_DebugValue[4] = { 0x49, 0x14, 0x60, 0x82 };
			Decrypt(BO2_DebugValue, 4);

			byte BO2_SecurityValue[4] = { 0x49, 0x14, 0x9B, 0xC1 };
			Decrypt(BO2_SecurityValue, 4);

			Response->bDebugValue = *(int*)&BO2_DebugValue[0]; // Server side this
			Response->bSecurityValue = *(int*)&BO2_SecurityValue[0]; // Server side this
			Response->qwMachineId = RandomMachineID;
			Mem::Copy(Response->bMacAddress, RandomMacAddress, 0x6);
		}

		if (logMessage == 0 || logMessageSize == 0) {
			*(DWORD*)(bdAntiCheatResponses + 0x60) = 0;
			memset((PVOID)(bdAntiCheatResponses + 0x1E), 0, 0x40);
			return;
		}

		Mem::Copy((PVOID)(bdAntiCheatResponses + 0x1E), (PVOID)logMessage, (logMessageSize <= 0x40) ? logMessageSize : 0x40);
		*(DWORD*)(bdAntiCheatResponses + 0x60) = (logMessageSize < 0x40) ? logMessageSize : 0x40;
	}
}


BOOL LiveAntiCheat_ProcessChallengeResponseHook(DWORD controllerIndex, DWORD ChallengeType, DWORD *params, QWORD *Result) {
	debug_sys("[INFO] Anticheat: Spoofing Challenge Response -> 0x%08X", CURRENT_TITLEID);
	if (CURRENT_TITLEID == COD_BLACK_OPS_2) {
		QWORD Seed;
		switch (ChallengeType) {
		case 0x03: {
			Seed = *(QWORD *)*(DWORD *)(*(DWORD *)params + 4);
			*Result = 0;

			byte BO2_Seed1[8] = { 0xCD, 0x63, 0xCD, 0x79, 0xD6, 0xBB, 0x6C, 0x93 };
			Decrypt(BO2_Seed1, 8);

			byte BO2_Seed2[8] = { 0x57, 0x2C, 0x84, 0xE0, 0xFC, 0xBB, 0x93, 0x3C };
			Decrypt(BO2_Seed2, 8);

			byte BO2_Seed3[8] = { 0xCD, 0x63, 0xCD, 0x79, 0xD6, 0xBB, 0x6C, 0x93 };
			Decrypt(BO2_Seed3, 0x8);

			if (Seed == *(long long*)&BO2_Seed1[0]) { // Server side this
				if (CURRENT_MULTIPLAYER) {
					byte BO2_Seed[8] = { 0xBF, 0x45, 0x91, 0xD1, 0x9C, 0xBA, 0x6F, 0x11 };
					Decrypt(BO2_Seed, 8);
					*Result = *(long long*)&BO2_Seed[0];
				}
				else {
					byte BO2_Seed[8] = { 0x4E, 0x87, 0xF5, 0xE5, 0x3B, 0x3B, 0x87, 0x09 };
					Decrypt(BO2_Seed, 8);
					*Result = *(long long*)&BO2_Seed[0];
				}
				return true;
			}
			else if (Seed == *(long long*)&BO2_Seed2[0]) { // Server side this
				if (CURRENT_MULTIPLAYER) {
					byte BO2_Flag[8] = { 0xCE, 0x12, 0x05, 0xBF, 0xBB, 0x2A, 0x84, 0x5C };
					Decrypt(BO2_Flag, 8);
					*Result = *(long long*)&BO2_Flag[0];
				}
				else {
					byte BO2_Flag[8] = { 0x9E, 0x19, 0xAB, 0x68, 0x24, 0xB1, 0xB9, 0x2E };
					Decrypt(BO2_Flag, 8);
					*Result = *(long long*)&BO2_Flag[0];
				}
				return true;
			}
			return HandleProcessError(ExceptionMsg);
		}case 0x04: {
			char chDvar[0x30];
			Mem::Copy(chDvar, (char*)*(DWORD*)(*(DWORD*)params + 4), *(DWORD*)(*(DWORD*)params + 8));

			char cg_drawFPS[11];
			char g_speed[8];
			buildCharStr(cg_drawFPS, ~'c', ~'g', ~'_', ~'d', ~'r', ~'a', ~'w', ~'F', ~'P', ~'S', ~'\0');
			buildCharStr(g_speed, ~'g', ~'_', ~'s', ~'p', ~'e', ~'e', ~'d', ~'\0');
			if (!memcmp(chDvar, cg_drawFPS, 0x0A)) { // Encrypt strings
				*Result = 0;
				return true;
			}
			else if (!memcmp(chDvar, g_speed, 0x07)) { // Encrypt strings
				byte BO2_GSpeedVal[4] = { 0x49, 0x14, 0x60, 0x3D };
				Decrypt(BO2_GSpeedVal, 4);
				*Result = *(int*)&BO2_GSpeedVal[0];
				return true;
			}
			return false; //??
		}case 0x07: {
			*Result = 0; // Encrpyt / Server side this just to be annoying
			return true;
		}case 0x08: {
			*Result = 0; // Encrpyt / Server side this just to be annoying
			return true;
		}default: {
			return HandleProcessError(ExceptionMsg);		}
		}
	}

	if (CURRENT_TITLEID == COD_AW) {
		QWORD Seed;
		switch (ChallengeType) {
		case 0xC8: {
			Seed = *(QWORD *)*(DWORD *)(*(DWORD *)params + 4);
			*Result = 0;

			// This will not work if it's not casted to DWORD, trust me...
			byte AW_Seed[4] = { 0x49, 0x1E, 0x3B, 0x7D };
			Decrypt(AW_Seed, 4);
			if (Seed == (DWORD)*(int*)&AW_Seed[0]) { // Server side this
				if (CURRENT_MULTIPLAYER) {
					byte AW_Flag[4] = { 0xB4, 0x2C, 0x5B, 0xBE };
					Decrypt(AW_Flag, 4);
					*Result = *(int*)&AW_Flag[0] & 0xFFFFFFFF;
				}
				else {
					byte AW_Flag[4] = { 0x42, 0x43, 0xFE, 0xA4 };
					Decrypt(AW_Flag, 4);
					*Result = *(int*)&AW_Flag[0] & 0xFFFFFFFF;
				}
				return true;
			}
			return HandleProcessError(ExceptionMsg);

		}default: {
			return HandleProcessError(ExceptionMsg);
		}
		}
		return true;
	}

	return HandleProcessError(ExceptionMsg);
	return false;
}

bool Bo3_LiveAntiCheat_ProcessChallengeResponseHook(DWORD memoryHashInfo) {
	if (*(int*)(memoryHashInfo + 0x0C) == 0 || *(int*)memoryHashInfo == 1) {
		QWORD seed = *(long long *)(memoryHashInfo + 0x18);
		UINT start = *(UINT*)(memoryHashInfo + 0x04);
		UINT length = *(UINT*)(memoryHashInfo + 0x08);
		DWORD crc32 = 0;

		byte data[0x21];
		Mem::Copy(data, Teapot::Session, 0x10);
		Mem::Copy(data + 0x10, &seed, 0x8);
		Mem::Copy(data + 0x18, &length, 0x4);
		Mem::Copy(data + 0x1C, &start, 0x4);
		Mem::Copy(data + 0x20, &CURRENT_MULTIPLAYER, 0x1);

		NET::CLIENT nclient;
		if (SUCCEEDED(nclient.create(true))) {
			if (SUCCEEDED(nclient.SendCommand(NET_CMD_BYPASS_BO3, data, sizeof(data), &crc32, sizeof(crc32)))) {
				if (seed != 0) {
					*(DWORD*)(memoryHashInfo + 0x18) = 0;
					*(DWORD*)(memoryHashInfo + 0x1C) = crc32;
					*(DWORD*)(memoryHashInfo + 0x0C) = *(DWORD*)(memoryHashInfo + 0x08);
					debug_sys("[INFO] Anticheat: Spoofing Challenge Response -> 0x%08X", CURRENT_TITLEID);
					return tourBo3_LiveAntiCheat_ProcessChallengeResponseHook.callOriginal(memoryHashInfo);
				}
			}
		}
	}

	return HandleProcessError(ExceptionMsg);
}

BOOL Ghosts_LiveAntiCheat_ProcessChallengeResponseHook(DWORD MemoryHashStruct) {
	WORD ChallengeType = *(WORD*)(MemoryHashStruct + 0x8);
	DWORD MemoryHashInfo = *(DWORD*)(MemoryHashStruct + 0x34);
	if (!MemoryHashInfo) return tourGhosts_LiveAntiCheat_ProcessChallengeResponse.callOriginal(MemoryHashStruct);
	debug_sys("[INFO] Anticheat: Spoofing Challenge Response -> 0x%08X", CURRENT_TITLEID);

	switch (ChallengeType) {
	case 0xC8: {
		if (*(DWORD*)MemoryHashInfo != 1) {
			return HandleProcessError(ExceptionMsg);
		}

		if (*(DWORD*)(MemoryHashInfo + 0x0C) == 0) {
			byte Ghosts_Seed[4] = { 0x49, 0x14, 0x50, 0xBA };
			Decrypt(Ghosts_Seed, 4);

			if (*(QWORD*)(MemoryHashInfo + 0x20) == (WORD)*(int*)&Ghosts_Seed[0]) { // Server side
				*(QWORD*)(MemoryHashInfo + 0x20) = 0;

				if (CURRENT_MULTIPLAYER) {
					byte Ghosts_Hash[4] = { 0xB7, 0xD5, 0xE8, 0x63 };
					Decrypt(Ghosts_Hash, 4);
					*(DWORD*)(MemoryHashInfo + 0x24) = *(int*)&Ghosts_Hash[0];
				}
				else {
					byte Ghosts_Hash[4] = { 0xA5, 0x1C, 0x01, 0x2A };
					Decrypt(Ghosts_Hash, 4);
					*(DWORD*)(MemoryHashInfo + 0x24) = *(int*)&Ghosts_Hash[0];
				}
				*(DWORD*)(MemoryHashInfo + 0x0C) = *(DWORD*)(MemoryHashInfo + 0x08);
				return tourGhosts_LiveAntiCheat_ProcessChallengeResponse.callOriginal(MemoryHashStruct);
			}

			return HandleProcessError(ExceptionMsg);
		}
		return HandleProcessError(ExceptionMsg);
	}
	default: {
		return HandleProcessError(ExceptionMsg);
	}
	}
}

int LiveAntiCheat_GiveProbationHook(int controllerIndex, int gameMode, float time) {
	return tourLiveAntiCheat_GiveProbation.callOriginal(controllerIndex, gameMode, 0.0f);
}

DWORD XexGetModuleHandleHook(PSZ moduleName, PHANDLE hand) {
	if (moduleName != NULL)
		if (memcmp(moduleName, getString(STR_XBDM).cstr, 4) == 0) {
			*hand = 0;
			return 0xC0000225;
		}
	return Native::XexGetModuleHandle(moduleName, hand);
}

VOID GenerateConsoleInformation() {
	BYTE* MachineID = (BYTE*)Native::Malloc(0x8);

	MachineID[0] = 0xFA;
	MachineID[1] = 0x00;
	MachineID[2] = 0x00;
	MachineID[3] = 0x00;
	MachineID[4] = 0x04;

	randomBytes((BYTE*)(MachineID + 4), 4);
	Mem::Copy(&RandomMachineID, MachineID, 8);
	Native::Free(MachineID);

	RandomMacAddress[0] = 0x00;
	RandomMacAddress[1] = 0x22;
	RandomMacAddress[2] = 0x48;

	randomBytes((BYTE*)(RandomMacAddress + 3), 3);
	debug("[INFO] Anticheat - Generated Random MAC: %s", BytesToHexStr(RandomMacAddress, 6).cstr);
}

DWORD XNetXnAddrToMachineIdHook(XNCALLER_TYPE xnc, const XNADDR* pxnaddr, QWORD* pqwMachineId) {
	Mem::Copy((BYTE*)pxnaddr->abEnet, RandomMacAddress, 6);
	*(QWORD*)&pxnaddr->abOnline[8] = _byteswap_uint64(*(QWORD*)&RandomMachineID);
	*pqwMachineId = RandomMachineID;
	return ERROR_SUCCESS;
}

bool bdByteBuffer_WriteHook(int* buffer, const void* data, int size) {
	if (size == 0x6) {
		Mem::Copy((BYTE*)data, &RandomMacAddress, 6);
		debug_sys("[INFO] Anticheat: MAC spoofed -> 0x%08X", CURRENT_TITLEID);
	}
	else if (size == 0x8 && *(byte*)data == 0xFA) {
		Mem::Copy((BYTE*)data, &RandomMachineID, 8);
		debug_sys("[INFO] Anticheat: Machine ID spoofed -> 0x%08X", CURRENT_TITLEID);
	}
	return tourbdByteBuffer_Write.callOriginal(buffer, data, size);
}

int NetDll_sendHook(XNCALLER_TYPE xnc, SOCKET s, BYTE* buf, int len, int flags) {
	if (len == 0x54) {
		// Set our static hash
		memset((BYTE*)(buf + 0x15), (CURRENT_MULTIPLAYER ? 0x87D153B5 : 0xBF5F8BB0), 0x4);

		// Wipe the seed number from buffer
		memset((BYTE*)(buf + 0xC), 0, 4);

		// Set the seed number statically to 1 so they believe we are hashing the same seed over and over
		buf[0xC] = 1;
	}

	return TourNetDll_send.callOriginal(xnc, s, buf, len, flags);
}

HRESULT CheckVersion(AppManager::C_AppInstance *pInstance, DWORD VersionNum) {
	if (pInstance->Version != VersionNum) {
		FNotify(L"Crackpot - Your game needs to be updated!").show(false, false, 4000);
		debug("[INFO] Title is out of date.");
		return E_FAIL;
	}
	return ERROR_SUCCESS;
}

bool AntiCheatBypass::Install(AppManager::C_AppInstance *pInstance) {
	if (!AppMgrIsTitleApplication(pInstance)) return false;
	CURRENT_MULTIPLAYER = (pInstance->CachedDllNameW == L"default_mp.xex");
	if (pInstance->TitleID == COD_BO3) CURRENT_MULTIPLAYER = (pInstance->CachedDllNameW == L"default.xex");
	CURRENT_TITLEID = pInstance->TitleID;

	switch (pInstance->TitleID) {
	case COD_BO3:
		if (!ini::BP_BO3 && !ini::NoKVMode) break;
		if (FAILED(CheckVersion(pInstance, 8))) break;
		GenerateConsoleInformation();

		CURRENT_MULTIPLAYER = (pInstance->CachedDllNameW == L"default.xex");
		debug_sys("IS MULTIPLAYER? %s", CURRENT_MULTIPLAYER ? "TRUE" : "FALSE");

		tourBo3_LiveAntiCheat_ProcessChallengeResponseHook.setupDetour(CURRENT_MULTIPLAYER ? 0x82539C78 : 0x82538EB0, Bo3_LiveAntiCheat_ProcessChallengeResponseHook);
		Native::PatchInJump(CURRENT_MULTIPLAYER ? (PDWORD)0x827B54C8 : (PDWORD)0x827A75C8, (int)setLogMessageHook, false, true);
		break;
	case COD_AW:
		if (!ini::BP_GHOSTS && !ini::NoKVMode) break;
		if (FAILED(CheckVersion(pInstance, 17))) break;
		GenerateConsoleInformation();

		tourbdByteBuffer_Write.setupDetour(CURRENT_MULTIPLAYER ? 0x821B6360 : 0x826BE250, bdByteBuffer_WriteHook);
		Native::PatchInJump(CURRENT_MULTIPLAYER ? (PDWORD)0x822CC740 : (PDWORD)0x8258C060, (DWORD)LiveAntiCheat_ProcessChallengeResponseHook);
		Native::PatchInJump(CURRENT_MULTIPLAYER ? (PDWORD)0x821DF510 : (PDWORD)0x826D9F28, (DWORD)setLogMessageHook, false, true);
		break;
	case COD_GHOSTS:
		if (!ini::BP_GHOSTS && !ini::NoKVMode) break;
		if (FAILED(CheckVersion(pInstance, 17))) break;
		GenerateConsoleInformation();

		tourbdByteBuffer_Write.setupDetour(CURRENT_MULTIPLAYER ? 0x827A0108 : 0x826320B0, bdByteBuffer_WriteHook);
		tourGhosts_LiveAntiCheat_ProcessChallengeResponse.setupDetour(CURRENT_MULTIPLAYER ? 0x82629F98 : 0x825131D8, Ghosts_LiveAntiCheat_ProcessChallengeResponseHook);
		Native::PatchInJump(CURRENT_MULTIPLAYER ? (PDWORD)0x827BE130 : (PDWORD)0x82647700, (DWORD)setLogMessageHook, false, true);
		break;
	case COD_BLACK_OPS_2:
		if (!ini::BP_GHOSTS && !ini::NoKVMode) break;
		if (FAILED(CheckVersion(pInstance, 18))) break;
		GenerateConsoleInformation();

		if (CURRENT_MULTIPLAYER) {
			tourBo2_CL_DispatchConnectionlessPacket.setupDetour(Native::DecVal(0xA514C7B8) /*0x82277888*/, Bo2_CL_DispatchConnectionlessPacketHook);
			tourLiveAntiCheat_GiveProbation.setupDetour(0x825C5330, LiveAntiCheat_GiveProbationHook);
		}

		tourbdByteBuffer_Write.setupDetour(CURRENT_MULTIPLAYER ? 0x8286A348 : 0x8264D4D0, bdByteBuffer_WriteHook);
		PatchModuleImport(pInstance->pLDR, getString(STR_KERNEL).cstr, 405, (DWORD)XexGetModuleHandleHook);
		Native::PatchInJump(CURRENT_MULTIPLAYER ? (PDWORD)0x8259A5E0 : (PDWORD)0x824A7C50, (DWORD)LiveAntiCheat_ProcessChallengeResponseHook);
		Native::PatchInJump(CURRENT_MULTIPLAYER ? (PDWORD)0x82884CC0 : (PDWORD)0x826592D0, (DWORD)setLogMessageHook, false, true);
		break;
	case GTA_V:
		/*if (!ini::BP_GTAV) break;
		Mem::Copy((PVOID)0x82FDB57C, &GTABypass, 0x18);
		*(int*)0x835287B4 = 0x60000000;
		*(int*)0x827D2164 = 0x60000000;
		*(int*)0x83288A30 = 0x48000104;
		*(int*)0x82CF7828 = 0x39400001;
		*(int*)0x82CF7704 = 0x480000F8;*/
		break;
	default:
		break;
	}

	return CURRENT_MULTIPLAYER;
}