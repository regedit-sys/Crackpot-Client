#include "stdafx.h"
#include "common.h"
#include "xbox/Devkit.h"
#include "Teapot.h"

HANDLE EventTeapotInit;
void InitNativeSystem();

extern "C" int XapiInitProcess();
extern "C" int XapiCallThreadNotifyRoutines(int);
extern "C" int XapiPAL50Incompatible();
extern "C" int XamTerminateTitle();
extern "C" int _mtinit();
extern "C" int _rtinit();
extern "C" int _cinit(int);
extern "C" int _cexit(int);
extern "C" int _CRT_INIT(...);
extern "C" int __CppXcptFilter(...);
extern "C" static int __proc_attached;

BOOL APIENTRY DllMain(HANDLE module, DWORD res, LPVOID lpR);
unsigned char branchReplaceData[0x2C] = { 't', 'p', 'l', 'm', 'b', 'u', 0, 0, (sizeof(branchReplaceData) >> 24) & 0xFF, (sizeof(branchReplaceData) >> 16) & 0xFF, (sizeof(branchReplaceData) >> 8) & 0xFF, sizeof(branchReplaceData) & 0xFF };

#pragma code_seg(push, r1, ".ptext")
#pragma comment(linker, "/entry:tplEntry")
#pragma optimize("", off)
__forceinline void _REVERSEBYTES(byte* start, int size) {
	byte* lo = start;
	byte* hi = start + size - 1;
	byte swap;
	while (lo < hi) {
		swap = *lo;
		*lo++ = *hi;
		*hi-- = swap;
	}
}

__forceinline void _RC4(PBYTE key, DWORD keyLen, PBYTE Data, DWORD size, byte seed) {
	byte num;
	int num2;
	BYTE buffer[0x100];
	BYTE buffer2[0x100];
	byte keyChecksum = 0;

	_REVERSEBYTES(key, keyLen);
	for (DWORD i = 0; i < keyLen; ++i) {
		keyChecksum += (key[i] ^ key[i]);
		keyChecksum = keyChecksum % 0xFF;
	}
	for (num2 = 0; num2 < 0x100; num2++) {
		buffer[num2] = (byte)num2;
		buffer2[num2] = key[num2 % keyLen];
	}
	int index = 0;
	for (num2 = 0; num2 < 0x100; num2++) {
		index = ((index + buffer[num2]) + buffer2[num2]) % 0x100;
		num = buffer[num2];
		buffer[num2] = buffer[index];
		buffer[index] = num;
	}
	num2 = index = 0;
	for (DWORD i = 0; i < size; i++) {
		num2 = (num2 + 1) % 0x100;
		index = (index + buffer[num2]) % 0x100;
		num = buffer[num2];
		buffer[num2] = buffer[index];
		buffer[index] = num;
		int num5 = (buffer[num2] + buffer[index]) % 0x100;
		Data[i] = (byte)(Data[i] ^ buffer[num5]);
		Data[i] = (byte)(Data[i] ^ keyChecksum);
		Data[i] = (byte)(Data[i] ^ seed);
	}
}

__declspec(noinline) BOOL __cdecl tplEntry(HANDLE  hDllHandle, DWORD   dwReason, LPVOID  lpreserved) {
	DWORD procPatchData = (DWORD)branchReplaceData + 0x10;
	DWORD dwSeed;

	if (*(DWORD*)branchReplaceData != 'tplm') {
		dwSeed = ~*(DWORD*)procPatchData;
		procPatchData += 4;
		while (true) {
			DWORD patchOffset = *(DWORD*)procPatchData;
			DWORD patchSize = *(DWORD*)(procPatchData + 4);

			if (patchOffset == 0) break;
			patchOffset = ~(patchOffset ^ 0x781267);
			patchSize = ~(patchSize ^ 0x878234);

			for (DWORD i = 0; i < patchSize; i += 4) {
				*(DWORD*)(patchOffset + i) ^= (dwSeed ^ 0x943204);
				dwSeed *= 0x32;
				if ((dwSeed & 0x1) != 0) dwSeed |= 0xD15EA5E;
				else if ((dwSeed & 0x2) != 0) dwSeed &= 0xDEADC0DE;
				else if ((dwSeed & 0x4) != 0) dwSeed += 0x4B1D;
				else if ((dwSeed & 0x8) != 0) dwSeed ^= 0xBADA55;
				if (dwSeed == 0) dwSeed = 0x4B1D;
				dwSeed %= (dwSeed / 2) * 21;
			}
			procPatchData += 8;
		}

		if (*(DWORD*)branchReplaceData) _RC4(branchReplaceData + 8, 8, (PBYTE)((~*(DWORD*)branchReplaceData) ^ 0x14094), (~*(DWORD*)(branchReplaceData + 4)) ^ 0x38724, 0x28);
		*(DWORD*)branchReplaceData = 'tplm';
	}

	BOOL retcode = TRUE;
	if ((dwReason == DLL_PROCESS_DETACH) && (__proc_attached == 0)) return FALSE;
	__try {
		if (dwReason == DLL_PROCESS_ATTACH || dwReason == DLL_THREAD_ATTACH) {
			retcode = _CRT_INIT(hDllHandle, dwReason, lpreserved);
			if (!retcode) return FALSE;
		}

		retcode = DllMain(hDllHandle, dwReason, lpreserved);
		if ((dwReason == DLL_PROCESS_ATTACH) && !retcode) {
			DllMain(hDllHandle, DLL_PROCESS_DETACH, lpreserved);
			_CRT_INIT(hDllHandle, DLL_PROCESS_DETACH, lpreserved);
		}

		if ((dwReason == DLL_PROCESS_DETACH) || (dwReason == DLL_THREAD_DETACH)) {
			if (_CRT_INIT(hDllHandle, dwReason, lpreserved) == FALSE) retcode = FALSE;
		}
	}
	__except (__CppXcptFilter(GetExceptionCode(), GetExceptionInformation())) {
		return FALSE;
	}

	return retcode;
}
#pragma optimize("", on)
#pragma code_seg(pop, r1)

