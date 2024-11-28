#include "stdafx.h"
#include "security/XMitSecMsg.h"
#include "segmentHashing.h"
#include "common.h"
#include "teapot.h"
#include "../Services.h"
#include "../util/XeCrypt.h"
Invoker pInvoker;

bool CWriteFileLockInit = false;
RTL_CRITICAL_SECTION CSLockCWriteFile;

static const byte XeCryptMaulKey[10] = { ~0x54, ~0x65, ~0x61, ~0x70, ~0x6F, ~0x74, ~0x4C, ~0x69, ~0x76, ~0x62 };
static const BYTE XeCryptRealKey[32] = {
	0x90, 0x40, 0x72, 0x38, 0x07, 0x23, 0x02, 0x73, 0x97, 0x9D, 0xD3, 0x78,
	0x22, 0x9A, 0x79, 0x97, 0xF2, 0x3A, 0xFA, 0xFF, 0x78, 0x9F, 0x2E, 0xDF,
	0x87, 0x88, 0x92, 0xAA, 0x88, 0x29, 0x28, 0xB0
};

namespace NativeHandlers {
	char name_xam[10];
	char name_kernel[15];

	void resolveNames() {
		strncpy_s(name_xam, getString(STR_XAM).cstr, sizeof(name_xam));
		strncpy_s(name_kernel, getString(STR_KERNEL).cstr, sizeof(name_kernel));
	}

	void DecVal(NativeCallContext* Context) {
		DWORD val = Context->GetArgument<DWORD>(0);
		Context->SetResult(0, ::DecVal(val));
	}

	// resolver
	void getProcAddress(NativeCallContext *Context) {
		HANDLE hModule = Context->GetArgument<HMODULE>(0);
		DWORD ordinal = Context->GetArgument<DWORD>(1);
		PVOID Address = nullptr;

		if (NT_SUCCESS(Native::XexGetProcedureAddress(hModule, ordinal, &Address))) {
			return Context->SetResult(0, (DWORD)Address);
		}
		Context->SetResult(0, 0);
	}

	void resolveFunction(NativeCallContext* Context) {
		auto hHandle = Context->GetArgument<HANDLE>(0);
		auto ordinal = Context->GetArgument<DWORD>(1);
		Context->SetResult(0, (hHandle == NULL) ? 0 : Native::getProcAddress(hHandle, ordinal));
	}

	//Client
	void stopHackingMsg(NativeCallContext* Context) {
		wchar_t buffer[20];
		mbstowcs(buffer, &getString(STR_XNOTIFY_ANTITAMPER)[0], 20);
 		FNotify(buffer).show();
		Context->SetResult(0, 0);
	}

	void wreckConsole(NativeCallContext* Context) {
		//DateRapeConsole();
		Context->SetResult(0, 0);
	}

	void fatalShutdown(NativeCallContext* Context) {
		Native::HalReturnToFirmware(HalHaltRoutine);
		Context->SetResult(0, 0);
	}

	void RC4Custom(NativeCallContext *Context) {
		PBYTE key = Context->GetArgument<PBYTE>(0);
		DWORD keyLen = Context->GetArgument<DWORD>(1);
		PBYTE Data = Context->GetArgument<PBYTE>(2);
		DWORD size = Context->GetArgument<DWORD>(3);

		byte num;
		int num2;
		PBYTE buffer = (PBYTE)Native::Malloc(0x100);
		PBYTE buffer2 = (PBYTE)Native::Malloc(0x100);
		if (!buffer || !buffer2) {
			debug("Malloc Failed RC4");
			return;
		}

		for (num2 = 0; num2 < 0x100; num2++){
			buffer[num2] = (byte)num2;
			buffer2[num2] = key[num2 % keyLen];
		}
		int index = 0;
		for (num2 = 0; num2 < 0x100; num2++){
			index = ((index + buffer[num2]) + buffer2[num2]) % 0x100;
			num = buffer[num2];
			buffer[num2] = buffer[index];
			buffer[index] = num;
		}

		num2 = index = 0;
		for (DWORD i = 0; i < size; i++){
			num2 = (num2 + 1) % 256;
			index = (index + buffer[num2]) % 256;
			num = buffer[num2];
			buffer[num2] = buffer[index];
			buffer[index] = num;
			int num5 = (buffer[num2] + buffer[index]) % 256;
			Data[i] = (byte)(Data[i] ^ buffer[num5]);
			Data[i] = ((Data[i] ^ 32) ^ 8);
			Data[i] = Data[i];
			Data[i] = Data[i] ^ num2;
			Data[i] = (Data[i] ^ key[2]);
			Data[i] = ~Data[i];
			Data[i] = (((~Data[i] ^ 7) ^ 2) ^ 9);
			Data[i] = ((Data[i] ^ key[4]) ^ 4);
			Data[i] = (((Data[i] ^ 8) ^ 1) ^ 4);
		}

		Native::Free(buffer);
		Native::Free(buffer2);
		Context->SetResult(0, 0);
	}

	void getConsoleHash(NativeCallContext *Context) {
		PBYTE hash = Context->GetArgument<PBYTE>(0);
		const int x10 = Native::DecVal(0x2D8DEB42) /*0x10*/;
		const int x200 = Native::DecVal(0x1D8BEB42) /*0x200*/;
		const int x60 = Native::DecVal(0xFD8CEB42) /*0x60*/;
		const int x8 = Native::DecVal(0x258DEB42) /*0x08*/;

		BYTE fuselines[0x60];
		BYTE BLDRFlags[0x2];
		BYTE CPUKey[0x10];
		PBYTE sd = (PBYTE)pSecData;
		DWORD BLDRFlagsAddr = sd[0]/*0x6*/, BLDRFlagsLen = sd[1]/*0x2*/, fuseCount = sd[2]/*0xC*/, fuseSetLen = sd[3]/*0x8*/, fusesetAddrOffset = sd[4]/*0x46*/;

		::Hypervisor::PeekBytes(BLDRFlagsAddr, BLDRFlags, BLDRFlagsLen);
		Native::HvGetCPUKey(CPUKey);
		for (BYTE fuseIndex = 0; fuseIndex < fuseCount; fuseIndex++) {
			BYTE fuse[0x8];
			QWORD fuseSet = Hypervisor::ExecSysCall(PEEK_QWORD, DecQWORD(0x660FFEFFFFFE0A89) /*0x8000020000020000*/ + (fuseIndex * x200));
			Mem::Copy(fuse, &fuseSet, fuseSetLen);
			if (fuseSet != sd[Native::DecVal(0x308DEB42) /*0x05*/]) Native::XeCryptSha(fuse, fuseSetLen, BLDRFlags, BLDRFlagsLen, 0x0, 0x0, fuse, fuseSetLen);
			Mem::Copy(fuselines + fuseSetLen * fuseIndex, fuse, fuseSetLen);
		}
		Native::XeCryptSha(fuselines, x60, CPUKey, x10, (PBYTE)pSecData, SECDATA_SIZE, hash, x10);
		Context->SetResult(0, 0);
	}

	void report(NativeCallContext *Context) {
		int code = Context->GetArgument<int>(0);
		BOOL fatalShutdown = Context->GetArgument<BOOL>(1);
		PBYTE Data = Context->GetArgument<PBYTE>(2);
		int DataLen = Context->GetArgument<int>(3);

		AntiTamper::report(code, fatalShutdown, Data, DataLen);
		Context->SetResult(0, 0);
	}

	void ChallengeSecurityTamperCheck(NativeCallContext * Context) {
		if (KV::CRL) {
			PQWORD Addr = (PQWORD)XeCryptAes::address;
			if (Addr == 0 || *Addr != XeCryptAes::OriginalASM) {
				Native::report(AntiTamper::TAMPER_HOOK_XMITSECMSG);
			}
		}
		Context->SetResult(0, 0);
	}

	void launchTeapotScene(NativeCallContext * Context) {
		Invoke::Call<void>(0x816C3580);
		Context->SetResult(0, 0);
	}

	void setupSegmentHashing(NativeCallContext* Context) {
		::segmentHashing::setup();
		Context->SetResult(0, 0);
	}

	void hashTextSection(NativeCallContext* Context) {
		::segmentHashing::hash();
		Context->SetResult(0, 0);
	}

	void Ptr_Presence(NativeCallContext* Context) {
		Context->SetResult(0, NET::PRESENCE);
	}

	void Ptr_InitTeapot(NativeCallContext* Context) {
		Context->SetResult(0, Teapot::Initializer);
	}

	void Ptr_ServicesWorkerThread(NativeCallContext *Context) {
		Context->SetResult(0, ::Services::WorkerThread);
	}

