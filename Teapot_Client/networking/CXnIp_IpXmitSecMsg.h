#pragma once

struct CXnIp_IpXmitSecMsgHookValues_s {
	DWORD address;
	DWORD desiredMsg;
	DWORD desiredSize;
	BYTE bufferOfset;
	BYTE bufferSize;
	BYTE xekeysOffset;
};

namespace CXnIp_IpXmitSecMsg {
	extern CXnIp_IpXmitSecMsgHookValues_s values;
	extern detour<void> tour;
	extern byte *xeKeysBuffer;
	extern byte *xeKeysECCDigestHash;
	extern byte *xoscBuffer;
	void CXnIp_IpXmitSecMsgHook(int r3, XNCALLER_TYPE xnc, int* CSecReg, unsigned __int16 unk, byte* dataBuff, unsigned int size, void * unk2, unsigned int unk3);
	void prepareRequest(PBYTE buffer, int type, PBYTE ECCDigestHash = nullptr);
	bool install(PBYTE CXnIp_data);
	bool tamperCheck();
}