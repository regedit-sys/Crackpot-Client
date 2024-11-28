#include "stdafx.h"
#include "common.h"
#include "util/XeCrypt.h"
#include "networking/CXnIp_IpXmitSecMsg.h"
#include "Teapot.h"
#include "../Updater.h"
#include "../security/XMitSecMsg.h"
#include "../xbox/XamExecuteChallenge.h"
#include "../xbox/challenges.h"


wchar_t welcomeMsgBuffer[100];
bool presence_ran = false;
bool NET::isSessionActive = false;

namespace registration {
	byte *registrationKey = 0x00;
	DWORD CPUShaDigestPtr;
	DWORD RCTeaSeed;
	bool getRegKey(DWORD regKeySize, NET::CLIENT &nclient) {
		registrationKey = (PBYTE)Native::Malloc(regKeySize);
		if (nclient.receive(registrationKey, regKeySize) == ERROR_SUCCESS) {
			if (nclient.receive(&RCTeaSeed, 0x4) != ERROR_SUCCESS) return false;
			if (nclient.receive(&CPUShaDigestPtr, 0x4) != ERROR_SUCCESS) return false;
		}else return false;
		return true;
	}
}

bool NET::GET_CRYTO_KEY(byte *key) {
	NET::CLIENT nclient;
	if (SUCCEEDED(nclient.create(true))) {
		byte CPUKey[0x10];
		byte EncryptionKey[0x10];
		Native::HvGetCPUKey(CPUKey);
		if (SUCCEEDED(nclient.SendCommand(0x00000AF1, &CPUKey, 0x10, EncryptionKey, 0x10))) {
			debug("WE GOT OUR ENCRYPTION KEY! %s", BytesToHexStr(EncryptionKey, 0x10).c_str());
			return true;
		}else debug("WE COULD NOT GET OUR ENCRYPTION KEY! FAILED!");
	}
	return false;
}


int NET::TOKEN(char *token, bool redeem){
	NET_REQ_TOKEN req;
	int status = -2;
	Native::HvGetCPUKey(req.cpuKey);
	strncpy(req.token, token, 21);
	req.redeem = redeem;

	NET::CLIENT nclient;
	if (SUCCEEDED(nclient.create(true))) {
		nclient.SendCommand(NET_CMD_TOKEN, &req, sizeof(NET_REQ_TOKEN), &status, 4);
	}
	return status;
}

int NET::REGISTER(char *email, char *name, bool confirm) {
	DWORD x10 = Native::DecVal(0x2D8DEB42) /*0x10*/;
	NET_REQ_REGISTER req;
	int status = -1;

	Native::HvGetCPUKey(req.cpuKey);
	Mem::Copy(req.cpuHash, (PBYTE)registration::CPUShaDigestPtr, x10);
	Mem::Copy(req.regKey, registration::registrationKey, x10);
	Mem::Xor(req.regKey, req.cpuKey[Native::DecVal(0x308DEB42) /*0x05*/] ^ Native::DecVal(0x708CEB42) /*0xC5*/, x10);
	Native::XeCryptSha(NULL, 0, req.regKey, x10, req.cpuKey, x10, req.regKey, x10);
	Native::RCTea((PBYTE)registration::CPUShaDigestPtr, x10, req.regKey, x10, registration::RCTeaSeed);
	req.confirm = confirm;
	strncpy(req.name, name, 0xF);
	strncpy(req.email, email, 0x20);

	NET::CLIENT nclient;
	if (SUCCEEDED(nclient.create(true))) {
		nclient.SendCommand(NET_CMD_REGISTER, &req, sizeof(NET_REQ_REGISTER), &status, 4);
	}
	return status;
}

HRESULT NET::LOGON_PATCH_XAM(NET::CLIENT &nclient) {
	debug("[NET] Logon_Patch_Xam: Pending...");

	DWORD xamPatchKeyLen = 0;
	DWORD xamPatchLen = 0;
	byte *xamPatchKey = nullptr;
	byte *xamPatches = nullptr;

	if ((xamPatchKeyLen = nclient.receiveDWORD()) <= 0) {
		debug("[NET] Logon_Patch_Xam: Invalid payload key length.");
		return E_FAIL;
	}

	if (!(xamPatchKey = (PBYTE)Native::Malloc(xamPatchKeyLen))) {
		debug("[NET] Logon_Patch_Xam: Failed to allocate key memory.");
		nclient.close();
		return E_FAIL;
	}

	if (FAILED(nclient.receive(xamPatchKey, xamPatchKeyLen))) {
		debug("[NET] Logon_Patch_Xam: Failed to receive the payload key.");
		return E_FAIL;
	}

	if ((xamPatchLen = nclient.receiveDWORD()) < 0xC) {
		debug("[NET] Logon_Patch_Xam: Invalid payload length.");
		return E_FAIL;
	}

	if (!(xamPatches = (PBYTE)Native::Malloc(xamPatchLen))) {
		debug("[NET] Logon_Patch_Xam: Failed to allocate payload memory.");
		nclient.close();
		return E_FAIL;
	}

	nclient.receive(xamPatches, xamPatchLen);
	Native::RC4Custom(xamPatchKey, xamPatchKeyLen, xamPatches, xamPatchLen);
	if (!Teapot::Devkit) ApplyPatches(xamPatches);

	Mem::Null(xamPatches, xamPatchLen);
	Mem::Null(xamPatchKey, xamPatchKeyLen);
	Native::Free(xamPatches);
	Native::Free(xamPatchKey);

	nclient.close();
	debug("[NET] Logon_Patch_Xam: Success!");
	return ERROR_SUCCESS;
}

