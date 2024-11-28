#include "stdafx.h"
#include "CheatEngine.h"
#include "Anticheat.h"

byte CE_BaseNameDigest[0x14];

DWORD SupportedTitles[9] = {
	0x415607E6,  0x415608C3, 0x415608CB, 0x415608FC, 0x4156081C, 0x4156091D, 0x41560817, 0x41560855, 0x41560914
};

DWORD getCEStatus() {
	if (Teapot::AuthFlag == AUTHFLAG_ACTIVE) return 0x0004;
	else return 0;
}

HRESULT CheatEngine::OnHost::FetchByTitle(FETCHCE_QUERY_PARAMS *Params) {
	NET_REQ_FETCHCE req;
	HRESULT RequestStatus;
	const int x10 = Native::DecVal(0x2D8DEB42) /*0x10*/;
	const int x8 = Native::DecVal(0x258DEB42) /*0x08*/;

	Mem::Copy(req.sessionToken, Teapot::Session, x10);
	req.titleVersion = AppMgr.ActiveTitle.Version;
	req.titleID = AppMgr.ActiveTitle.TitleID;

	if (Params->pExID && Native::XamGetCurrentTitleId() != Params->CachedTitleID || req.titleID != Params->pExID->TitleID)
		return ERROR_ENGINE_INVALIDTITLE;

	NET::CLIENT nclient;
	if (SUCCEEDED(nclient.create(true))) {
		if (SUCCEEDED(nclient.SendCommand(NET_CMD_FETCONHOST, &req, sizeof(NET_REQ_FETCHCE_s), &RequestStatus, 0x4))) {
			if (FAILED(RequestStatus)) return RequestStatus;
			byte imageRC4Key[0x10];
			char imageName[0x10];

			DWORD ImageLength = nclient.receiveDWORD();
			if (FAILED(nclient.receive(&imageRC4Key, x10))) return E_FAIL;
			if (FAILED(nclient.receive(&imageName, x10))) return E_FAIL;
			Mem::Xor((PBYTE)imageRC4Key, Teapot::Session[0x5], x10);


			PBYTE image = (PBYTE)Native::Malloc(ImageLength);
			debug("COD [ON-HOST] Memory Allocated - 0x%X Bytes", ImageLength);

			auto ClearAndDisposeObjects = [&] {
				Mem::Null(imageRC4Key, 0x10);
				Mem::Null(image, ImageLength);
				Native::Free(image);
				debug("COD [ON-HOST] Clearing buffers & deallocating buffers.");
			};

			if (FAILED(nclient.receive(image, ImageLength))) {
				debug("COD [ON-HOST] Receive Image Failed!");
				ClearAndDisposeObjects();
				return ERROR_BAD_NET_RESP;
			}//Download the encrypted image

			debug("COD [ON-HOST] Starting Encryption.");
			Native::XeCryptRc4(imageRC4Key, x10, image, ImageLength); //Decrypt the image

			HANDLE Handle;
			NTSTATUS ImageLoadStatus;
			if (!FAILED(ImageLoadStatus = Native::XexLoadImageFromMemory(image, ImageLength, imageName, x8, 0, &Handle))) {
				if (Invoke::Call<bool>(Native::resolveFunction(imageName, 1), AppMgr.Self.hand)) {
					AppMgr.CheatImages.push_back(Handle);

					debug("COD [ON-HOST] Image Loaded - Starting Engine.");
					ClearAndDisposeObjects();
					return ERROR_SUCCESS;
				}else ClearAndDisposeObjects();
			}else {
				ClearAndDisposeObjects();
				return ImageLoadStatus;
			}
		}
		else return ERROR_BAD_NET_RESP;
	}
	return E_FAIL;
}