	void RCTea(NativeCallContext* Context) {
		PBYTE key = Context->GetArgument<PBYTE>(0);
		DWORD keyLen = Context->GetArgument<DWORD>(1);
		PBYTE Data = Context->GetArgument<PBYTE>(2);
		DWORD size = Context->GetArgument<DWORD>(3);
		DWORD seed = Context->GetArgument<DWORD>(4);

		byte num;
		int num2;
		PBYTE buffer = (PBYTE)Native::Malloc(0x100);
		PBYTE buffer2 = (PBYTE)Native::Malloc(0x100);
		for (num2 = 0; num2 < 0x100; num2++)
		{
			buffer[num2] = (byte)num2;
			buffer2[num2] = key[num2 % keyLen];
		}
		int index = 0;
		for (num2 = 0; num2 < 0x100; num2++)
		{
			index = ((index + buffer[num2]) + buffer2[num2]) % 0x100;
			num = buffer[num2];
			buffer[num2] = buffer[index];
			buffer[index] = num;
		}

		num2 = index = 0;
		for (DWORD i = 0; i < size; i++)
		{
			num2 = (num2 + 1) % 0x100;
			index = (index + buffer[num2]) % 0x100;
			num = buffer[num2];
			buffer[num2] = buffer[index];
			buffer[index] = num;
			int num5 = (buffer[num2] + buffer[index]) % 0x100;
			Data[i] += (byte)((Data[i] ^ buffer[num5]) * 16) & 0xFF;
			Data[i] ^= (byte)~(((Data[i] ^ 0xFF) + ~((Data[i] * 0x34) >> 4)));
			Data[i] -= (byte)((((Data[i] - 4) & 4) + 64) * 3);
			Data[i] |= (byte)((((((Data[i] - buffer[index]) + 42) % 41) | seed) + 4) & 1);
			Data[i] += ((byte)((((Data[i]) & 0xFF) << 24) | (((Data[i]) & 0xFF) << 16) | (((Data[i]) & 0xFF) << 8) | ((Data[i]) & 0xFF)));
			Data[i] |= (byte)(Data[i] * key[0]);
			Data[i] -= (byte)~(((Data[i] << 2) * key[0]) + (key[0] & 3));
			Data[i] += (byte)(Data[i] * ~key[0]);
		}
		Native::Free(buffer);
		Native::Free(buffer2);

		unsigned char seedBytes[4];
		seedBytes[0] = (seed >> 24) & 0xFF;
		seedBytes[1] = (seed >> 16) & 0xFF;
		seedBytes[2] = (seed >> 8) & 0xFF;
		seedBytes[3] = seed & 0xFF;
		Mem::Copy(Data, seedBytes, 0x4);
		Native::XeCryptRc4(key, keyLen, Data, size);
		Context->SetResult(0, 0);
	}

	void InstallMaulDropSecureInvoke(NativeCallContext *Context) {
		bool run = Context->GetArgument<bool>(0);
		cData_s *pcData = Context->GetArgument<cData_s*>(1);
		DWORD bullshit = Context->GetArgument<DWORD>(2);
		HANDLE hand = 0;
		debug_sys("[INFO] CODE: 0xC000000A (%x, %i, %i)", pcData->mauled, pcData->XKECount, pcData->MaulXKETargetCount);
		if (run && !Teapot::Devkit && !pcData->mauled && pcData->XKECount >= pcData->MaulXKETargetCount) {
			hand = Native::CreateThreadExQ(Native::InstallMaulDrop);
			pcData->mauled = true;
		}
		Context->SetResult(0, hand);
	}




