#include "stdafx.h"
#include "Updater.h"

using namespace Teapot;

C_UPDATER::C_UPDATER(bool Force) {
	this->Pkg.Image = nullptr;
	this->Force = Force;
	this->CachedImageSize = 0;
	this->AllocationSuccess = false;
	this->WasError = false;
}

C_UPDATER::~C_UPDATER() {
	if (this->Pkg.Image) {
		debug("[UPDATER] Deconstructor: Image Data Ptr is valid and has been free'd!");
		Native::Free(this->Pkg.Image);
	}
}

HRESULT C_UPDATER::VerifyIntegrity(INTEGRITY_MODE Mode) {
	byte DataIntegrityHash[0x14];

	switch (Mode) {
	case MODE_PREWRITE: {
		Native::XeCryptSha(0, 0, 0, 0, this->Pkg.Image, this->Pkg.ImageSize, DataIntegrityHash, 0x14);
		break;
	}case MODE_POSTWRITE: {
		std::string Path = GetSelfPath();
		MemoryBuffer mbClientBin;
		if (cReadFile(Path.cstr, mbClientBin)) {
			Native::XeCryptSha(0, 0, 0, 0, mbClientBin.GetData(), mbClientBin.GetDataLength(), DataIntegrityHash, 0x14);
			break;
		}
		debug("[UPDATER] VerifyIntegrity: Failed to open %s", Path.cstr);
		return ERROR_UPDATER_INTEG_OPENFAIL;
	}default:
		debug("[UPDATER] VerifyIntegrity: Unknown Mode (%x)", Mode);
		return ERROR_UPDATER_INTEG_UNKMODE;
		break;
	}

	if (memcmp(DataIntegrityHash, Pkg.Digest, 0x14) != 0) {
		return ERROR_UPDATER_INTEG_FAILED + Mode; //(0x9 & 0xA is reserved)
	}
	return ERROR_SUCCESS;
}

HRESULT C_UPDATER::SendCMD() {
	byte SessionCopy[0x10];
	size_t RequestSize = offsetof(struct UPDATE_RESPONSE, Image);

	this->Client.close();
	Mem::Copy(SessionCopy, Teapot::Session, 0x10);

	if (FAILED(this->Client.create(true))) return ERROR_UPDATER_REQ_CREATE;
	if (FAILED(this->Client.SendCommand(NET_CMD_GETUPDATE, SessionCopy, 0x10, &Pkg, RequestSize))) return ERROR_UPDATER_REQ_CMD;
	if (Pkg.ImageSize < AppMgr.Self.pLDR->SizeOfFullImage / 3) return ERROR_UPDATER_INVALID_SIZE;
	if (Pkg.ImageSize > AppMgr.Self.pLDR->SizeOfFullImage * 2) return ERROR_UPDATER_INVALID_SIZE;
	this->CachedImageSize = Pkg.ImageSize;
	return ERROR_SUCCESS;
}

HRESULT C_UPDATER::DownloadImage() {
	if (FAILED(this->Client.receive(Pkg.Image, Pkg.ImageSize))) {
		debug("[UPDATER] Allocate: Unable to download the image.");
		return ERROR_UPDATER_RECV_IMAGE;
	}
	return ERROR_SUCCESS;
}

HRESULT C_UPDATER::Allocate() {
	if (this->WasError && this->AllocationSuccess && this->CachedImageSize != Pkg.ImageSize) {
		debug("[UPDATER] Allocate: The cached image size does not match the current.");
		this->AllocationSuccess = false;
		Native::Free(Pkg.Image);
	}

	if (!this->AllocationSuccess) {
		this->Pkg.Image = (BYTE*)Native::Malloc(Pkg.ImageSize);
		if (!this->Pkg.Image) {
			debug("[UPDATER] Allocate: Not enough free memory.");
			return ERROR_UPDATER_ALLOCATE;
		}
		this->AllocationSuccess = true;
	}
	else debug("[UPDATER] Allocate: Memory is already allocated. Block being re-used.");
	return ERROR_SUCCESS;
}

HRESULT C_UPDATER::WriteUpdate() {
	std::string Path = GetSelfPath();
	if (!Native::CWriteFile(Path.cstr, Pkg.Image, Pkg.ImageSize)) {
		debug("[UPDATER] WriteUpdate: Unable to write image to %s", Path.cstr);
		return ERROR_UPDATER_WRITEIMAGE;
	}
	return ERROR_SUCCESS;
}

HRESULT C_UPDATER::Query() {
	HRESULT Status = ERROR_SUCCESS;

	if (FAILED(Status = this->SendCMD())) {
		return Status;
	}

	if (FAILED(Status = this->Allocate())) {
		return Status;
	}

	if (FAILED(Status = this->DownloadImage())) {
		return Status;
	}

	if (FAILED(Status = this->VerifyIntegrity(MODE_PREWRITE))) {
		return Status;
	}

	if (FAILED(Status = this->WriteUpdate())) {
		return Status;
	}

	if (FAILED(Status = this->VerifyIntegrity(MODE_POSTWRITE))) {
		return Status;
	}

	debug("[UPDATER] Image Digest: %s", BytesToHexStr(Pkg.Digest, 0x10).cstr);
	debug("[UPDATER] Image Length: 0x%08X | (%i Bytes)", Pkg.ImageSize, Pkg.ImageSize);
	debug("[UPDATER] Critical: %x", Pkg.Critical);
	return ERROR_SUCCESS;
}

void C_UPDATER::InstallLatest() {
	HRESULT Status = E_FAIL;

	for (int i = 0; i < 3; i++) {
		if (!IS_ERROR(Status = this->Query())) break;
		debug("[UPDATER] Attempt Failed: #%i", i);
		Native::Sleep(1500);
		this->WasError = true;
	}

	if (SUCCEEDED(Status)) {
		std::wstring NotifyMsg = L"Crackpot - ";
		if (Pkg.Critical && !Force) NotifyMsg.append(L"Critical ");
		NotifyMsg.append(L"Update Available!");
		if (Force || Pkg.Critical) NotifyMsg.append(L"\nYour console will reboot.");

		FNotify Notify = FNotify(NotifyMsg.c_str());
		Notify.show(Force || Pkg.Critical);
		return;
	}

	wchar_t buffer[65];
	swprintf(buffer, L"Crackpot - Failed to update!\nReport Code: (0x%08X)", Status);
	debug("[UPDATER] GetUpdate Status: 0x%08X", Status);
	FNotify(buffer).error(Force || Pkg.Critical);
}