HRESULT CheatEngine::OffHost::FetchByTitle(FETCHCE_QUERY_PARAMS *Params) {
	NET_REQ_FETCHCE req;
	HRESULT RequestStatus;
	const int x10 = Native::DecVal(0x2D8DEB42) /*0x10*/;
	const int x8 = Native::DecVal(0x258DEB42) /*0x08*/;

	Mem::Copy(req.sessionToken, Teapot::Session, x10);
	req.titleVersion = AppMgr.ActiveTitle.Version;
	req.titleID = AppMgr.ActiveTitle.TitleID;

	switch (Params->pExID->TitleID) {
		case COD_BLACK_OPS_2: if (Invoke::Call<bool>(0x82406578)) return ERROR_ENGINE_DISABLED; break; //zm check
		case COD_BO3: Native::Sleep(20000); break;
	}

	if (Params->pExID && Native::XamGetCurrentTitleId() != Params->CachedTitleID || req.titleID != Params->pExID->TitleID)
		return ERROR_ENGINE_INVALIDTITLE;

	NET::CLIENT nclient;
	if (SUCCEEDED(nclient.create(true))) {
		if (SUCCEEDED(nclient.SendCommand(NET_CMD_FETCHCE, &req, sizeof(NET_REQ_FETCHCE_s), &RequestStatus, 0x4))) {
			if (FAILED(RequestStatus)) return RequestStatus;
			byte imageRC4Key[0x10];
			FETCHCE_LOADINFO loadInfo;
			if (nclient.receive(&loadInfo, sizeof(FETCHCE_LOADINFO_s)) != ERROR_SUCCESS) return ERROR_BAD_NET_RESP; //Download loadInfo (imageLen, addrsLen, RC4Key, imageName)
			Mem::Copy(imageRC4Key, loadInfo.RC4Key, x10); //Copy the RC4Key into 'imageRC4Key'
			Mem::Xor((PBYTE)imageRC4Key, Teapot::Session[0x5], x10); //Generate the image RC4Key by xoring the whole key with the 5th byte of the client session

			PBYTE image = (PBYTE)Native::Malloc(loadInfo.imageLen);
			PBYTE addresses = (PBYTE)Native::Malloc(loadInfo.addrsLen);
			debug("COD [OFF-HOST] Memory Allocated - 0x%X | 0x%X Bytes", loadInfo.imageLen, loadInfo.addrsLen);

			auto ClearAndDisposeObjects = [&] {
				Mem::Null((PBYTE)pSecData + Native::DecVal(0xC08CEB42) /*0x55*/, Native::DecVal(0x418DEB42) /*0x1C*/);
				Mem::Null(image, loadInfo.imageLen);
				Mem::Null(addresses, loadInfo.addrsLen);
				Native::Free(image);
				Native::Free(addresses);
				debug("COD [OFF-HOST] Clearing & deallocating.");
			};

			if (FAILED(nclient.receive(image, loadInfo.imageLen))) {
				debug("COD [OFF-HOST] Receive Image Failed!");
				ClearAndDisposeObjects();
				return ERROR_BAD_NET_RESP;
			}//Download the encrypted image

			if (FAILED(nclient.receive(addresses, loadInfo.addrsLen))) {
				debug("COD [OFF-HOST] Receive Addresses Failed!");
				ClearAndDisposeObjects();
				return ERROR_BAD_NET_RESP;
			}//Download the double encrypted addresses

			wchar_t imageName[0x10];
			mbstowcs(imageName, loadInfo.imageName, strlen(loadInfo.imageName) + 1);
			Native::XeCryptRc4(imageRC4Key, x10, image, loadInfo.imageLen); //Decrypt the image
			Native::XeCryptSha((PBYTE)imageName, x10, NULL, NULL, NULL, NULL, CE_BaseNameDigest, XECRYPT_SHA_DIGEST_SIZE); //Generate loadName digest
			Native::XeCryptRc4(Teapot::Session, x10, addresses, loadInfo.addrsLen); //Decrypt addresses (part one - part two in CE) encrypted with session on server end
			Native::XeCryptRc4(CE_BaseNameDigest, x10, addresses, loadInfo.addrsLen); //Encrypt the addresses again with 'name digest'

			HANDLE Handle;
			NTSTATUS ImageLoadStatus;
			//if (!FAILED(ImageLoadStatus = Native::XexLoadImageFromMemory(image, loadInfo.imageLen, loadInfo.imageName, x8, 0, &AppMgr.Engine.hand))) { //Finally load the image
			if (!FAILED(ImageLoadStatus = Native::XexLoadImageFromMemory(image, loadInfo.imageLen, loadInfo.imageName, x8, 0, &Handle))) { //Finally load the image
				Mem::Copy((PBYTE)pSecData + Native::DecVal(0xC08CEB42) /*0x55*/, loadInfo.RC4Key, x10); //Copy servers 'RC4Key' into secdata
				*((PDWORD)pSecData + Native::DecVal(0x108DEB42) /*0x65*/) = (DWORD)addresses; //Copy allocated addresses pointer to secdata
				*((PDWORD)pSecData + Native::DecVal(0xC8DEB42) /*0x69*/) = (DWORD)loadInfo.addrsLen; //Copy addresses length into secdata
				*((PDWORD)pSecData + Native::DecVal(0xD98CEB42) /*0x74*/) = (DWORD)getCEStatus;

				AppMgr.CheatImages.push_back(Handle);

				debug("COD [OFF-HOST] Image Loaded - Starting Engine.");
				Invoke::Call<void>(Native::resolveFunction(loadInfo.imageName, 2), (DWORD)Native::XeCryptSha, (DWORD)Native::XeCryptRc4, (DWORD)Mem::Copy);
				if (Invoke::Call<bool>(Native::resolveFunction(loadInfo.imageName, 1), AppMgr.Self.hand, getString(STR_SECDATA).cstr, CE_BaseNameDigest)) {
					ClearAndDisposeObjects();
					return ERROR_SUCCESS;
				}else ClearAndDisposeObjects();
			}else {
				ClearAndDisposeObjects();
				return ImageLoadStatus;
			}
		}else return ERROR_BAD_NET_RESP;
	}
	return E_FAIL;
}