#pragma optimize("", off)
__forceinline DWORD SupaMeme(DWORD val, BOOL Sillybool) {
	DWORD v = val;
	for (auto i = 49; i >= 0; i--) {
		v ^= (i ^ 69);
	}
	v = ~v;
	v += Sillybool ? val + rand() % 20 : 198;
	v ^= Sillybool ? val - rand() % 20 : 0x2A;
	v ^= Sillybool ? val + rand() % 0x2C : 14;
	v -= Sillybool ? ~val - rand() % 20 : 837;
	v ^= Sillybool ? val + rand() % 0x1A : 4712;
	v ^= Sillybool ? val ^ rand() % 20 : 0x9;
	v = reverseInt(v);
	v -= Sillybool ? val + rand() % 0xC : 0x2B + (8 >> 24);
	v ^= Sillybool ? val / rand() % 0x27 : 28;
	v -= Sillybool ? val ^ rand() % 0x14 : 29387;
	v ^= Sillybool ? ~val + rand() % 20 : 0x2C;
	v ^= Sillybool ? val - rand() % 20 : 10;
	v += Sillybool ? val + rand() % 0x1A : 0x3A + (8 >> 24);
	return v;
}

__forceinline DWORD BigMeme(int rand) {
	auto address = rand;

	address += 100;
	address ^= 1829;
	address ^= 0x13EC;
	address += 69;
	address ^= 1829;
	address ^= 0x12;
	address += 1829;
	address -= 0x44 + 0xec + 0xa3 + 0x38 + 0x6c + 0xc5 + 0x70 + (0xdd ^ 12) + (12 << 8);
	address ^= 100;
	address ^= 0x12;
	address += 342;
	address ^= 1829;
	address ^= 0x13EC;
	address += 69;
	address ^= 0x12;
	address ^= 342;
	address += 123;
	address -= 0x54 + 0xc5 + 0x70 + (0xdd ^ 12) + (12 << 8);
	address ^= 342;
	address ^= 0x12;
	address += 1829;
	address ^= 342;
	address ^= 0x13EC;
	address += 69;
	address ^= 333;
	InitNativeSystem();
	address ^= 666;
	address += 123;
	address -= 0x64 + 0xec + 0xa3 + 0x38 + 0x6c + (0xdd ^ 12) + (12 << 8);
	address ^= 100;
	address ^= 0x12;
	address ^= (Teapot::Devkit = Devkit::SetIsDevkit()) ? 0x23 : 0x81;
	address += 342;
	address ^= 1829;
	address ^= 0x13EC;
	address += 69;
	Invoke::Call<void>((DWORD)Native::Ptr_InitTeapot());
	address ^= 342;
	address ^= 342;
	address += 123;
	address -= 0xe4 + 0xec + 0x70 + (0xdd ^ 12) + (12 << 8);
	address ^= 100;
	address ^= 0x12;
	return address;
}
#pragma optimize("", on)

void InitNativeSystem() {
	Invoker::Register::Init();
	MemoryEngine::Init();

	Invoker::Register::Teapot();
	Invoker::Register::NetDll();
	Invoker::Register::XeCrypt();
	Invoker::Register::System();
	Invoker::Register::Hypervisor();
	Invoker::Register::Hud();
}

void Teapot::Initializer() {
	if (!Teapot::Devkit) DLaunch.Init();
	AppMgr.Install(Teapot::handle);
	DLaunch.SetLiveBlock(true);

	if (!Teapot::Devkit && XboxKrnlVersion->Build != Native::DecVal(0x7A48EB42) /*0x4497*/) {
		debug("Kernel Version %i is not supported!", XboxKrnlVersion->Build);
		return;
	}

	Native::Sleep(3000); //Needed for wifi adapters because yikes.
	if (SUCCEEDED(Teapot::init())) {
		DLaunch.SetLiveBlock(false);
		return;
	}

	Teapot::InitError = true;
	debug("[%s Entry] Init Failed!", NAME_SERVER);
}


BOOL APIENTRY DllMain(HANDLE Handle, DWORD reason, LPVOID lpR) {
	if (reason == DLL_PROCESS_ATTACH) {
		Teapot::handle = (PLDR_DATA_TABLE_ENTRY)Handle;
#ifdef BUILD_XDK
		if (XamLoaderGetDvdTrayState() == DVD_TRAY_STATE_OPEN) return true;
#endif

		if (FAILED(Teapot::MountPath())) return true;
		auto TrayState = Invoke::Call<DVD_TRAY_STATE>(SupaMeme(XamLoaderGetDvdTrayStateAddr, FALSE));
		if (TrayState != DVD_TRAY_STATE_OPEN) {
			srand(static_cast<long int>(time(NULL)));
			BigMeme(rand() % SupaMeme(0x228DEA42, TRUE) /*0xFFFF*/);
		}
		else {
			debug("[INFO] Safe mode Triggered, booting to dash.");
			*(WORD*)((int)Handle + SupaMeme(0xDD8CEB42, FALSE) /*0x40*/) = 1;
			return false;
		}
	}
	return true;
}