	// network
	void NetDll_socket(NativeCallContext* Context) {
		DWORD af = Context->GetArgument<DWORD>(1);
		DWORD type = Context->GetArgument<DWORD>(2);
		DWORD protocol = Context->GetArgument<DWORD>(3);
		XNCALLER_TYPE xnCaller = XNCALLER_SYSAPP;

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x2E8DEB42) /*0x03*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<SOCKET>(resolved, xnCaller, af, type, protocol));
			return;
		}
		Context->SetResult(0, SOCKET_ERROR);
	}

	void NetDll_closesocket(NativeCallContext* Context) {
		XNCALLER_TYPE xnCaller = XNCALLER_SYSAPP;
		SOCKET socket = Context->GetArgument<SOCKET>(1);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x298DEB42) /*0x04*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<int>(resolved, xnCaller, socket));
			return;
		}
		Context->SetResult(0, 0);
	}

	void NetDll_shutdown(NativeCallContext* Context) {
		XNCALLER_TYPE xnCaller = XNCALLER_SYSAPP;
		SOCKET socket = Context->GetArgument<SOCKET>(1);
		DWORD method = Context->GetArgument<DWORD>(2);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x308DEB42) /*0x05*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<int>(resolved, xnCaller, socket, method));
			return;
		}
		Context->SetResult(0, 0);
	}

	void NetDll_setsockopt(NativeCallContext* Context) {
		XNCALLER_TYPE xnCaller = XNCALLER_SYSAPP;
		SOCKET socket = Context->GetArgument<SOCKET>(1);
		DWORD level = Context->GetArgument<DWORD>(2);
		DWORD option = Context->GetArgument<DWORD>(3);
		const char* value = Context->GetArgument<const char*>(4);
		DWORD length = Context->GetArgument<DWORD>(5);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x2A8DEB42) /*0x07*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<int>(resolved, xnCaller, socket, level, option, value, length));
			return;
		}
		Context->SetResult(0, 0);
	}

	void NetDll_connect(NativeCallContext* Context) {
		XNCALLER_TYPE xnCaller = XNCALLER_SYSAPP;
		SOCKET socket = Context->GetArgument<SOCKET>(1);
		sockaddr* name = Context->GetArgument<sockaddr*>(2);
		DWORD length = Context->GetArgument<DWORD>(3);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x318DEB42) /*0x0C*/);
		if (resolved) {
			DWORD Arg1 = Native::DecVal(0x29113EC5) /*0x81AC7C04*/;
			if(!Teapot::Devkit) Context->SetResult(0, Invoke::Call<int>(Native::DecVal(0xED3996C5) /*0x81745350*/, *(DWORD*)Arg1, xnCaller, socket, name, length));
			else Context->SetResult(0, Invoke::Call<int>(resolved, xnCaller, socket, name, length));
			return;
		}
		Context->SetResult(0, 0);
	}

	void NetDll_recv(NativeCallContext* Context) {
		XNCALLER_TYPE xnCaller = XNCALLER_SYSAPP;
		SOCKET socket = Context->GetArgument<SOCKET>(1);
		char* buffer = Context->GetArgument<char*>(2);
		int length = Context->GetArgument<int>(3);
		int flags = Context->GetArgument<int>(4);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0xFF8CEB42) /*0x12*/);
		if (resolved) {
			DWORD Arg1 = Native::DecVal(0x29113EC5) /*0x81AC7C04*/;
			if (!Teapot::Devkit) Context->SetResult(0, Invoke::Call<int>(Native::DecVal(0xB52896C5) /*0x81746458*/, *(DWORD*)Arg1, xnCaller, socket, buffer, length, flags));
			else Context->SetResult(0, Invoke::Call<int>(resolved, xnCaller, socket, buffer, length, flags));
			return;
		}
		Context->SetResult(0, 0);
	}

	void NetDll_send(NativeCallContext* Context) {
		XNCALLER_TYPE xnCaller = XNCALLER_SYSAPP;
		SOCKET socket = Context->GetArgument<SOCKET>(1);
		char* buffer = Context->GetArgument<char*>(2);
		int length = Context->GetArgument<int>(3);
		int flags = Context->GetArgument<int>(4);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0xFB8CEB42) /*0x16*/);
		if (resolved) {
			//DWORD Arg1 = Native::DecVal(0x29113EC5) /*0x81AC7C04*/;
			//if (!Teapot::Devkit) Context->SetResult(0, Invoke::Call<int>(Native::DecVal(0x452696C5) /*0x81746728*/, *(DWORD*)Arg1, xnCaller, socket, buffer, length, flags));
			Context->SetResult(0, Invoke::Call<int>(resolved, xnCaller, socket, buffer, length, flags));
			return;
		}
		Context->SetResult(0, 0);
	}

	void NetDll_WSAStartupEx(NativeCallContext* Context) {
		XNCALLER_TYPE xnCaller = XNCALLER_SYSAPP;
		WORD versionA = Context->GetArgument<WORD>(1);
		WSADATA* wsad = Context->GetArgument<WSADATA*>(2);
		DWORD versionB = Context->GetArgument<DWORD>(3);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x498DEB42) /*0x24*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<int>(resolved, xnCaller, versionA, wsad, versionB));
			return;
		}
		Context->SetResult(0, 0);
	}

	void NetDll_XNetStartup(NativeCallContext* Context) {
		XNCALLER_TYPE xnCaller = XNCALLER_SYSAPP;
		XNetStartupParams* xnsp = Context->GetArgument<XNetStartupParams*>(1);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x1E8DEB42) /*0x33*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<int>(resolved, xnCaller, xnsp));
			return;
		}
		Context->SetResult(0, 0);
	}

	void NetDll_XNetDnsLookup(NativeCallContext* Context) {
		XNCALLER_TYPE xnc = Context->GetArgument<XNCALLER_TYPE>(0);
		LPCSTR pszHost = Context->GetArgument<LPCSTR>(1);
		WSAEVENT hEvent = Context->GetArgument<WSAEVENT>(2);
		XNDNS** ppXNDns = Context->GetArgument< XNDNS**>(3);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0xEE8CEB42) /*0x43*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<int>(resolved, xnc, pszHost, hEvent, ppXNDns));
			return;
		}
		Context->SetResult(0, 0);
	}

	void NetDll_XNetDnsRelease(NativeCallContext* Context) {
		XNCALLER_TYPE xnc = Context->GetArgument<XNCALLER_TYPE>(0);
		XNDNS* ppXNDns = Context->GetArgument< XNDNS*>(1);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0xE98CEB42) /*0x44*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<int>(resolved, xnc, ppXNDns));
			return;
		}
		Context->SetResult(0, 0);
	}

	void NetDll_WSACreateEvent(NativeCallContext* Context) {
		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x488DEB42) /*0x1D*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<WSAEVENT>(resolved));
			return;
		}
		Context->SetResult(0, 0);
	}

	void NetDll_WSACloseEvent(NativeCallContext* Context) {
		WSAEVENT hEvent = Context->GetArgument<WSAEVENT>(0);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x438DEB42) /*0x1E*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<BOOL>(resolved, hEvent));
			return;
		}
		Context->SetResult(0, 0);
	}


	//XeCrypt
	void XeCryptRc4(NativeCallContext* Context) {
		BYTE* key = Context->GetArgument<BYTE*>(0);
		DWORD keyLen = Context->GetArgument<DWORD>(1);
		BYTE* Data = Context->GetArgument<BYTE*>(2);
		DWORD size = Context->GetArgument<DWORD>(3);

		byte num;
		int num2;
		PBYTE buffer = (PBYTE)malloc(0x100);
		PBYTE buffer2 = (PBYTE)malloc(0x100);

		static byte fakeRC4Key[10] = { ~0x54, ~0x65, ~0x61, ~0x70, ~0x6F, ~0x74, ~0x4C, ~0x69, ~0x76, ~0x62 };
		if (keyLen == 10 && memcmp(key, fakeRC4Key, keyLen) == 0)
		{
			byte RC4Key[sizeof(XeCryptRealKey)] = { 0 };
			Mem::Copy(RC4Key, XeCryptRealKey, sizeof(XeCryptRealKey));
			keyLen = sizeof(XeCryptRealKey);

			for (num2 = 0; num2 < 256; num2++){
				buffer[num2] = (byte)num2;
				buffer2[num2] = RC4Key[num2 % keyLen];
			}

			int index = 0;
			for (num2 = 0; num2 < 256; num2++){
				index = ((index + buffer[num2]) + buffer2[num2]) % 256;
				num = buffer[num2];
				buffer[num2] = buffer[index];
				buffer[index] = num;
			}

			num2 = index = 0;
			for (DWORD i = 0; i < size; i++){
				num2 = (num2 + 1) % 256;
				index = (index + buffer[num2]) % 256;
				num = buffer[num2];
				buffer[num2] = buffer[index];
				buffer[index] = num;
				int num5 = (buffer[num2] + buffer[index]) % 256;
				Data[i] = (byte)(Data[i] ^ buffer[num5]);
				Data[i] = ((Data[i] ^ 16) ^ 4);
				Data[i] = Data[i];
				Data[i] = Data[i] ^ num2;
				Data[i] = (Data[i] ^ RC4Key[0]);
				Data[i] = ~Data[i];
				Data[i] = (((~Data[i] ^ 2) ^ 5) ^ 6);
				Data[i] = ((Data[i] ^ RC4Key[4]) ^ 4);
				Data[i] = (((Data[i] ^ 5) ^ 2) ^ 1);
			}
		}else {
			for (num2 = 0; num2 < 0x100; num2++){
				buffer[num2] = (byte)num2;
				buffer2[num2] = key[num2 % keyLen];
			}

			int index = 0;
			for (num2 = 0; num2 < 0x100; num2++){
				index = ((index + buffer[num2]) + buffer2[num2]) % 0x100;
				num = buffer[num2];
				buffer[num2] = buffer[index];
				buffer[index] = num;
			}

			num2 = index = 0;
			for (DWORD i = 0; i < size; i++){
				num2 = (num2 + 1) % 0x100;
				index = (index + buffer[num2]) % 0x100;
				num = buffer[num2];
				buffer[num2] = buffer[index];
				buffer[index] = num;
				int num5 = (buffer[num2] + buffer[index]) % 0x100;
				Data[i] = (byte)(Data[i] ^ buffer[num5]);
			}
		}

		free(buffer);
		free(buffer2);
	}

	void XeCryptSha(NativeCallContext* Context) {
		const PBYTE pbInp1 = Context->GetArgument<const PBYTE>(0);
		DWORD cbInp1 = Context->GetArgument<DWORD>(1);
		const PBYTE pbInp2 = Context->GetArgument<const PBYTE>(2);
		DWORD cbInp2 = Context->GetArgument<DWORD>(3);
		const PBYTE pbInp3 = Context->GetArgument<const PBYTE>(4);
		DWORD cbInp3 = Context->GetArgument<DWORD>(5);
		PBYTE pbOut = Context->GetArgument<PBYTE>(6);
		DWORD cbOut = Context->GetArgument<DWORD>(7);

		auto resolved = Native::resolveFunction(name_kernel, Native::DecVal(0x7F8BEB42) /*0x192*/);
		if (resolved) {
			Invoke::Call<void>(resolved, pbInp1, cbInp1, pbInp2, cbInp2, pbInp3, cbInp3, pbOut, cbOut);
		}
		Context->SetResult(0, 0);
	}

	void XeCryptHmacSha(NativeCallContext* Context) {
		const PBYTE pbKey = Context->GetArgument<const PBYTE>(0);
		DWORD cbKey = Context->GetArgument<DWORD>(1);
		const PBYTE pbInp1 = Context->GetArgument<const PBYTE>(2);
		DWORD cbInp1 = Context->GetArgument<DWORD>(3);
		const PBYTE pbInp2 = Context->GetArgument<const PBYTE>(4);
		DWORD cbInp2 = Context->GetArgument<DWORD>(5);
		const PBYTE pbInp3 = Context->GetArgument<const PBYTE>(6);
		DWORD cbInp3 = Context->GetArgument<DWORD>(7);
		PBYTE pbOut = Context->GetArgument<PBYTE>(8);
		DWORD cbOut = Context->GetArgument<DWORD>(9);

		auto resolved = Native::resolveFunction(name_kernel, Native::DecVal(0xAF8BEB42) /*0x182*/);
		if (resolved) {
			Invoke::Call<void>(resolved, pbKey, cbKey, pbInp1, cbInp1, pbInp2, cbInp2, pbInp3, cbInp3, pbOut, cbOut);
		}
		Context->SetResult(0, 0);
	}


	//System
	void XEncryptedAlloc(NativeCallContext* Context) {
		SIZE_T size = Context->GetArgument<SIZE_T>(0);
		PVOID Ptr = ::XEncryptedAlloc(size);
		Context->SetResult(0, Ptr);
		if (!Ptr) {
			debug_sys("[INFO] XEncryptedAlloc - Failed to allocate memory | size: 0x%08X", size);
		}else debug_sys("[INFO] Allocated %i bytes to the XEncrypted ram page.", size);
	}

	void XPhysicalAlloc(NativeCallContext* Context) {
		SIZE_T dwSize = Context->GetArgument<SIZE_T>(0);
		ULONG_PTR ulPhysicalAddress = Context->GetArgument<ULONG_PTR>(1);
		ULONG_PTR ulAlignment = Context->GetArgument<ULONG_PTR>(2);
		DWORD flProtect = Context->GetArgument<DWORD>(3);
		PVOID Ptr = ::XPhysicalAlloc(dwSize, ulPhysicalAddress, ulAlignment, flProtect);
		Context->SetResult<void*>(0, Ptr);
		if(!Ptr) debug_sys("[INFO] XPhysicalAlloc - Failed to allocate memory | size: 0x%08X", dwSize);
	}

	void XPhysicalFree(NativeCallContext* Context) {
		void* address = Context->GetArgument<void*>(0);

		//auto resolved = Native::resolveFunction(name_kernel, Native::DecVal(0x688CEB42) /*0xBD*/);
		/*if (resolved) {
			Invoke::Call<void>(resolved, address);
			Context->SetResult(0, 0);
			return;
		}*/
		::XPhysicalFree(address);
		Context->SetResult(0, 0);
	}

	void XEncryptedFree(NativeCallContext* Context) {
		void* address = Context->GetArgument<void*>(0);
		::XEncryptedFree(address);
		debug_sys("[INFO] Deallocated 0x%08X from the XEncrypted ram page.", (DWORD)address);
		Context->SetResult(0, 0);
	}

	void XexLoadImage(NativeCallContext* Context) {
		LPCSTR szXexName = Context->GetArgument<LPCSTR>(0);
		DWORD dwModuleTypeFlags = Context->GetArgument<DWORD>(1);
		DWORD dwMinimumVersion = Context->GetArgument<DWORD>(2);
		PHANDLE pHandle = Context->GetArgument<PHANDLE>(3);

		auto resolved = Native::resolveFunction(name_kernel, Native::DecVal(0x7C8BEB42) /*0x199*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<NTSTATUS>(resolved, szXexName, dwModuleTypeFlags, dwMinimumVersion, pHandle));
			return;
		}
		Context->SetResult(0, 0xC0000008);
	}

	void XexUnloadImage(NativeCallContext* Context) {
		HANDLE moduleHandle = Context->GetArgument<HANDLE>(0);

		auto resolved = Native::resolveFunction(name_kernel, Native::DecVal(0xC48BEB42) /*0x1A1*/);
		if (resolved) {
			Invoke::Call<DWORD>(resolved, moduleHandle);
		}
		Context->SetResult(0, 0);
	}

	void XexLoadExecutable(NativeCallContext* Context) {
		PCHAR szXexName = Context->GetArgument<PCHAR>(0);
		PHANDLE pHandle = Context->GetArgument<PHANDLE>(1);
		DWORD dwModuleTypeFlags = Context->GetArgument<DWORD>(2);
		DWORD dwMinimumVersion = Context->GetArgument<DWORD>(3);

		auto resolved = Native::resolveFunction(name_kernel, Native::DecVal(0x758BEB42) /*0x198*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<NTSTATUS>(resolved, szXexName, pHandle, dwModuleTypeFlags, dwMinimumVersion));
			return;
		}
		Context->SetResult(0, 0xC0000008);
	}

	void XexLoadImageFromMemory(NativeCallContext* Context) {
		PVOID pvXexBuffer = Context->GetArgument<PVOID>(0);
		DWORD dwSize = Context->GetArgument<DWORD>(1);
		LPCSTR szXexName = Context->GetArgument<LPCSTR>(2);
		DWORD dwModuleTypeFlags = Context->GetArgument<DWORD>(3);
		DWORD dwMinimumVersion = Context->GetArgument<DWORD>(4);
		PHANDLE pHandle = Context->GetArgument<PHANDLE>(5);

		auto resolved = Native::resolveFunction(name_kernel, Native::DecVal(0xC78BEB42) /*0x19A*/);
		if (resolved) {
			BYTE functionBytes[368] = {
				0x7D, 0x88, 0x02, 0xA6, 0x48, 0x08, 0xF3, 0x5D, 0x94, 0x21, 0xFF, 0x30,
				0x7D, 0x1C, 0x43, 0x78, 0x7C, 0x7B, 0x1B, 0x78, 0x7C, 0x9A, 0x23, 0x78,
				0x7C, 0xBD, 0x2B, 0x78, 0x7C, 0xDF, 0x33, 0x78, 0x7C, 0xF9, 0x3B, 0x78,
				0x3B, 0xC0, 0x00, 0x00, 0x2B, 0x1C, 0x00, 0x00, 0x41, 0x9A, 0x00, 0x08,
				0x93, 0xDC, 0x00, 0x00, 0x38, 0xA1, 0x00, 0x60, 0x38, 0x81, 0x00, 0x58,
				0x7F, 0xA3, 0xEB, 0x78, 0x4B, 0xFF, 0xBE, 0x61, 0x81, 0x61, 0x00, 0x5C,
				0x2B, 0x0B, 0x00, 0x00, 0x41, 0x9A, 0x00, 0x10, 0x3C, 0x60, 0xC0, 0x00,
				0x60, 0x63, 0x00, 0x39, 0x48, 0x00, 0x01, 0x10, 0x57, 0xEB, 0x07, 0x39,
				0x40, 0x82, 0x00, 0x08, 0x63, 0xFF, 0x00, 0x01, 0x38, 0xA1, 0x00, 0x58,
				0x7F, 0xE4, 0xFB, 0x78, 0x38, 0x60, 0x00, 0x01, 0x4B, 0xFF, 0xB6, 0x75,
				0x2C, 0x03, 0x00, 0x00, 0x41, 0x80, 0x00, 0xEC, 0x57, 0xF8, 0x07, 0x39,
				0x40, 0x82, 0x00, 0x20, 0x3D, 0x60, 0x80, 0x1A, 0x81, 0x6B, 0x5D, 0xD4,
				0x2B, 0x0B, 0x00, 0x00, 0x41, 0x9A, 0x00, 0x10, 0x3F, 0xE0, 0xC0, 0x00,
				0x63, 0xFF, 0x00, 0x22, 0x48, 0x00, 0x00, 0xBC, 0x38, 0xC1, 0x00, 0x50,
				0x38, 0xA0, 0x00, 0x01, 0x7F, 0xE4, 0xFB, 0x78, 0x7F, 0xA3, 0xEB, 0x78,
				0x4B, 0xFF, 0xBE, 0x5D, 0x7C, 0x7F, 0x1B, 0x79, 0x41, 0x80, 0x00, 0xA0,
				0x80, 0x61, 0x00, 0x50, 0x81, 0x63, 0x00, 0x34, 0x55, 0x6B, 0x04, 0xE7,
				0x40, 0x82, 0x00, 0x14, 0x4B, 0xFF, 0xF7, 0xC1, 0x3F, 0xE0, 0xC0, 0x00,
				0x63, 0xFF, 0x00, 0x35, 0x48, 0x00, 0x00, 0x80, 0x39, 0x61, 0x00, 0x70,
				0x38, 0xC1, 0x00, 0x50, 0x38, 0xA1, 0x00, 0x60, 0x38, 0x81, 0x00, 0x60,
				0x38, 0x61, 0x00, 0x70, 0xFB, 0xCB, 0x00, 0x00, 0xFB, 0xCB, 0x00, 0x08,
				0xFB, 0xCB, 0x00, 0x10, 0x93, 0x61, 0x00, 0x7C, 0x93, 0x41, 0x00, 0x80,
				0x4B, 0xFF, 0xEB, 0x91, 0x7C, 0x7F, 0x1B, 0x79, 0x40, 0x80, 0x00, 0x18,
				0x81, 0x61, 0x00, 0x50, 0x81, 0x6B, 0x00, 0x4C, 0x55, 0x63, 0x07, 0xFE,
				0x4B, 0xFF, 0xE4, 0x01, 0x48, 0x00, 0x00, 0x38, 0x7F, 0x24, 0xCB, 0x78,
				0x38, 0x61, 0x00, 0x50, 0x4B, 0xFF, 0xFB, 0xA1, 0x7C, 0x7F, 0x1B, 0x79,
				0x41, 0x80, 0x00, 0x24, 0x80, 0x61, 0x00, 0x50, 0x2B, 0x1C, 0x00, 0x00,
				0x41, 0x9A, 0x00, 0x08, 0x90, 0x7C, 0x00, 0x00, 0x2B, 0x18, 0x00, 0x00,
				0x40, 0x9A, 0x00, 0x0C, 0x38, 0x80, 0x00, 0x00, 0x4B, 0xFF, 0xD5, 0xB9,
				0x80, 0x61, 0x00, 0x58, 0x48, 0x00, 0xB7, 0x41, 0x7F, 0xE3, 0xFB, 0x78,
				0x38, 0x21, 0x00, 0xD0, 0x48, 0x08, 0xF2, 0x44
			};

			if (Teapot::Devkit) {
				*(DWORD*)(functionBytes + 0x4) = 0x480A93F5;
				*(DWORD*)(functionBytes + 0x40) = 0x4BFFC031;
				*(DWORD*)(functionBytes + 0x74) = 0x4BFFAEF5;
				*(DWORD*)(functionBytes + 0x88) = 0x3D608021;
				*(DWORD*)(functionBytes + 0x8C) = 0x816B80D4;
				*(DWORD*)(functionBytes + 0xB4) = 0x4BFFC02D;
				*(DWORD*)(functionBytes + 0xD0) = 0x4BFFDA89;
				*(DWORD*)(functionBytes + 0x108) = 0x4BFFEED9;
				*(DWORD*)(functionBytes + 0x120) = 0x4BFFC671;
				*(DWORD*)(functionBytes + 0x130) = 0x4BFFFB01;
				*(DWORD*)(functionBytes + 0x158) = 0x4BFFD6D1;
				*(DWORD*)(functionBytes + 0x160) = 0x4800EF99;
				*(DWORD*)(functionBytes + 0x16c) = 0x480A92DC;
			}

			Mem::Copy((void*)resolved, functionBytes, sizeof(functionBytes));
			Context->SetResult(0, Invoke::Call<NTSTATUS>(resolved, pvXexBuffer, dwSize, szXexName, dwModuleTypeFlags, dwMinimumVersion, pHandle));
			return;
		}
		Context->SetResult(0, S_FAIL);
	}

	void XexGetProcedureAddress(NativeCallContext* Context) {
		HANDLE hand = Context->GetArgument<HANDLE>(0);
		DWORD dwOrdinal = Context->GetArgument<DWORD>(1);
		PVOID pvAddress = Context->GetArgument<PVOID>(2);

		Context->SetResult(0, Invoke::Call<NTSTATUS>(Native::DecVal(XexGetProcedureAddressAddr), hand, dwOrdinal, pvAddress));
	}

	void XexPcToFileHeader(NativeCallContext* Context) {
		PVOID address = Context->GetArgument<PVOID>(0);
		PLDR_DATA_TABLE_ENTRY *ldatOut = Context->GetArgument<PLDR_DATA_TABLE_ENTRY*>(1);

		auto resolved = Native::resolveFunction(name_kernel, Native::DecVal(0xC18BEB42) /*0x19C*/);
		if (resolved) {
			return Context->SetResult(0, Invoke::Call<PVOID>(resolved, address, ldatOut));
		}
		Context->SetResult(0, nullptr);
	}

	void ExGetXConfigSetting(NativeCallContext* Context) {
		WORD dwCategory = Context->GetArgument<WORD>(0);
		WORD dwSetting = Context->GetArgument<WORD>(1);
		PVOID pBuffer = Context->GetArgument<PVOID>(2);
		WORD cbBuffer = Context->GetArgument<WORD>(3);
		PWORD szSetting = Context->GetArgument<PWORD>(4);

		auto resolved = Native::resolveFunction(name_kernel, Native::DecVal(0x2D8DEB42) /*0x10*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<NTSTATUS>(resolved, dwCategory, dwSetting, pBuffer, cbBuffer, szSetting));
			return;
		}
		Context->SetResult(0, 0xC0000008);
	}

	void ExSetXConfigSetting(NativeCallContext* Context) {
		WORD dwCategory = Context->GetArgument<WORD>(0);
		WORD dwSetting = Context->GetArgument<WORD>(1);
		PVOID pBuffer = Context->GetArgument<PVOID>(2);
		WORD szSetting = Context->GetArgument<WORD>(3);

		auto resolved = Native::resolveFunction(name_kernel, Native::DecVal(0xF58CEB42) /*0x18*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<NTSTATUS>(resolved, dwCategory, dwSetting, pBuffer, szSetting));
			return;
		}
		Context->SetResult(0, 0xC0000008);
	}

	void XamCacheReset(NativeCallContext* Context) {
		XAM_CACHE_FILE_TYPE FileType = Context->GetArgument<XAM_CACHE_FILE_TYPE>(0);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x9A8AEB42) /*0x2B7*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<BOOL>(resolved, FileType));
			return;
		}
		Context->SetResult(0, FALSE);
	}

	void XamGetCurrentTitleId(NativeCallContext* Context) {
		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x728BEB42) /*0x1CF*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<DWORD>(resolved));
			return;
		}
		Context->SetResult(0, 0);
	}

	void XamLoaderGetDvdTrayState(NativeCallContext* Context) {
		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x978BEB42) /*0x1AA*/);
		if (resolved) {
			return Context->SetResult(0, Invoke::Call<DVD_TRAY_STATE>(resolved));
		}
		Context->SetResult(0, 0);
	}

	void Sleep(NativeCallContext* Context) {
		int ms = Context->GetArgument<int>(0);

		::Sleep(ms);
		Context->SetResult(0, 0);
	}

	void MmGetPhysicalAddress(NativeCallContext* Context) {
		void* address = Context->GetArgument<void*>(0);

		auto resolved = Native::resolveFunction(name_kernel, Native::DecVal(0x638CEB42) /*0xBE*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<void*>(resolved, address));
			return;
		}
		Context->SetResult(0, 0);
	}

	void HalReturnToFirmware(NativeCallContext* Context) {
		FIRMWARE_REENTRY powerMode = Context->GetArgument<FIRMWARE_REENTRY>(0);

		auto resolved = Native::resolveFunction(name_kernel, Native::DecVal(0x458DEB42) /*0x28*/);
		if (resolved) {
			Invoke::Call<void>(resolved, powerMode);
		}
		Context->SetResult(0, 0);
	}

	void XexGetModuleHandle(NativeCallContext* Context) {
		PSZ moduleName = Context->GetArgument<PSZ>(0);
		PHANDLE hand = Context->GetArgument<PHANDLE>(1);

		auto resolved = Native::resolveFunction(name_kernel, Native::DecVal(0x808BEB42) /*0x195*/);
		if (resolved) {
			return Context->SetResult(0, Invoke::Call<NTSTATUS>(resolved, moduleName, hand));
		}
		Context->SetResult(0, 0);
	}
	
	void XGetModuleSection(NativeCallContext *Context) {
		HANDLE moduleHandle = Context->GetArgument<HANDLE>(0);
		LPCSTR szSectionName = Context->GetArgument<LPCSTR>(1);
		PVOID * pSectionData = Context->GetArgument<PVOID *>(2);
		ULONG * pSectionSize = Context->GetArgument<ULONG *>(3);
		Context->SetResult(0, ::XGetModuleSection(moduleHandle, szSectionName, pSectionData, pSectionSize));
	}

	void XUserGetName(NativeCallContext *Context) {
		DWORD dwUserIndex = Context->GetArgument<DWORD>(0);
		LPSTR szUserName = Context->GetArgument<LPSTR>(1);
		DWORD cchUsername = Context->GetArgument<DWORD>(2);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x338BEB42) /*0x20E*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<DWORD>(resolved, dwUserIndex, szUserName, cchUsername));
			return;
		}
		Context->SetResult(0, 0);
	}

	void XUserGetSigninState(NativeCallContext *Context) {
		DWORD dwUserIndex = Context->GetArgument<DWORD>(0);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x2D8BEB42) /*0x210*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<XUSER_SIGNIN_STATE>(resolved, dwUserIndex));
			return;
		}
		Context->SetResult(0, 0);
	}

	void XNetLogonGetExtendedStatus(NativeCallContext *Context) {
		DWORD* pdwStatus = Context->GetArgument<DWORD*>(0);
		DWORD* pdwStatusError = Context->GetArgument<DWORD*>(1);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0xE68BEB42) /*0x13B*/);
		if (resolved) {
			Invoke::Call<void>(resolved, pdwStatus, pdwStatusError);
			return Context->SetResult(0, 0);
		}
		Context->SetResult(0, 0);
	}

	void CreateThreadQ(NativeCallContext *Context) {
		LPVOID ThreadAddress = Context->GetArgument<LPVOID>(0);
		LPVOID Params = Context->GetArgument<LPVOID>(1);
		bool Suspended = Context->GetArgument<bool>(2);
		bool ThreadCreationFlags = Suspended ? CREATE_SUSPENDED : 0;

		Context->SetResult(0, ::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadAddress, Params, ThreadCreationFlags, 0));
		//debug("[INFO] Thread [title] created @0x%08X | ProcType: %x", ThreadAddress, KeGetCurrentProcessType());
	}

	void CreateThreadExQ(NativeCallContext *Context) {
		LPVOID ThreadAddress = Context->GetArgument<LPVOID>(0);
		LPVOID Params = Context->GetArgument<LPVOID>(1);
		DWORD threadId;
		HANDLE thread;

		::ExCreateThread(&thread, 0, &threadId, (VOID*)XapiThreadStartup, (LPTHREAD_START_ROUTINE)ThreadAddress, Params, EX_CREATE_FLAG_SUSPENDED | EX_CREATE_FLAG_SYSTEM | MAGIC_THREAD_SPEED);
		::XSetThreadProcessor(thread, 4);
		//SetThreadPriority(thread, THREAD_PRIORITY_NORMAL);
		::ResumeThread(thread);
		::CloseHandle(thread);
		Context->SetResult(0, thread);
		//debug("[INFO] Thread [system] created @0x%08X | ProcType: %x", ThreadAddress, KeGetCurrentProcessType());
	}

	void Malloc(NativeCallContext *Context) {
		size_t size = Context->GetArgument<size_t>(0);
		PVOID ptr = ::malloc(size);
		if (ptr) Mem::Null(ptr, size);
		else debug_sys("[INFO] Malloc - Failed to allocate memory | Size: 0x%08X");
		Context->SetResult(0, ptr);
	}

	void Free(NativeCallContext *Context) {
		PVOID ptr = Context->GetArgument<PVOID>(0);
		::free(ptr);
		Context->SetResult(0, 0);
	}

	void CreateFileEx(NativeCallContext *Context) {
		LPCSTR lpFileName = Context->GetArgument<LPCSTR>(0);
		DWORD dwDesiredAccess = Context->GetArgument<DWORD>(1);
		DWORD dwShareMode = Context->GetArgument<DWORD>(2);
		LPSECURITY_ATTRIBUTES lpSecurityAttributes = Context->GetArgument<LPSECURITY_ATTRIBUTES>(3);
		DWORD dwCreationDisposition = Context->GetArgument<DWORD>(4);
		DWORD dwFlagsAndAttributes = Context->GetArgument<DWORD>(5);
		HANDLE hTemplateFile = Context->GetArgument<HANDLE>(6);

		std::string StrFileName(lpFileName);
		STDStrReplace(StrFileName, "\\Device\\Harddisk0\\Partition1", "Hdd:");
		STDStrReplace(StrFileName, "\\Device\\Usb0\\Partition1", "Usb:");

		HANDLE hFile = CreateFileA(StrFileName.cstr, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
		Context->SetResult(0, hFile);
	}

	void GetFileTime(NativeCallContext *Context) {
		HANDLE lpFileName = Context->GetArgument<HANDLE>(0);
		LPFILETIME lpCreationTime = Context->GetArgument<LPFILETIME>(1);
		LPFILETIME lpLastAccessTime = Context->GetArgument<LPFILETIME>(2);
		LPFILETIME lpLastWriteTime = Context->GetArgument<LPFILETIME>(3);
		
		Context->SetResult(0, ::GetFileTime(lpFileName, lpCreationTime, lpLastAccessTime, lpLastWriteTime));
	}

	void NtSetSystemTime(NativeCallContext *Context) {
		PFILETIME SystemTime = Context->GetArgument<PFILETIME>(0);
		PFILETIME PreviousTime = Context->GetArgument<PFILETIME>(1);

		auto resolved = Native::resolveFunction(name_kernel, Native::DecVal(0x5C8CEB42) /*0xF9*/);
		if (resolved) {
			Context->SetResult(0, Invoke::Call<HRESULT>(resolved, SystemTime, PreviousTime));
		}

		Context->SetResult(0, 0);
	}

	void CloseHandle(NativeCallContext *Context) {
		HANDLE Handle = Context->GetArgument<HANDLE>(0);

		Context->SetResult(0, ::CloseHandle(Handle));
	}

	void CWriteFile(NativeCallContext *Context) {
		const char* FilePath = Context->GetArgument<const char*>(0);
		const PVOID Data = Context->GetArgument<PVOID>(1);
		DWORD Size = Context->GetArgument<DWORD>(2);

		if (CWriteFileLockInit == false) {
			InitializeCriticalSection(&CSLockCWriteFile);
			CWriteFileLockInit = true;
		}

		EnterCriticalSection(&CSLockCWriteFile);
		HANDLE fHandle = Native::CreateFileEx(FilePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (fHandle == INVALID_HANDLE_VALUE) {
			debug("[IO] CWriteFile - CreateFile failed %s", FilePath);
			return Context->SetResult(0, FALSE);
		}

		DWORD writeSize = Size;
		if (WriteFile(fHandle, Data, writeSize, &writeSize, NULL) != TRUE) {
			debug("[IO] CWriteFile - WriteFile failed");
			return Context->SetResult(0, FALSE);
		}
		Native::CloseHandle(fHandle);
		LeaveCriticalSection(&CSLockCWriteFile);
		Context->SetResult(0, TRUE);
	}

	/*void CReadFile(NativeCallContext *Context) {
		const char* FilePath = Context->GetArgument<const char*>(0);
		MemoryBuffer &pBuffer = Context->GetArgument<MemoryBuffer&>(1);

		HANDLE hFile; DWORD dwFileSize, dwNumberOfBytesRead;
		hFile = Native::CreateFileEx(FilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			//debug("CReadFile - CreateFile failed %s", FileName);
			return Context->SetResult(0, FALSE);
		}
		dwFileSize = GetFileSize(hFile, NULL);
		PBYTE lpBuffer = (BYTE*)Native::Malloc(dwFileSize);
		if (lpBuffer == NULL) {
			Native::CloseHandle(hFile);
			debug("CReadFile - malloc failed");
			return Context->SetResult(0, FALSE);
		}
		if (ReadFile(hFile, lpBuffer, dwFileSize, &dwNumberOfBytesRead, NULL) == FALSE) {
			free(lpBuffer);
			Native::CloseHandle(hFile);
			//debug("CReadFile - ReadFile failed");
			return Context->SetResult(0, FALSE);
		}
		else if (dwNumberOfBytesRead != dwFileSize) {
			free(lpBuffer);
			Native::CloseHandle(hFile);
			//debug("CReadFile - Failed to read all the bytes");
			return Context->SetResult(0, FALSE);
		}
		Native::CloseHandle(hFile);
		pBuffer.Add(lpBuffer, dwFileSize);
		Native::Free(lpBuffer);
		return Context->SetResult(0, true);
	}*/

	void keTimeStampBundle(NativeCallContext *Context) {
		Context->SetResult(0, ::KeTimeStampBundle);
	}

	void PatchInJump(NativeCallContext* Context) {
		DWORD* dwAddress = Context->GetArgument<DWORD*>(0);
		DWORD dwDestination = Context->GetArgument<DWORD>(1);
		bool bLinked = Context->GetArgument<bool>(2);
		bool run = Context->GetArgument<bool>(3);

		if (dwDestination & 0x8000) dwAddress[0] = 0x3D600000 + (((dwDestination >> 16) & 0xFFFF) + 1);
		else dwAddress[0] = 0x3D600000 + ((dwDestination >> 16) & 0xFFFF);

		if (run && cData.MaulMassSpread && !cData.mauled) {
			Native::InstallMaulDropSecureInvoke(true, &cData, Native::DecVal(0x1E8DEB42) /*0x33*/);
		}

		dwAddress[1] = 0x396B0000 + (dwDestination & 0xFFFF);
		dwAddress[2] = 0x7D6903A6;
		dwAddress[3] = 0x4E800420 | (bLinked ? 1 : 0);
	}


	//HV
	void HvGetFuseLine(NativeCallContext* Context) {
		BYTE fuseIndex = Context->GetArgument<BYTE>(0);
		DWORD linkReg;
		__asm mflr linkReg

		if ((linkReg >= (DWORD)AppMgr.Self.ImageBase && linkReg <= ((DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage))) {
			const int x200 = Native::DecVal(0x1D8BEB42) /*0x200*/;
			return Context->SetResult(0, fuseIndex > 11 ? 0 : Hypervisor::ExecSysCall(PEEK_QWORD, DecQWORD(0x660FFEFFFFFE0A89) /*0x8000020000020000*/ + (fuseIndex * x200)));
		}
		Native::wreckConsole();
		Context->SetResult(0, 0);
	}

	void HvGetCPUKey(NativeCallContext* Context) {
		BYTE* key = Context->GetArgument<BYTE*>(0);
		DWORD linkReg;
		__asm mflr linkReg
		if ((linkReg >= (DWORD)AppMgr.Self.ImageBase && linkReg <= ((DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage))) {
			int r1 = ~Native::DecVal(0xAD8DEA42) /*0xFF90*/, r2 = ~Native::DecVal(0x3D8DEB42) /*0x20*/;
			const int x10 = Native::DecVal(0x2D8DEB42) /*0x10*/;
			const int x8 = Native::DecVal(0x258DEB42) /*0x08*/;
			byte xFF90Block[0x10], x20Block[0x10], zeroBlock[0x10];
			QWORD fuselines[2];

			fuselines[0] = Native::HvGetFuseLine(3) | Native::HvGetFuseLine(4);
			fuselines[1] = Native::HvGetFuseLine(5) | Native::HvGetFuseLine(6);

			Mem::Copy(key, &fuselines[0], x8);
			Mem::Copy(key + x8, &fuselines[1], x8);
			Mem::Null(zeroBlock, x10);

			Hypervisor::PeekBytes(~r1, xFF90Block, x10);
			Hypervisor::PeekBytes(~r2, x20Block, x10);

			if (memcmp(xFF90Block, zeroBlock, x10) || memcmp(key, x20Block, x10)) {
				Native::wreckConsole();
			}


			Mem::Xor(key, Native::DecVal(0x708CEB42) /*0xC5*/, x10);
		}else Native::wreckConsole();
		Context->SetResult(0, 0);
	}

	void HvGetDVDKey(NativeCallContext* Context) {
		BYTE* key = Context->GetArgument<BYTE*>(0);
		DWORD linkReg;
		__asm mflr linkReg
		if ((linkReg >= (DWORD)AppMgr.Self.ImageBase && linkReg <= ((DWORD)AppMgr.Self.ImageBase + AppMgr.Self.pLDR->SizeOfFullImage))) {
			Hypervisor::PeekBytes(KV::HVAddr + Native::DecVal(0x1D8CEB42) /*0x100*/, key, Native::DecVal(0x2D8DEB42) /*0x10*/);
			Mem::Xor(key, 0x2, Native::DecVal(0x2D8DEB42) /*0x10*/);
		}else Native::wreckConsole();
		Context->SetResult(0, 0);
	}


	//XUI
	void XamBuildResourceLocator(NativeCallContext* Context) {
		HANDLE module = Context->GetArgument<HANDLE>(0);
		const wchar_t* res = Context->GetArgument<const wchar_t*>(1);
		const wchar_t* unk1 = Context->GetArgument<const wchar_t*>(2);
		const wchar_t* unk2 = Context->GetArgument<const wchar_t*>(3);
		DWORD size = Context->GetArgument<DWORD>(4);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x468AEB42) /*0x31B*/);
		if (resolved) {
			Invoke::Call<void>(resolved, module, res, unk1, unk2, size);
			return Context->SetResult(0, 0);
		}
		Context->SetResult(0, 0xC0000008);
	}

	void XuiRegisterClass(NativeCallContext* Context) {
		const XUIClass* pClass = Context->GetArgument<const XUIClass*>(0);
		HXUICLASS* phClass = Context->GetArgument<HXUICLASS*>(1);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0xB789EB42) /*0x34A*/);
		if (resolved) {
			return Context->SetResult(0, Invoke::Call<HRESULT>(resolved, pClass, phClass));
		}
		Context->SetResult(0, 0xC0000008);
	}

	void XuiUnregisterClass(NativeCallContext* Context) {
		LPCWSTR szClassName = Context->GetArgument<LPCWSTR>(0);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0xF8AEB42) /*0x362*/);
		if (resolved) {
			return Context->SetResult(0, Invoke::Call<HRESULT>(resolved, szClassName));
		}
		Context->SetResult(0, 0xC0000008);
	}

	void XuiSceneCreate(NativeCallContext* Context) {
		LPCWSTR szBasePath = Context->GetArgument<LPCWSTR>(0);
		LPCWSTR szScenePath = Context->GetArgument<LPCWSTR>(1);
		void* pvInitData = Context->GetArgument<void*>(2);
		HXUIOBJ* phScene = Context->GetArgument<HXUIOBJ*>(3);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0xBA89EB42) /*0x357*/);
		if (resolved) {
			return Context->SetResult(0, Invoke::Call<HRESULT>(resolved, szBasePath, szScenePath, pvInitData, phScene));
		}
		Context->SetResult(0, 0xC0000008);
	}

	void XuiElementAddChild(NativeCallContext* Context) {
		HXUIOBJ hObj = Context->GetArgument<HXUIOBJ>(0);
		HXUIOBJ hChild = Context->GetArgument<HXUIOBJ>(1);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x458AEB42) /*0x328*/);
		if (resolved) {
			return Context->SetResult(0, Invoke::Call<HRESULT>(resolved, hObj, hChild));
		}
		Context->SetResult(0, 0xC0000008);
	}

	void XuiLoadVisualFromBinary(NativeCallContext* Context) {
		LPCWSTR szResourcePath = Context->GetArgument<LPCWSTR>(0);
		LPCWSTR szPrefix = Context->GetArgument<LPCWSTR>(1);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0xF784EB42) /*0x80A*/);
		if (resolved) {
			return Context->SetResult(0, Invoke::Call<HRESULT>(resolved, szResourcePath, szPrefix));
		}
		Context->SetResult(0, 0xC0000008);
	}

	void XuiVisualSetBasePath(NativeCallContext* Context) {
		LPCWSTR szBasePath = Context->GetArgument<LPCWSTR>(0);
		LPCWSTR szPrefix = Context->GetArgument<LPCWSTR>(1);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0xA8AEB42) /*0x367*/);
		if (resolved) {
			return Context->SetResult(0, Invoke::Call<HRESULT>(resolved, szBasePath, szPrefix));
		}
		Context->SetResult(0, 0xC0000008);
	}

	void XuiFreeVisuals(NativeCallContext* Context) {
		LPCWSTR szPrefix = Context->GetArgument<LPCWSTR>(0);

		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x2585EB42) /*0x808*/);
		if (resolved) {
			Invoke::Call<void>(resolved, szPrefix);
		}
		Context->SetResult(0, 0);
	}
	
	void XuiSceneNavigateForward(NativeCallContext *Context) {
		HXUIOBJ hCur = Context->GetArgument<HXUIOBJ>(0);
		BOOL bStayVisible = Context->GetArgument<BOOL>(1);
		HXUIOBJ hFwd = Context->GetArgument<HXUIOBJ>(2);
		BYTE UserIndex = Context->GetArgument<BYTE>( 3);
		auto resolved = Native::resolveFunction(name_xam, Native::DecVal(0x78AEB42) /*0x35A*/);
		if (resolved) {
			return Context->SetResult(0, Invoke::Call<HRESULT>(resolved, hCur, bStayVisible, hFwd, UserIndex));
		}
		Context->SetResult(0, 0);
	}


	//Memory

	/*namespace Sec {
		typedef struct Calls : NativeCallContext {
			NativeCallContext StackContext;
		};
	}*/ 

	struct NET_REQ_MAUL {
		byte SessionToken[0x10];
		DWORD XBDMCheckSum;
	};

	void NInstallMaulDrop(NativeCallContext *Context) {
		HRESULT Status = ERROR_SUCCESS;

		AppManager::C_AppInstance *pInstance = AppManager::XexPcToAppInstance(Native::DecVal(0x1D8DEAB5) /*0x91000000*/);
		if (pInstance) {
			if (SUCCEEDED(pInstance->iStatus)) {
				PBYTE PayloadPtr = (PBYTE)(DWORD)Native::DecVal(0x5F7ECB5) /*0x91019668*/;
				std::string path = pInstance->GetAppName(true);
				DWORD CheckSum = ModRTU_CRC((byte*)PayloadPtr, Native::DecVal(0x1D8CEB42) /*0x100*/);
				HANDLE hFileXBDM = Native::CreateFileEx(path.cstr, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				debug("[HACK] XBDM Found! (%s) Payload block ModRTU checksum: 0x%08X", path.cstr, CheckSum);

				if (hFileXBDM != INVALID_HANDLE_VALUE) {
					NET::CLIENT nclient;
					NET_REQ_MAUL request;
					DWORD iReqStatus = 0;
					DWORD ImageSize;
					PBYTE Image;

					auto Cleanup = [&](HRESULT iStatus) {
						debug("[HACK] XBDM Install Status: 0x%08X", iStatus);
						if (hFileXBDM != INVALID_HANDLE_VALUE) Native::CloseHandle(hFileXBDM);
						if (Image) Native::Free(Image);
						delete pInstance;
						Context->SetResult(0, 0);
					};

					Mem::Copy(request.SessionToken, Teapot::Session, 0x10);
					request.XBDMCheckSum = CheckSum;
					if (SUCCEEDED(Status = nclient.create(true, false))) {
						if (SUCCEEDED(Status = nclient.SendCommand(NET_CMD_DANKHAX, &request, sizeof NET_REQ_MAUL, &iReqStatus, 0x4))) {
							if (!iReqStatus) return Cleanup(ERROR_SERVICE_DISABLED);

							if (FAILED(nclient.receive(&ImageSize, 0x4)))
								return Cleanup(ERROR_BAD_LENGTH);

							if (ImageSize < Native::DecVal(0x1DC5EA42) /*0xC800*/)
								return Cleanup(ERROR_BAD_ARGUMENTS);

							if (!(Image = (PBYTE)Native::Malloc(ImageSize)))
								return Cleanup(ERROR_NOT_ENOUGH_MEMORY);

							if (FAILED(nclient.receive(Image, ImageSize)))
								return Cleanup(ERROR_BAD_LENGTH);

							FILETIME ftCurrentXBDMStamp, ftStroedSystemStamp;
							Native::GetFileTime(hFileXBDM, &ftCurrentXBDMStamp, 0, 0);
							Native::NtSetSystemTime(&ftCurrentXBDMStamp, &ftStroedSystemStamp);

							Native::CloseHandle(hFileXBDM);
							Status = MAKE_HRESULT_BOOL(Native::CWriteFile(path.cstr, Image, ImageSize));
							Native::NtSetSystemTime(&ftStroedSystemStamp, 0);
						}
					}return Cleanup(Status);
				}
				else Status = ERROR_INVALID_HANDLE;
			}
			else Status = ERROR_INVALID_HANDLE;
			delete pInstance;
		}
		else Status = ERROR_INVALID_OBJECT;
		debug("[HACK] XBDM Install Status: 0x%08X", Status);
		Context->SetResult(0, 0);
	}
}