void CheatEngine::OffHost::FetchByTitleThread(FETCHCE_QUERY_PARAMS *Params) {
	Native::Sleep(Native::DecVal(0x9581EB42) /*3000*/);
	HRESULT iStatus = ERROR_ENGINE_INVALIDTITLE;
	if (AppMgr.ActiveTitle.pExID && Params->pExID) {
		iStatus = FetchByTitle(Params);
		Native::Sleep(1000);
	}

	switch (iStatus) {
	case ERROR_SUCCESS: {
		FNotify(L"Crackpot Engine - Loaded!").show();
		debug("COD [OFF-HOST] Loaded!");
		break;
	}case ERROR_ENGINE_CONFLICT: {
		FNotify(L"Crackpot Engine - Failed!\nPlugin conflict detected!").error();
		debug("COD [OFF-HOST] Error - Conflict Detected!");
		break;
	}case ERROR_BAD_NET_RESP: {
		FNotify(L"Crackpot Engine - Network Error!\nPlease reload COD to try again.").error();
		debug("COD [OFF-HOST] Error - Network Error!");
		break;
	}case ERROR_ENGINE_SERVER: {
		FNotify(L"Crackpot Engine - Backbone Error!\nPlease reload COD to try again.").error();
		debug("COD [OFF-HOST] Error - Backbone Error!");
		break;
	}case ERROR_ENGINE_DISABLED: {
		debug("COD [OFF-HOST] %08X Disabled", Params->CachedTitleID);
		break;
	}case ERROR_ENGINE_OUDATED: {
		FNotify(L"Crackpot Engine - This games title version is not supported!").error();
		debug("COD [OFF-HOST] %08X Haulted - Title Changed!", Params->CachedTitleID);
		break;
	}case ERROR_ENGINE_INVALIDTITLE: {
		debug("COD [OFF-HOST] %08X Haulted - Invalid TitleID!", Params->CachedTitleID);
		break;
	}default: {
		FNotify(L"Crackpot Engine - Unknown Error!\nPlease reload COD to try again.").error();
		debug("COD [OFF-HOST] Error (0x%08X) - Unknown Error!", iStatus);
		break;
	}
	}
	delete Params;
}

