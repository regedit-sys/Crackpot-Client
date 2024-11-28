#include "stdafx.h"
#include "XamExecuteChallenge.h"
#include "challenges.h"
#include "../security/XMitSecMsg.h"
#include "../Services.h"

#define ERROR_XEC_RECV_RESP        0xC3000001
#define ERROR_XEC_RECV_KEY         0xC3000002
#define ERROR_XEC_RECV_DIGEST      0xC3000003
#define ERROR_XEC_RECV_XOSCADDR    0xC3000004
#define ERROR_XEC_RECV_AESHOOKADDR 0xC3000005
#define ERROR_XEC_SANITY_HEADER    0xC3000006
#define ERROR_XEC_INSTALLAESHOOK   0xC3000007

detour<NTSTATUS> XamExecuteChallengeStub;
byte Xbox::Challenges::Xam::IntegrityDigest[0xC];
DWORD ChallengeCount = 0;
bool ChalRan = false;

byte SupportedChallengeHash[0x14] = {
	0x0E, 0xA5, 0xDD, 0x7C, 0x32, 0x13, 0xEA, 0x72, 0x93, 0x02,
	0x3E, 0x25, 0x73, 0xC1, 0xEA, 0xD9, 0x6F, 0xDF, 0xC6, 0x36
};

#pragma pack(push, 1)
typedef struct NET_REQ_XEC_s {
	byte SessionToken[0x10];
	byte Salt[0x10];
	byte KVCpuDigest[0x14];
	int ConsoleType;
	DWORD UpdateSequence;
	DWORD TickCount;
	bool CRL;
	bool FCRT;
	bool Type1KV;
}NET_REQ_XEC, *pNET_REQ_XEC;
#pragma pack(pop)

__forceinline void DoPayloadIntegrity(PBYTE Payload, DWORD dwPayload) {
	byte CurrentChallengeHash[0x14];

	Native::XeCryptSha(Payload, Native::DecVal(0x8D89EB42) /*0x3F0*/, NULL, NULL, NULL, NULL, CurrentChallengeHash, 0x14);
	if (memcmp(CurrentChallengeHash, SupportedChallengeHash, 0x14) != 0) {
		Native::report(AntiTamper::INTEG_CHALLENGE_HVPAYLOAD, false, Payload, dwPayload);
		FNotify(L"Crackpot - 0x13EC Mismatch!\nRebooting for your protection..").error(true);
		debug("[XBL] HVCP Hash does NOT Match!");
		DLaunch.SetLiveBlock(true);
		Native::Sleep(INFINITE);
	}
}

__forceinline NTSTATUS Xbox::Challenges::Xam::ExecuteRequest(XAM_CHAL_BUFFER *Response, PBYTE Salt) {
	DWORD Status;
	NET_REQ_XEC Request;
	NET::CLIENT nclient;
	memcpy(Request.SessionToken, Teapot::Session, 0x10);
	memcpy(Request.Salt, Salt, 0x10);
	memcpy(Request.KVCpuDigest, KV::CPUKeyDigest, 0x14);
	Request.ConsoleType = KV::getConsoleType();
	Request.UpdateSequence = KV::updateSequence;
	Request.TickCount = Native::keTimeStampBundle()->TickCount;
	Request.CRL = KV::CRL;
	Request.FCRT = KV::FCRT;
	Request.Type1KV = KV::type1KV;

	byte IntegrityDigest[0x10];
	byte EncryptionKey[0x14];

	if (FAILED((Status = nclient.create(true, false)))) return Status;
	if (SUCCEEDED(nclient.SendCommand(NET_CMD_CHALLENGES_XAM_NEW, &Request, sizeof(NET_REQ_XEC), &Status, 4))) {
		if (Status == RESP_AUTHENTICATION_EXPIRED) {
			FNotify(L"Crackpot - Time Expired!\nRebooting...").error(true);
			DLaunch.SetLiveBlock(true);
			Sleep(INFINITE);
		}
	}

	if (Status == RESP_AUTHENTICATION_SUCCESS || Status == RESP_AUTHENTICATION_FREEMODE || Status == RESP_AUTHENTICATION_TIMESTART) {
		if (nclient.receiveDWORD() == NET_STATUS_FAILURE) return E_FAIL;
		if (FAILED(nclient.receive(Response, 0x100))) return ERROR_XEC_RECV_RESP;
		if (FAILED(nclient.receive(IntegrityDigest, 0x10))) return ERROR_XEC_RECV_DIGEST;
		if (FAILED(nclient.receive(EncryptionKey, 0x14))) return ERROR_XEC_RECV_KEY;

		if (Status == RESP_AUTHENTICATION_TIMESTART) FNotify(L"Crackpot - New day started!").show();
		XeCryptAes::PreparePacket(Response, PACKET_ID_XKE, IntegrityDigest, EncryptionKey);

		if (!KV::CRL) {
			if (cData.xNotify.bxam) FNotify(sData.notifyHVChal).show();
			KV::CRL = true;
		}

		cData.XKECount++;
		//ChallengeCount++;
		//ChalRan = true;
		return S_OK;
	}

	return E_FAIL;
}

NTSTATUS Xbox::Challenges::Xam::XamExecuteChallengeHook(PBYTE Payload, DWORD dwPage, PBYTE Salt, DWORD dwSalt, XAM_CHAL_BUFFER *Response, DWORD dwResponce) {
	const DWORD x1000 = Native::DecVal(0x1D7DEB42) /*0x1000*/;
	const DWORD x100 = Native::DecVal(0x1D8CEB42) /*0x100*/;
	NTSTATUS Status = STATUS_INVALID_PARAMETER;

	DWORD dwPayload = ((dwPage + 0xFFF) & Native::DecVal(0x1D9DEB42) /*0xFFFFF000*/);
	if (dwPayload >= x1000 && dwPage > 0 && dwResponce >= x100) {
		if (Teapot::Initialized && !Services::HasRan) {
			Native::report(AntiTamper::TAMPER_SERVICES_WORKER, true);
			Native::Sleep(INFINITE);
		}

		Mem::Null(Response, dwResponce);
		DoPayloadIntegrity(Payload, dwPayload);
		for (int i = 0; i < 3; i++) {
			if (!IS_ERROR(Status = ExecuteRequest(Response, Salt))) break;
		}
	}

	if (IS_ERROR(Status)) {
		wchar_t buffer[65];
		debug("[XBL] XamExecuteChallenge - Returned: 0x%08X", Status);
		swprintf(buffer, L"Crackpot - Challenge Error (0x%08X)\nPlease Report!", Status);
		FNotify(buffer).error(true);
		Native::Sleep(INFINITE);
	}

	dumpBuffer("XamExecuteChallenge", Response, 0x100, 0);
	debug("[XBL] XamExecuteChallenge - Page Size: 0x%08X | %i bytes", dwPage, dwPage);
	debug("[XBL] XamExecuteChallenge - Completion Status: 0x%08X", Status);

	return S_OK;
}