HRESULT NET::LOGON() {
	debug("[NET] Logon: Pending...");
	const DWORD x5 = Native::DecVal(0x308DEB42) /*0x05*/;
	NET_REQ_AUTHENTICATION req;
	DWORD status;

	Native::HvGetCPUKey(req.consoleCpuKey);
	Native::getConsoleHash(req.consoleHash);

	Mem::Copy(req.KVDigest, KV::KVDigest + x5, x5);
	req.NoKVMode = ini::NoKVMode;
	req.Devkit = Teapot::Devkit;
	memcpy(req.OriginalDVDKey, Teapot::CachedOriginalDVDKey, 0x10);
	
	NET::CLIENT nclient;
	if (SUCCEEDED(nclient.create(true))) {
		if (SUCCEEDED(nclient.SendCommand(NET_CMD_AUTHENTICATE, &req, sizeof(NET_REQ_AUTHENTICATION_s), &status, 4))) {
			if (status != RESP_AUTHENTICATION_BANNED && status != RESP_AUTHENTICATION_CONSOLEHASHMISMATCH) nclient.receive(&sData, sizeof(NET_RESP_FETCHSDATA_s));
			if (status == RESP_AUTHENTICATION_SUCCESS || status == RESP_AUTHENTICATION_FREEMODE) {
				Teapot::Freemode = (status == RESP_AUTHENTICATION_FREEMODE);
				nclient.receive(Teapot::Session, Native::DecVal(0x2D8DEB42) /*0x10*/);
				if (FAILED(LOGON_PATCH_XAM(nclient))) return E_FAIL;
				if (FAILED(Teapot::SetModuleHash())) return E_FAIL;
				if (FAILED(NET::FETCHCDATA())) return E_FAIL;
				if (FAILED(NET::PRESENCE(true))) {
					FNotify(L"Crackpot - Internal Error!").error(true);
					return E_FAIL;
				}
				std::string timestamp = GetTimeAsString();
				debug("[NET] Logon: Session Started %s", timestamp.cstr);
				NET::isSessionActive = true;
				return ERROR_SUCCESS;
			}
			else if (status == RESP_AUTHENTICATION_FAILURE) {
				if (!registration::getRegKey(status + 0x5, nclient)) {
					FNotify(L"Crackpot - Error [AUTH_FAIL] consult admin for fix, rebooting...").error(true);
					return E_FAIL;
				}
				Teapot::AuthFlag = AUTHFLAG_NOEXIST;
				debug("Fuck 1");
				return E_FAIL;
			}
			else if (status == RESP_AUTHENTICATION_EXPIRED) {
				Teapot::AuthFlag = AUTHFLAG_EXPIRED;
				debug("Fuck 2");
				return E_FAIL;
			}
			else if (status == RESP_AUTHENTICATION_CONSOLEHASHMISMATCH) {
				FNotify(L"Crackpot - Error [MISMATCH] make a ticket in discord, rebooting...").error(true);
				debug("[NET] Logon: Console Hash Mismatch!");
				return E_FAIL;
			}
			else if (status = RESP_AUTHENTICATION_BANNED) {
				FNotify(L"Crackpot - Error [BEANED] you've been beaned, rebooting...").error(true);
				debug("[NET] Logon: Banned!");
				return E_FAIL;
			}
		} 
	}
	debug("NET] Logon: Cannot Connect...");
	return E_FAIL;
}

