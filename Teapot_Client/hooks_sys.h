#pragma once

#define XOSCHalSendSMCMessagePtr 0x900166A0
#define XamLoaderExecuteAsyncChallengePtr 0x8169CCD0//0x8169CB08
#define XOSKVDigestPtr 0x8E03AA40
#define XOSCDiskVerification 0x8E038780

extern AsyncEvent AsyncPresenceEvent;

 static bool isBadTitle(DWORD title) {
	switch (title) {
		case 0xFFFF0055: // Xex Menu
		case 0xC0DE9999: // Xex Menu alt
		case 0xFFFE07FF: // XShellXDK
		case 0xF5D20000: // FSD
		case 0xFFFF011D: // DashLaunch
		case 0xFFFEFF43: // Xell Launch GOD
		case 0xFEEDC0DE: // XYZProject
		case 0xFFED0707: // SNES360
		case 0x00000176: // XM360
		case 0x00000167: // FSD3
		case 0x00FBAFBA: // FinalBurnAlpha
		case 0x00000174: // MAME360
		case 0x00000171: // FCE360
		case 0xFFFF051F: // Atari2600
		case 0x1CED2911: // PS1Emu
		case 0x00000115: // GenisisPlus
		case 0x00000189: // Simple360NandFlasher
		case 0x00000155: // PSCRX
		case 0x00000184: // OpenBOR
		case 0x58550B60: // Custom Game
		case 0x58550E53: // Shutshimi
		case 0x00000166: // Aurora 
		case 0x00000172: // DSon360
		case 0x00000198: // Demon Nand Switcher
		case 0xFFED7301: // GameboyAdvanceEmu
		case 0x00000191: // Rick360
		case 0x00000197: // ppsp
		case 0x00000193: // Raw360
		case 0x00000192: // Rem360
		case 0x00000186: // MilkyTracker 
		case 0x00000194: // Exult360
		case 0x00000187: // xBermuda360
		case 0x00000173: // PCEngine
		case 0xFFFFA500: // Amiga360
		case 0x00000195: // XeBoyAdvance 
		case 0x75447253: // Q2Custom
		case 0x07022007: // NeoGeo Pocket 
		case 0x00000188: // Flash360
		case 0x00001039: // SNES360PAL2
		case 0x00000175: // SMS360
		case 0x03172006: // PCEngine
		case 0x00000185: // OpenJazz360 
		case 0x00000169: // Capcom CPXIII
		case 0x00000183: // Quake3_360
		case 0x00000182: // Virtual Jaguar
		case 0x00000177: // NXE2GOD
		case 0xFFED7300: // SegaMegaDrive
		case 0x00000178: // SuperMarioDrive
		case 0x00000179: // FFPlay
		case 0x00000170: // XeXMenu2.0
		case 0x00000180: // D0SB0x
		case 0x000003D0: // 3D0
		case 0x00000119: // ScumVm
		case 0x00000181: // Play System 3 Emu
		case 0x66776655: // Star Control 2
		case 0xD008D009: // Doom Legacy
		case 0x53544232: // SuperTransBall2
		case 0xFFFF7269: // RiseofTheTriad
		case 0x0A600853: // Decent
		case 0x00C76C3B: // Mad Bomber 
		case 0x00000001: // FxMenu(XexMenuEdit)
		case 0x00000196: // xCBR
		case 0x08192006: // Pokemon Mini
		case 0x10022007: // MSX
		case 0x07162006: // Commodore64
		case 0x08152006: // Supervision
		case 0x03202006: // Atari Lnyx 
		case 0x10052007: // Intellivision
		case 0x08272006: // Atari7800
		case 0x10082007: // Atari5200
		case 0x07012007: // Wonderswan
		case 0x10122007: // AmStrad CMC
		case 0x4D5307DF: return true; //unknown homebrew title
		default:return false;
	}
	return false;
}

typedef DWORD(*ExecuteSupervisorChallenge_t)(DWORD dwTaskParam1, char* pbDaeTableName, DWORD cbDaeTableName, PBYTE respPtr, DWORD size);
//DWORD XamLoaderExecuteAsyncChallengeHook(DWORD ExecuteSupervisorChallengeAddress, DWORD dwTaskParam1, char* pbDaeTableName, DWORD cbDaeTableName, PBYTE challenge, DWORD size);

void EventListener();
extern HRESULT SystemHooks_init();

NTSTATUS XexLoadExecutableHook(PCHAR szXexName, PHANDLE pHandle, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion);
NTSTATUS XexLoadImageHook(LPCSTR szXexName, DWORD dwModuleTypeFlags, DWORD dwMinimumVersion, PHANDLE pHandle);
void InstallThreadProcServiceSystemTasks();