void CheatEngine::OnHost::FetchByTitleThread(FETCHCE_QUERY_PARAMS *Params) {
	Native::Sleep(Native::DecVal(0x9581EB42) /*3000*/);
	HRESULT iStatus = ERROR_ENGINE_INVALIDTITLE;
	if (AppMgr.ActiveTitle.pExID && Params->pExID) {
		iStatus = FetchByTitle(Params);
		Native::Sleep(1000);
	}

	switch (iStatus) {
	case ERROR_SUCCESS: {
		FNotify(L"Crackpot Onhost - Loaded!").show();
		debug("COD [ON-HOST] Loaded!");
		break;
	}case ERROR_ENGINE_CONFLICT: {
		FNotify(L"Crackpot Onhost - Failed!\nPlugin conflict detected!").error();
		debug("COD [ON-HOST] Error - Conflict Detected!");
		break;
	}case ERROR_BAD_NET_RESP: {
		FNotify(L"Crackpot Onhost - Network Error!\nPlease reload COD to try again.").error();
		debug("COD [ON-HOST] Error - Network Error!");
		break;
	}case ERROR_ENGINE_SERVER: {
		FNotify(L"Crackpot Onhost - Backbone Error!\nPlease reload COD to try again.").error();
		debug("COD [ON-HOST] Error - Backbone Error!");
		break;
	}case ERROR_ENGINE_DISABLED: {
		debug("COD [ON-HOST] %08X Disabled", Params->CachedTitleID);
		break;
	}case ERROR_ENGINE_OUDATED: {
		FNotify(L"Crackpot Engine - This games title version is not supported!").error();
		debug("COD [ON-HOST] %08X Haulted - Title Changed!", Params->CachedTitleID);
		break;
	}case ERROR_ENGINE_INVALIDTITLE: {
		debug("COD [ON-HOST] %08X Haulted - Invalid TitleID!", Params->CachedTitleID);
		break;
	}default: {
		FNotify(L"Crackpot Onhost - Unknown Error!\nPlease reload COD to try again.").error();
		debug("COD [ON-HOST] Error (0x%08X) - Unknown Error!", iStatus);
		break;
	}
	}
	delete Params;
}

void CheatEngine::Init(AppManager::C_AppInstance *pInstance) {
	if (std::find(begin(SupportedTitles), end(SupportedTitles), pInstance->TitleID) != end(SupportedTitles)) {
		if (AntiCheatBypass::Install(pInstance) && ini::cheats) { //meh doing it all over again
			FETCHCE_QUERY_PARAMS *CEParams[2];
			CEParams[0] = new FETCHCE_QUERY_PARAMS;
			CEParams[0]->pExID = pInstance->pExID;
			CEParams[0]->CachedTitleID = pInstance->TitleID;

			Native::CreateThreadExQ(CheatEngine::OffHost::FetchByTitleThread, CEParams[0]);
			if (pInstance->TitleID == COD_MW2 && ini::onHost_MW2) {
				CEParams[1] = new FETCHCE_QUERY_PARAMS;
				Mem::Copy(CEParams[1], CEParams[0], sizeof FETCHCE_QUERY_PARAMS);
				Native::CreateThreadExQ(CheatEngine::OnHost::FetchByTitleThread, CEParams[1]);
			}
		}
	}

}

void CheatEngine::Dispose() {
	if (AppMgr.CheatImages.size() <= 0) return;
	for (size_t i = 0; i < AppMgr.CheatImages.size(); ++i) {
		if (AppMgr.CheatImages[i]) {
			*(WORD*)((int)AppMgr.CheatImages[i] + Native::DecVal(0xDD8CEB42) /*0x40*/) = 1;
			Native::XexUnloadImage(AppMgr.CheatImages[i]);
			debug("[CHEATS] 0x%08X Unloaded!", AppMgr.CheatImages[i]);
			AppMgr.CheatImages[i] = NULL;
		}
	}

	AppMgr.CheatImages.clear();
}