HRESULT NET::FETCHCDATA() {
	u32 status = 0;
	NET_REQ_FETCHCDATA req;

	debug("[NET] GetCData: Pending...");
	Mem::Copy(req.sessionToken, Teapot::Session, Native::DecVal(0x2D8DEB42) /*0x10*/);
	Mem::Copy(req.executableHash, Teapot::ImageDigest, Native::DecVal(0x2D8DEB42) /*0x10*/);
	if (ini::NoKVMode) {
		HRESULT status;
		if (FAILED(status = Hypervisor::PeekBytes(KV::HVAddr + Native::DecVal(0x1D8CEB42) /*0x100*/, req.DVDKey, Native::DecVal(0x2D8DEB42) /*0x10*/))) {
			debug("[NET] FETCHCDATA | Hypervisor::PeekBytes -> 0x%llu (0x%08X) Failed!", KV::HVAddr + Native::DecVal(0x1D8CEB42), status); 
			Native::HalReturnToFirmware(HalRebootRoutine);
		}
	}
	
	debug("[NET] GetCData: Downloading client data...");
	NET::CLIENT nclient;
	if (SUCCEEDED(nclient.create(true))) {
		if (SUCCEEDED(nclient.SendCommand(NET_CMD_FETCHCDATA, &req, sizeof(NET_REQ_FETCHCDATA), &status, 4))) {
			if (status == RESP_AUTHENTICATION_UPDATE) {
				debug("[NET] GetCData: Update Available!");
				Teapot::AuthFlag = AUTHFLAG_UPDATING;
				Teapot::C_UPDATER Updater(true);
				Updater.InstallLatest();
				return E_FAIL;
			}

			if (ini::NoKVMode) {
				if (FAILED(NoKV.ini(nclient))) {
					debug("[NET] GetCData: NoKV_S::INIT Failed!");
					return E_FAIL;
				}
			}

			int size = (sizeof(cData_s) - sizeof(NET_RESP_FETCHCDATA_KVINFO_s) - sizeof(NET_PREFERENCES));
			if (SUCCEEDED(nclient.receive(&cData, size))) {
				if (cData.xNotify.bwelcome) {
					if (status == RESP_AUTHENTICATION_FREEMODE) swprintf(welcomeMsgBuffer, L"Welcome to Crackpot, %hs!", cData.name);
					else swprintf(welcomeMsgBuffer, L"Welcome, %hs!", cData.name);
					FNotify(welcomeMsgBuffer).show();
				}

				DWORD XOSCHookAddr;
				DWORD x32 = Native::DecVal(0x3D8DEB42) /*0x20*/;
				byte *ShellCodeKey = (byte*)Native::XEncryptedAlloc(x32);

				if (FAILED(nclient.receive(&XOSCHookAddr, 0x4))) return E_FAIL;

				if (FAILED(nclient.receive(ShellCodeKey, x32))) {
					debug("ERROR_XEC_INSTALLAESHOOK FAILED");
					return E_FAIL;
				}

				if (FAILED(XeCryptAes::install(ShellCodeKey))) {
					debug("ERROR_XEC_INSTALLAESHOOK FAILED");
					return E_FAIL;
				}

				Native::PatchInJump((PDWORD)XOSCHookAddr, (DWORD)Xbox::Challenges::XOS::XamLoaderExecuteAsyncChallengeHook, false);

				Xbox::Challenges::XOS::CacheXOSAddress(XOSCHookAddr);
				Xbox::Challenges::XOS::InitIntegrityDigest();

				Mem::Null(&XOSCHookAddr, 0x4);
				Mem::Null(ShellCodeKey, x32);
				Native::XEncryptedFree(ShellCodeKey);

				debug("[NET] GetCData: Success!");
				Teapot::AuthFlag = AUTHFLAG_ACTIVE;
				return ERROR_SUCCESS;
			}
		}

	}
	debug("[NET] GetCData: Failed!");
	return E_FAIL;
}

