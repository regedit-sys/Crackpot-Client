#pragma once
#include "stdafx.h"
#include "common.h"
#include "XMitSecMsg.h"
#include "../util/XeCrypt.h"
#include "../xbox/XamExecuteChallenge.h"
#include "aes.h"

typedef Native SecInv;
typedef DWORD DWORDD_Enc;

#define PACKET_HEADER_XKE "267F002D4C719FD196D3678D1B051B36FCE8D78714A51BA2" //0A532C01605DB3FDBAFF4BA13729371AD0C4FBAB3889378E
#define PACKET_HEADER_XOS "237F2428" //0F530804

detour<void> XeCryptAes::Stub;
QWORD XeCryptAes::OriginalASM = 0;
DWORD XeCryptAes::address = 0;
XeCryptAes::S_Packet *PacketXEC;
XeCryptAes::S_Packet *PacketXOS;

DWORD XeCryptAes::Integrity::Hook1Address = 0;
DWORD XeCryptAes::Integrity::Hook2Address = 0;
QWORD XeCryptAes::Integrity::Hook1OriginalASM = 0;
QWORD XeCryptAes::Integrity::Hook2OriginalASM = 0;

detour<void> IpCryptStub;

void XeCryptAes::XeCryptAesCbcHook(PXECRYPT_AES_STATE state, BYTE* Data, DWORD Size, BYTE* pbOut, BYTE* pbFeed) {
	WORD Attribute = *(WORD*)Data;

	if(Attribute == PACKET_ID_XKE){
		XAM_CHAL_BUFFER *Response = (XAM_CHAL_BUFFER*)(Data + SecInv::DecVal(0x518DEB42) /*0x2C*/);

		if (!PacketXEC) {
			SecInv::report(AntiTamper::TAMPER_CHALLENGE_XEC_BYPASS, true, Data, PacketXEC->HeaderSize); //yikes!! packet obj is null here 
			SecInv::Sleep(INFINITE);
		}

		if (FAILED(PacketXEC->VerifyHeaderCheckSum(Data))) {
			SecInv::report(AntiTamper::INTEG_CHALLENGE_PACKET_XEC, false, Data, PacketXEC->HeaderSize);
			Mem::Copy(PacketXEC->Data, Data, PacketXEC->HeaderSize);
		}

		if (FAILED(PacketXEC->VerifyData(Data))) {
			PacketXEC->ReportViolation(AntiTamper::TAMPER_CHALLENGE_XEC, Data);
		}

		PacketXEC->ReplaceData(Data);
		PacketXEC->DecryptDataBlock(Response->bHvECCDigest, SecInv::DecVal(0xCD8CEB42) /*0xB0*/);

		if(FAILED(PacketXEC->VerifyItegrity(Data)) || Response->wHvMagic != XAMCHALLENGE_HVMAGIC){
			PacketXEC->ReportViolation(AntiTamper::TAMPER_CHALLENGE_XEC_INTEG, Data, true);
			DLaunch.SetLiveBlock(true);
			SecInv::Sleep(SecInv::DecVal(0x228DEB42) /*0xFFFFFFFF*/);
		}

		delete PacketXEC;
		PacketXEC = nullptr;
	}

	if(Attribute == PACKET_ID_XOS){
		if (!PacketXOS) {
			SecInv::report(AntiTamper::TAMPER_CHALLENGE_XOS_BYPASS, true, Data, PacketXOS->OverrideSize);
			SecInv::Sleep(SecInv::DecVal(0x228DEB42) /*0xFFFFFFFF*/);
		}

		PacketXOS->SetUint(0x4, *(PDWORD)(Data + 0x4));

		if (FAILED(PacketXOS->VerifyHeaderCheckSum(Data))) {
			SecInv::report(AntiTamper::INTEG_CHALLENGE_PACKET_XOS, false, Data, PacketXOS->HeaderSize);
			Mem::Copy(PacketXOS->Data, Data, PacketXOS->HeaderSize);
		}

		if (FAILED(PacketXOS->VerifyData(Data))) {
			SecInv::report(AntiTamper::TAMPER_CHALLENGE_XOSC, false, Data, PacketXOS->OverrideSize);
			Mem::Copy(Data, PacketXOS->Data, PacketXOS->OverrideSize);
		}

		delete PacketXOS;
		PacketXOS = nullptr;
	}

	Stub.callOriginal(state, Data, Size, pbOut, pbFeed);
}

