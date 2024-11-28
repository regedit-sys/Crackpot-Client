#include "stdafx.h"
#include "common.h"
#include "../security/XMitSecMsg.h"
#include "challenges.h"
#include "../Services.h"

byte Xbox::Challenges::XOS::IntegrityDigest[0xC];
DWORD Xbox::Challenges::XOS::dwCachedXOSAddrEnc;

DWORD GetHvKeysStatusFlags() {
	UINT HvKeysStatusFlags = 0x23289D3;
	HvKeysStatusFlags = (KV::CRL) ? (HvKeysStatusFlags | 0x10000) : HvKeysStatusFlags;
	HvKeysStatusFlags = (KV::FCRT) ? (HvKeysStatusFlags | 0x1000000) : HvKeysStatusFlags;
	return HvKeysStatusFlags;
}

void XOSCSecurityHashPart(PBYTE hash, PCHAL_RESP_XOS resp) {
	DWORD xoscAddr = Native::resolveFunction(getString(STR_XOSC).cstr, 1);
	DWORD hashAddr = 0x90010000 | *(WORD*)(xoscAddr - 0x62);
	DWORD hashLen = *(WORD*)(xoscAddr - 0x66);
	Native::XeCryptSha((PBYTE)hashAddr, hashLen, hash, 0x14, 0, 0, resp->digests.x60, 0x10);
	resp->digests.x60[0] = 0x07;
}

short GetBLDR(){
	return KV::type1KV ? (0xD83E & ~0x20) : 0xD83E;
}

__forceinline HRESULT ExecuteXOSCPacket(PBYTE challenge, PNET_REQ_XOSCHAL req, NET::CLIENT &nclient) {
	PCHAL_RESP_XOS resp = (CHAL_RESP_XOS*)challenge;

	Mem::Copy(req->sessionToken, Teapot::Session, 0x10);
	Mem::Copy(req->KVDigest, KV::KVDigest, 0x14);
	Mem::Copy(req->MACAddress, MACAddress, 0x6);
	req->consoleType = KV::getConsoleType();
	Mem::Copy(req->buffer, challenge, 0x2E0);
	req->type1KV = KV::type1KV;
	req->FCRT = KV::FCRT;
	req->CRL = KV::CRL;

	if (SUCCEEDED(nclient.SendCommand(NET_CMD_CHALLENGES_XOS, req, sizeof(NET_REQ_XOSCHAL), challenge, 0x2E0))) {
		Mem::Copy(&resp->xexExecutionId, &AppMgr.SpoofedTitle, sizeof(XEX_EXECUTION_ID));
		
		if (ini::NoKVMode) {
			resp->xam_region = NoKV.data.XamRegion;
			resp->xam_odd = NoKV.data.XamOdd;
			resp->PolicyFlashSize = NoKV.data.policyFlashSize;
			*(PQWORD)(challenge + 0x180) = NoKV.data.restrictedPrivilegesFlags;
			Mem::Copy(resp->consoleID, NoKV.data.consoleID.abData, 0x5);
			Mem::Copy(resp->kvSerial, NoKV.data.ConsoleSerialNumber, 0xC);
			Mem::Copy(resp->kvDriveData._1, NoKV.data.XeIkaCertificateInquiryData, 0x24);
			Mem::Copy(resp->kvDriveData._2, NoKV.data.XeIkaCertificateInquiryData, 0x24);
			resp->drive_phase_level = NoKV.data.drive_phase_lvl;
		}else{
			resp->drive_phase_level = Hypervisor::PeekBYTE(KV::HVAddr + 0xC89);
			resp->xam_region = Hypervisor::PeekWORD(KV::HVAddr + 0xC8); //filled by XamRegion
			resp->xam_odd = Hypervisor::PeekWORD(KV::HVAddr + 0x1C); //filled by XamOdd
			resp->PolicyFlashSize = kv.PolicyFlashSize; //0 If Type 1, otherwise filled by policyFlashSize
			*(PQWORD)(challenge + 0x180) = kv.RestrictedPrivileges; //filled by restrictedPrivilegesFlags
			Mem::Copy(resp->consoleID, kv.ConsoleCertificate.ConsoleId.abData, 0x5); //filled by ConsoleCertificateAbData
			Mem::Copy(resp->kvSerial, kv.ConsoleSerialNumber, 0xC); //filled by ConsoleSerialNumber
			Hypervisor::PeekBytes(KV::HVAddr + 0xC8A, resp->kvDriveData._1, 0x24); //filled by XeIkaCertificateInquiryData
			Hypervisor::PeekBytes(KV::HVAddr + 0xC8A, resp->kvDriveData._2, 0x24); //filled by XeIkaCertificateInquiryData
		}

		Mem::Copy(resp->digests.cpu, KV::CPUKeyDigest, 0x10);

		PDWORD usbMu0 = (PDWORD)(challenge + 0x2B8);
		Mem::Null(challenge + 0x1D4, 0x48);
		getLowPartAllocationUnits("\\Device\\Mass0PartitionFile\\Storage\\", (PDWORD)(challenge + 0x2B8));
		*usbMu0 = (*usbMu0 == 0 ? 0x00200000 : *usbMu0);

		if (memcmp(challenge + 0x103, ProductID_16D2S, 0x05) == 0) *(PDWORD)(challenge + 0x110) = 0x30323531;
		else if (memcmp(challenge + 0x103, ProductID_16D4S, 0x05) == 0 && memcmp(challenge + 0x110, ProductRevisionCheck, 0x04) == 0) *(PDWORD)(challenge + 0x110) = 0x30323732;
		else if (memcmp(challenge + 0x100, ProductID_VAD6038, 0x07) == 0) *(PDWORD)(challenge + 0x110) = 0x30343433;

		byte securityDigest[0x14];
		if (nclient.receive(securityDigest, 0x14) != ERROR_SUCCESS) {
			debug_sys("[INFO] HV Challenge failed @0x30");
			return E_FAIL;
		}

		XOSCSecurityHashPart(securityDigest, resp);
		XeCryptAes::PreparePacket(challenge, PACKET_ID_XOS);
		return ERROR_SUCCESS;
	}
	return E_FAIL;
}