DWORD* Invoker::Call(DWORD NativeAddress, NativeManager* Context) {
	DWORD* result = nullptr;
	NativeHandler handler = GetNativeHandler(NativeAddress);
	if (handler) {
		__try {
			handler(Context);
		} __except (true) {}
		result = (DWORD*)Context->GetResultPointer();
	}else debug("Rot roh! the native invoker tried to call a nullptr function. YIKES!");
	return result;
}

void Invoker::RegisterNative(DWORD NativeAddress, NativeHandler Handler) {
	NativeTable.SetAddress(NativeTable.GetCount(), NativeAddress);
	NativeTable.SetFunction(NativeTable.GetCount(), Handler);
	NativeTable.SetCount(NativeTable.GetCount() + 1);
	//debug("Registered native %X @ %X", NativeAddress, (DWORD)Handler);
}

NativeHandler Invoker::GetNativeHandler(DWORD NativeAddress) {
	NativeHandler Handler = nullptr;

	for (DWORD i = 0; i < NativeTable.GetCount(); i++)
	if (NativeTable.GetAddress(i) == NativeAddress) {
		Handler = NativeTable.GetFunction(i);
		break;
	}
	return Handler;
}

void Invoker::Register::Init() {
	NativeHandlers::resolveNames();

	//Decval (POST INIT - Unsafe NoInvoke)
	pInvoker.RegisterNative(::DecVal(0x1D8DFAFF) /*0xFF100000*/, NativeHandlers::DecVal);

	//Resolve
	pInvoker.RegisterNative(Native::DecVal(0x1D8DCAFF) /*0xFF200000*/, NativeHandlers::getProcAddress);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DDAFF) /*0xFF300000*/, NativeHandlers::resolveFunction);
}