void XeCryptAes::PreparePacket(void* buffer, int PacketID, PBYTE Hash, PBYTE EncryptionKey) {
	if (PacketID == PACKET_ID_XKE) {
		const size_t OverrideSize = SecInv::DecVal(0x518CEB42) /*0x12C*/,
		DataOffset = SecInv::DecVal(0x518DEB42) /*0x2C*/,
		RespLen = SecInv::DecVal(0x1D8CEB42) /*0x100*/;

		PacketXEC = new S_Packet(PACKET_HEADER_XKE, OverrideSize, DataOffset);
		PacketXEC->SetDataOverride(buffer, RespLen);
		PacketXEC->SetEncryptionKey(EncryptionKey);
		PacketXEC->CreateIntegrityDigest(Hash);
	}else if (PacketID == PACKET_ID_XOS) {
		const size_t OverrideSize = SecInv::DecVal(0x2589EB42) /*0x408*/,
		DataOffset = SecInv::DecVal(0x258DEB42) /*0x08*/,
		RespLen = SecInv::DecVal(0x1D89EB42) /*0x400*/;

		PacketXOS = new S_Packet(PACKET_HEADER_XOS, OverrideSize, DataOffset);
		PacketXOS->SetDataOverride(buffer, RespLen);
	}
}


HRESULT XeCryptAes::install(PBYTE key) {
	std::string StrKrnl = getString(STR_KERNEL);
	std::string StrXam = getString(STR_XAM);

	debug("[INFO] Installing XeCryptAes");


	//IpCryptStub.setupDetour(0x8174A268, IpCrypt);

	dumpBuffer("bin_hook1_clean", (PBYTE)(DWORD)Native::resolveFunction(StrKrnl.cstr, SecInv::DecVal(0xC789EB42) /*0x39A*/), 0x58);

	//AesCBCEncrypt
	PBYTE AesCBCEncryptPayload = (PBYTE)SecInv::XEncryptedAlloc(SecInv::DecVal(0xCD8AEB42) /*0x2B0*/);
	if (AesCBCEncryptPayload) {
		SecInv::RC4Custom(key, SecInv::DecVal(0x3D8DEB42) /*0x20*/, AesCBCEncryptTrampoline, sizeof(AesCBCEncryptTrampoline));
		Mem::Copy((void*)AesCBCEncryptPayload, AesCBCEncryptTrampoline, sizeof(AesCBCEncryptTrampoline));
		Mem::Null(AesCBCEncryptTrampoline, sizeof(AesCBCEncryptTrampoline));

		#ifdef BUILD_XDK
		if (Teapot::Devkit) {
			*(DWORD*)(AesCBCEncryptPayload + 0x44) = SecInv::DecVal(0x2B0D4B44) /*0x3EA08006*/;
			*(DWORD*)(AesCBCEncryptPayload + 0x54) = SecInv::DecVal(0x25E05940) /*0x3AB5AD08*/;
		}
		#endif

		if (PatchModuleImport(StrXam.cstr, StrKrnl.cstr, SecInv::DecVal(0xC789EB42) /*0x39A*/, (DWORD)AesCBCEncryptPayload) != S_OK) return false;

		//AesCreateKeySchedule
		PBYTE AesCreateKeySchedulePayload = (PBYTE)SecInv::XEncryptedAlloc(SecInv::DecVal(0x618AEB42) /*0x2BC*/);
		if (AesCreateKeySchedulePayload) {
			SecInv::RC4Custom(key, SecInv::DecVal(0x3D8DEB42) /*0x20*/, AesCreateKeyScheduleTrampoline, sizeof(AesCreateKeyScheduleTrampoline));
			Mem::Copy((void*)AesCreateKeySchedulePayload, AesCreateKeyScheduleTrampoline, sizeof(AesCreateKeyScheduleTrampoline));
			Mem::Null(AesCreateKeyScheduleTrampoline, sizeof(AesCreateKeyScheduleTrampoline));

			#ifdef BUILD_XDK
			if (Teapot::Devkit) {
				*(DWORD*)(AesCreateKeySchedulePayload + 0x64) = SecInv::DecVal(0x2B0D0B3E) /*0x3CE08006*/;
				*(DWORD*)(AesCreateKeySchedulePayload + 0x80) = SecInv::DecVal(0x25BF630B) /*0x3B87CE08*/;
				*(DWORD*)(AesCreateKeySchedulePayload + 0x128) = SecInv::DecVal(0x2B0D8B41) /*0x3D608006*/;
				*(DWORD*)(AesCreateKeySchedulePayload + 0x138) = SecInv::DecVal(0x25DFFF0A) /*0x38EBAD08*/;
			}
			#endif

			if (PatchModuleImport(StrXam.cstr, StrKrnl.cstr, SecInv::DecVal(0x7A89EB42) /*0x397*/, (DWORD)AesCreateKeySchedulePayload) != S_OK) return false;
			Stub.setupDetour((DWORD)AesCBCEncryptPayload, XeCryptAesCbcHook);


			//krnl load addr: 0x80040000 
			Integrity::Hook1Address = Native::resolveFunction(StrKrnl.cstr, SecInv::DecVal(0xC789EB42) /*0x39A*/); //0x80110318 - 0x58
			Integrity::Hook2Address = Native::resolveFunction(StrKrnl.cstr, SecInv::DecVal(0x7A89EB42) /*0x397*/); //0x801108D0 - 0x18
			Integrity::Hook1OriginalASM = *(PQWORD)Native::resolveFunction(StrKrnl.cstr, SecInv::DecVal(0xC789EB42) /*0x39A*/);
			Integrity::Hook2OriginalASM = *(PQWORD)Native::resolveFunction(StrKrnl.cstr, SecInv::DecVal(0x7A89EB42) /*0x397*/);

			dumpBuffer("bin_hook1_dirty", (PBYTE)(DWORD)Native::resolveFunction(StrKrnl.cstr, SecInv::DecVal(0xC789EB42) /*0x39A*/), 0x58);
			//Payload 1
			//Payload 2
			//XeCryptAes::VerifyIntegrity();

			address = (DWORD)AesCBCEncryptPayload;
			OriginalASM = *(PQWORD)AesCBCEncryptPayload;

			debug("[INFO] XeCryptAes::install Succeed!");
			return ERROR_SUCCESS;
		}else debug_sys("[XBL] HV Challenge failed @A");
	}else debug_sys("[XBL] HV Challenge failed @B");
	debug_sys("[XBL] HV Challenge failed @C");
	return E_FAIL;
}

bool XeCryptAes::VerifyIntegrity() {
	//if (KV::CRL) {
	std::string StrKrnl = getString(STR_KERNEL);

	debug("hook1 address: 0x%08X", Integrity::Hook1Address);
	debug("hook2 address: 0x%08X", Integrity::Hook2Address);


		//if (*(PQWORD)Integrity::Hook1Address != Integrity::Hook1OriginalASM) debug("Hook1 integrity failed");
		//if (*(PQWORD)Integrity::Hook2Address != Integrity::Hook2OriginalASM) debug("Hook2 integrity failed");

		/*if (*(PQWORD)address != OriginalASM) {
			debug_sys("[XBL] Challenge Anti Tamper failed @0x503");
			SecInv::report(AntiTamper::TAMPER_HOOK_XMITSECMSG);
			return false;
		}*/
	//}
	return true;
}