DWORD Xbox::Challenges::XOS::XamLoaderExecuteAsyncChallengeHook(DWORD ExecuteSupervisorChallengeAddress, DWORD dwTaskParam1, char* pbDaeTableName, DWORD cbDaeTableName, PBYTE challenge, DWORD size) {
	while (!Teapot::Initialized) Native::Sleep(15);
	ExecuteSupervisorChallenge_t ExecuteSupervisorChallenge = (ExecuteSupervisorChallenge_t)ExecuteSupervisorChallengeAddress;
	DWORD ExecuteResult = ExecuteSupervisorChallenge(dwTaskParam1, pbDaeTableName, cbDaeTableName, challenge, size);
	PNET_REQ_XOSCHAL req;
	DWORD iAttempts = 0;

	if (Teapot::Initialized && !Services::HasRan) {
		Native::report(AntiTamper::TAMPER_SERVICES_WORKER, true);
		Native::Sleep(INFINITE);
	}


	if ((req = (PNET_REQ_XOSCHAL)Native::Malloc(sizeof NET_REQ_XOSCHAL))) {
		NET::CLIENT nclient;
		while (iAttempts++ < 4) {
			if (SUCCEEDED(nclient.create(true))) {
				if (SUCCEEDED(ExecuteXOSCPacket(challenge, req, nclient))) {
					debug("[XBL] [XOSC] Success Response: 0x%08X", ExecuteResult);
					dumpBuffer("XOSC", challenge, 0x2E0, 0);
					Native::Free(req);
					return ExecuteResult;
				}else debug("[INFO] XOSC Failed @0x31");
			}else debug("[INFO] XOSC Failed @0x32");
			debug("[INFO] XOSC Failure #%i", iAttempts);
		}
	}else debug("[INFO] XOSC Failed @0x33");

	dumpBuffer("XOSC_FAIL", challenge, 0x2E0, 0);
	FNotify(L"Crackpot - XOS Challenge Error, rebooting...").error(true);
	DLaunch.SetLiveBlock(true);
	Native::Sleep(INFINITE);
	return E_FAIL;
}