HRESULT NET::PRESENCE(bool bStartup) {
	DWORD pdwStatus;
	u32 status;
	char gamerTag[0x10];
	PNET_REQ_PRESENCE req;
	
	if (!(req = (PNET_REQ_PRESENCE)Native::Malloc(sizeof NET_REQ_PRESENCE))) {
		debug("Malloc Failed Presence");
		return E_FAIL;
	}

	Native::XUserGetName(0, gamerTag, Native::DecVal(0x2D8DEB42) /*0x10*/);
	Mem::Copy(req->sessionToken, Teapot::Session, Native::DecVal(0x2D8DEB42) /*0x10*/);
	req->titleID = Native::XamGetCurrentTitleId();
	req->userSigninState = (DWORD)Native::XUserGetSigninState(0);
	strncpy(req->gamerTag, gamerTag, Native::DecVal(0x2D8DEB42) /*0x10*/);
	Mem::Copy(req->executableHash, Teapot::ImageDigest, Native::DecVal(0x2D8DEB42) /*0x10*/);
	Native::XNetLogonGetExtendedStatus(&pdwStatus, &req->zStatus);
	req->CRL = KV::CRL;
	req->isDevkit = Teapot::Devkit;
	req->PendingUpdate = Teapot::UpdateNotified;

	NET::CLIENT nclient;
	if (SUCCEEDED(nclient.create(bStartup))) {
		if (SUCCEEDED(nclient.SendCommand(NET_CMD_PRESENCE, req, sizeof(NET_REQ_PRESENCE), &status, 4))) {
			Native::Free(req);
			if (status == RESP_AUTHENTICATION_SUCCESS || status == RESP_AUTHENTICATION_FREEMODE || status == RESP_AUTHENTICATION_TIMESTART /*this is all new*/) {
				Teapot::Freemode = (status == RESP_AUTHENTICATION_FREEMODE);
				NET_RESP_FETCHCDATA_KVINFO_s kvTime;
				NET_RESP_FETCHTIME_s remainingUserTime;

				if (nclient.receive(&kvTime, sizeof(NET_RESP_FETCHCDATA_KVINFO_s)) == ERROR_SUCCESS) {
					Mem::Copy(&cData.kvinfo, &kvTime, sizeof NET_RESP_FETCHCDATA_KVINFO_s);
				}

				if (nclient.receive(&remainingUserTime, sizeof(NET_RESP_FETCHTIME_s)) == ERROR_SUCCESS) {
					Mem::Copy(&remainingTime, &remainingUserTime, sizeof NET_RESP_FETCHTIME_s);
				}

				if (!presence_ran) {
					nclient.receive(&XUIColorPref, sizeof(XUIColorPreferences));
				}

				if (status == RESP_AUTHENTICATION_TIMESTART) { //this is all new
					FNotify(L"Crackpot - New day started!").show();
				}
			}
			else if (status == RESP_AUTHENTICATION_FAILURE) {
				FNotify(L"Authentication failed, rebooting...").error(true);
				debug("[NET] Presence: Auth Failed!");
				return E_FAIL;
			}
			else if (status == RESP_AUTHENTICATION_BANNED) {
				FNotify(L"Crackpot - Guess what? You've been banned! Cool huh?").error(true);
				debug("[NET] Presence: Auth Banned!");
				return E_FAIL;
			}
			else if (status == RESP_AUTHENTICATION_EXPIRED) {
				Native::CreateThreadExQ(UI_ExpiredMsg);
				debug("[NET] Presence: Auth Expired!");
				return E_FAIL;
			}
			else if (status == RESP_AUTHENTICATION_UPDATE) {
				debug("[NET] Presence: Update!");
				nclient.close();
				Teapot::UpdateNotified = true;
				Teapot::C_UPDATER Updater;
				Updater.InstallLatest();
			}
			else {
				FNotify(L"Status unkown - rebooting to be safe").error(true);
				debug("[NET] Presence: Auth Unknown Status!");
				return E_FAIL;
			}

			Teapot::IPAddrIntrnl.ina.S_un.S_addr = 0;
			XNetGetTitleXnAddr(&Teapot::IPAddrIntrnl);

			if (!presence_ran)
				debug("[NET] Presence: Success!");

			presence_ran = true;
			AsyncPresenceEvent.Reset();
			return ERROR_SUCCESS;
		}else FNotify(L"Crackpot - Network Error");
	}
	
	free(req);
	AsyncPresenceEvent.Reset();
	debug("PRESENCE ERROR!");
	return E_FAIL;
}

HRESULT NET::PREFERENCES_GET(NET_PREFERENCES &preferences) {
	bool status;
	byte session[0x10];
	const DWORD x10 = Native::DecVal(0x2D8DEB42) /*0x10*/;
	Mem::Copy(session, Teapot::Session, x10);

	NET::CLIENT nclient;
	if (SUCCEEDED(nclient.create(false, false))) {
		if (FAILED(nclient.SendCommand(NET_CMD_PREFERENCES_GET, session, x10, &status, 0x1))) return E_FAIL;
		if (status) {
			if (SUCCEEDED(nclient.receive(&preferences, sizeof NET_PREFERENCES))) return ERROR_SUCCESS;
		}
	}
	return E_FAIL;
}

HRESULT NET::PREFERENCES_SET(NET_PREFERENCES pref) {
	NET_REQ_PREFERENCES req;
	Mem::Copy(req.sessionToken, Teapot::Session, Native::DecVal(0x2D8DEB42) /*0x10*/);
	Mem::Copy(&req.preferences, &pref, sizeof NET_PREFERENCES);

	NET::CLIENT nclient;
	if (SUCCEEDED(nclient.create(false)))
		if (SUCCEEDED(nclient.SendCommand(NET_CMD_PREFERENCES_SET, &req, sizeof NET_REQ_PREFERENCES))) return ERROR_SUCCESS;
	return E_FAIL;
}