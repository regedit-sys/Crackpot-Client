#include "stdafx.h"
#include "common.h" 
#include "CXnIp_IpXmitSecMsg.h"

byte *CXnIp_IpXmitSecMsg::xeKeysBuffer;
byte *CXnIp_IpXmitSecMsg::xeKeysECCDigestHash;
byte *CXnIp_IpXmitSecMsg::xoscBuffer;
CXnIp_IpXmitSecMsgHookValues_s CXnIp_IpXmitSecMsg::values;
detour<void> CXnIp_IpXmitSecMsg::tour;
QWORD OriginalASM = 0;

void CXnIp_IpXmitSecMsg::CXnIp_IpXmitSecMsgHook(int r3, XNCALLER_TYPE xnc, int* CSecReg, unsigned __int16 msgFlag, byte* dataBuff, unsigned int size, void * unk2, unsigned int unk3) {
	if (msgFlag == values.desiredMsg && size == values.desiredSize) {
		byte ECCDigestHash[0x10];
		if (memcmp(xeKeysBuffer, (BYTE*)(dataBuff + values.bufferOfset), values.bufferSize) != 0) {
			AntiTamper::report(AntiTamper::TAMPER_CHALLENGE_XKE);
		}

		memcpy((BYTE*)(dataBuff + values.bufferOfset), xeKeysBuffer, values.bufferSize); //COPY OUR CLEAN XEKEYS BUFFER
		xorBlock((BYTE*)(dataBuff + values.bufferOfset) + values.xekeysOffset, 0x14, sessionToken[6]); //UNXOR OUR ECC DIGEST
		xorBlock((BYTE*)(dataBuff + 0xDA) + values.xekeysOffset, 0x6, sessionToken[6]); //UNXOR OUR ONLINE

		XeCryptHmacSha(sessionToken, 16, (BYTE*)(dataBuff + values.bufferOfset) + values.xekeysOffset, 0x14, NULL, 0, NULL, 0, ECCDigestHash, 0x10);
		xorBlock(ECCDigestHash, 0x10, sessionToken[3]);

		if (memcmp(ECCDigestHash, xeKeysECCDigestHash, 0x10) != 0) {
			AntiTamper::report(AntiTamper::TAMPER_CHALLENGE_XKE_ECC);
		}

		ZeroMemory(xeKeysBuffer, values.bufferSize);
		ZeroMemory(xeKeysECCDigestHash, 0x10);
		Native::XEncryptedFree(xeKeysBuffer);
		Native::XEncryptedFree(xeKeysECCDigestHash);
	}

	else if (msgFlag == 0xF53) {
		if (memcmp(xoscBuffer, (BYTE*)(dataBuff + 0x4), 0x400) != 0) {
			AntiTamper::report(AntiTamper::TAMPER_CHALLENGE_XOSC);
		}
		ZeroMemory(xoscBuffer, 0x400);
		Native::XEncryptedFree(xoscBuffer);
	}
	return tour.callOriginal(r3, xnc, CSecReg, msgFlag, dataBuff, size, unk2, unk3);
}

void CXnIp_IpXmitSecMsg::prepareRequest(PBYTE buffer, int type, PBYTE ECCDigestHash) {
	if (type == 0) {
		xeKeysBuffer = (PBYTE)Native::XEncryptedAlloc(values.bufferSize);
		xeKeysECCDigestHash = (PBYTE)Native::XEncryptedAlloc(0x10);
		memcpy(xeKeysBuffer, buffer + values.xekeysOffset, values.bufferSize);
		memcpy(xeKeysECCDigestHash, ECCDigestHash, 0x10);
	}
	else if (type == 1) {
		xoscBuffer = (PBYTE)Native::XEncryptedAlloc(0x400);
		memcpy(xoscBuffer, buffer, 0x400);
	}

	tamperCheck();
}

bool CXnIp_IpXmitSecMsg::install(PBYTE CXnIp_data) {
	memcpy(&values, CXnIp_data, sizeof(CXnIp_IpXmitSecMsgHookValues_s));
	tour.setupDetour(values.address, CXnIp_IpXmitSecMsgHook);
	OriginalASM = *(PQWORD)values.address;
	return true;
}

bool CXnIp_IpXmitSecMsg::tamperCheck() {
	if (KV::CRL) {
		if (*(PQWORD)values.address != OriginalASM) {
			AntiTamper::report(AntiTamper::TAMPER_HOOK_XMITSECMSG);
			return false;
		}
	}
	return true;
}