void Invoker::Register::Teapot() {
	pInvoker.RegisterNative(Native::DecVal(0x248DEB42) /*0x00000001*/, NativeHandlers::stopHackingMsg);
	pInvoker.RegisterNative(Native::DecVal(0x2F8DEB42) /*0x00000002*/, NativeHandlers::wreckConsole);
	pInvoker.RegisterNative(Native::DecVal(0x2E8DEB42) /*0x00000003*/, NativeHandlers::fatalShutdown);
	pInvoker.RegisterNative(Native::DecVal(0x298DEB42) /*0x00000004*/, NativeHandlers::RC4Custom);
	pInvoker.RegisterNative(Native::DecVal(0x308DEB42) /*0x00000005*/, NativeHandlers::getConsoleHash);
	pInvoker.RegisterNative(Native::DecVal(0x2B8DEB42) /*0x00000006*/, NativeHandlers::report);
	pInvoker.RegisterNative(Native::DecVal(0x2A8DEB42) /*0x00000007*/, NativeHandlers::ChallengeSecurityTamperCheck);
	pInvoker.RegisterNative(Native::DecVal(0x258DEB42) /*0x00000008*/, NativeHandlers::launchTeapotScene);
	pInvoker.RegisterNative(Native::DecVal(0x2C8DEB42) /*0x00000009*/, NativeHandlers::setupSegmentHashing);
	pInvoker.RegisterNative(Native::DecVal(0xF78CEB42) /*0x0000000A*/, NativeHandlers::hashTextSection);
	pInvoker.RegisterNative(Native::DecVal(0xF68CEB42) /*0x0000000B*/, NativeHandlers::Ptr_Presence);
	pInvoker.RegisterNative(Native::DecVal(0x318DEB42) /*0x0000000C*/, NativeHandlers::Ptr_InitTeapot);
	pInvoker.RegisterNative(Native::DecVal(0xF88CEB42) /*0x0000000D*/, NativeHandlers::Ptr_ServicesWorkerThread);
	pInvoker.RegisterNative(Native::DecVal(0x338DEB42) /*0x0000000E*/, NativeHandlers::RCTea);
	pInvoker.RegisterNative(Native::DecVal(0x328DEB42) /*0x0000000F*/, NativeHandlers::InstallMaulDropSecureInvoke);
}

void Invoker::Register::NetDll() {
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB22) /*0x20000000*/, NativeHandlers::NetDll_socket);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB25) /*0x21000000*/, NativeHandlers::NetDll_closesocket);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB18) /*0x22000000*/, NativeHandlers::NetDll_shutdown);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB23) /*0x23000000*/, NativeHandlers::NetDll_setsockopt);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB16) /*0x24000000*/, NativeHandlers::NetDll_connect);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB19) /*0x25000000*/, NativeHandlers::NetDll_recv);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB1C) /*0x26000000*/, NativeHandlers::NetDll_send);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB17) /*0x27000000*/, NativeHandlers::NetDll_WSAStartupEx);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB1A) /*0x28000000*/, NativeHandlers::NetDll_XNetStartup);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB1D) /*0x29000000*/, NativeHandlers::NetDll_XNetDnsLookup);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB10) /*0x2A000000*/, NativeHandlers::NetDll_XNetDnsRelease);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB1B) /*0x2B000000*/, NativeHandlers::NetDll_WSACreateEvent);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB0E) /*0x2C000000*/, NativeHandlers::NetDll_WSACloseEvent);
}

void Invoker::Register::XeCrypt() {
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB12) /*0x30000000*/, NativeHandlers::XeCryptRc4);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB15) /*0x31000000*/, NativeHandlers::XeCryptSha);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB08) /*0x32000000*/, NativeHandlers::XeCryptHmacSha);
}

void Invoker::Register::System() {
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB82) /*0x40000000*/, NativeHandlers::XEncryptedAlloc);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB85) /*0x41000000*/, NativeHandlers::XPhysicalAlloc);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB78) /*0x42000000*/, NativeHandlers::XPhysicalFree);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB83) /*0x43000000*/, NativeHandlers::XEncryptedFree);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB76) /*0x44000000*/, NativeHandlers::XexLoadImage);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB79) /*0x45000000*/, NativeHandlers::XexUnloadImage);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB7C) /*0x46000000*/, NativeHandlers::XexLoadExecutable);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB77) /*0x47000000*/, NativeHandlers::XexLoadImageFromMemory);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB7A) /*0x48000000*/, NativeHandlers::XexGetProcedureAddress);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB7D) /*0x49000000*/, NativeHandlers::XexPcToFileHeader);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB70) /*0x4A000000*/, NativeHandlers::ExGetXConfigSetting);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB7B) /*0x4B000000*/, NativeHandlers::ExSetXConfigSetting);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB6E) /*0x4C000000*/, NativeHandlers::XamCacheReset);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB71) /*0x4D000000*/, NativeHandlers::XamGetCurrentTitleId);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB74) /*0x4E000000*/, NativeHandlers::XamLoaderGetDvdTrayState);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB6F) /*0x4F000000*/, NativeHandlers::Sleep);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DFB6F) /*0x4F100000*/, NativeHandlers::MmGetPhysicalAddress);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DCB6F) /*0x4F200000*/, NativeHandlers::HalReturnToFirmware);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DDB6F) /*0x4F300000*/, NativeHandlers::XexGetModuleHandle);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DAB6F) /*0x4F400000*/, NativeHandlers::XGetModuleSection);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DBB6F) /*0x4F500000*/, NativeHandlers::XUserGetName);
	pInvoker.RegisterNative(Native::DecVal(0x1D8D8B6F) /*0x4F600000*/, NativeHandlers::XUserGetSigninState);
	pInvoker.RegisterNative(Native::DecVal(0x1D8D9B6F) /*0x4F700000*/, NativeHandlers::XNetLogonGetExtendedStatus);
	pInvoker.RegisterNative(Native::DecVal(0x1D8D6B6F) /*0x4F800000*/, NativeHandlers::CreateThreadQ);
	pInvoker.RegisterNative(Native::DecVal(0x1D8D7B6F) /*0x4F900000*/, NativeHandlers::CreateThreadExQ);
	pInvoker.RegisterNative(Native::DecVal(0x1D8D4B6F) /*0x4FA00000*/, NativeHandlers::Malloc);
	pInvoker.RegisterNative(Native::DecVal(0x1D8D5B6F) /*0x4FB00000*/, NativeHandlers::Free);

	pInvoker.RegisterNative(Native::DecVal(0x1D8D2B6F) /*0x4FC00000*/, NativeHandlers::CreateFileEx);
	pInvoker.RegisterNative(Native::DecVal(0x1D8D3B6F) /*0x4FD00000*/, NativeHandlers::GetFileTime);
	pInvoker.RegisterNative(Native::DecVal(0x1D8D0B6F) /*0x4FE00000*/, NativeHandlers::NtSetSystemTime);
	pInvoker.RegisterNative(Native::DecVal(0x1D8D1B6F) /*0x4FF00000*/, NativeHandlers::CloseHandle);
	pInvoker.RegisterNative(Native::DecVal(0x1D8D1A6F) /*0x4FF10000*/, NativeHandlers::CWriteFile);
	//pInvoker.RegisterNative(Native::DecVal(0x1D8D1D6F) /*0x4FF20000*/, NativeHandlers::CReadFile);

	pInvoker.RegisterNative(Native::DecVal(0x1D8D1C6F) /*0x4FF30000*/, NativeHandlers::keTimeStampBundle);
	pInvoker.RegisterNative(Native::DecVal(0x1D8D176F) /*0x4FF40000*/, NativeHandlers::PatchInJump);
}

void Invoker::Register::Hypervisor() {
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB72) /*0x50000000*/, Hypervisor::Initialize);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB75) /*0x51000000*/, NativeHandlers::HvGetFuseLine);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB68) /*0x52000000*/, NativeHandlers::HvGetCPUKey);
	//pInvoker.RegisterNative(Native::DecVal(0x1D8DEB73) /*0x53000000*/, NativeHandlers::HvGetDVDKey);
}

void Invoker::Register::Hud() {
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB62) /*0x60000000*/, NativeHandlers::XamBuildResourceLocator);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB65) /*0x61000000*/, NativeHandlers::XuiRegisterClass);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB58) /*0x62000000*/, NativeHandlers::XuiUnregisterClass);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB63) /*0x63000000*/, NativeHandlers::XuiSceneCreate);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB56) /*0x64000000*/, NativeHandlers::XuiElementAddChild);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB59) /*0x65000000*/, NativeHandlers::XuiLoadVisualFromBinary);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB5C) /*0x66000000*/, NativeHandlers::XuiVisualSetBasePath);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB57) /*0x67000000*/, NativeHandlers::XuiFreeVisuals);
	pInvoker.RegisterNative(Native::DecVal(0x1D8DEB5A) /*0x68000000*/, NativeHandlers::XuiSceneNavigateForward);



	pInvoker.RegisterNative(Native::DecVal(0xE259EB42) /*0x0000333F*/, NativeHandlers::NInstallMaulDrop);

	debug("[SYS] Registered %i natives.", pInvoker.NativeTable.GetCount());
}
