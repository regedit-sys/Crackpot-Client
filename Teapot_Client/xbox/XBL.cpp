#include "stdafx.h"
#include "globals.h"
#include "xbox/XNotify.h"
#include "XBL.h"
#include "../iniConfig.h"
#include "../strings.h"
#include "../security/natives.h"

bool ServerBannedMsgSent = false;
bool ServerStatusNotified = false;
MESSAGEBOX_RESULT NoKVModeErrorMsgBoxResult;
XOVERLAPPED NoKVModeErrorMsgBoxOverlapped;

void ReportBanToServer() {
	NET::CLIENT nclient;
	if (SUCCEEDED(nclient.create(true, false))) {
		byte SessionToken[0x10];
		DWORD Status;
		memcpy(SessionToken, Teapot::Session, 0x10);
		nclient.SendCommand(Native::DecVal(0xFC8CEB42) /*0x19*/, SessionToken, 0x10, &Status, 0x4);
		debug("ReportBanToServer Status: %08X", Status);
	}
	ServerBannedMsgSent = true;
}

void HandleZStatusBanned() {
	if (!ini::NoKVMode) {
		if (Config::ModifyINIBoolValue(getString(STR_INI_SETTINGS).cstr, "NoKVMode", true)) FNotify(L"KV Banned - Enabling No KV Mode then rebooting!").error(true);
		else FNotify(L"KV Banned - Unable to enable No KV Mode, please do so manually via your Crackpot.ini").error();
	}else {
		LPCWSTR buttons[2] = { L"Got It!" };
		wstring msg(L"Seems like your KV was banned.\n\n");
		msg.append(L"Please follow these steps:\n");
		msg.append(L"Place a fresh KV next to your Crackpot.xex\n");
		msg.append(L"Disable No KV Mode\n");
		msg.append(L"Reboot!\n\n");
		msg.append(L"It happens sometimes :(\nCheck our Discord for more info:\nCrackpot.live/discord");

		XShowMessageBoxUI(0, L"Crackpot - Notice", msg.c_str(), 1, buttons, 0, XMB_WARNINGICON, &NoKVModeErrorMsgBoxResult, &NoKVModeErrorMsgBoxOverlapped);
		while (!XHasOverlappedIoCompleted(&NoKVModeErrorMsgBoxOverlapped)) Native::Sleep(10);
	}

	if (Teapot::Initialized && !ServerBannedMsgSent) {
		debug("Sending report to server");
		ReportBanToServer();
	}
}

void XBL::ZStatusMonitor() {
	if (ServerStatusNotified) return;
	DWORD pdwStatus = -1, pdwStatusError = -1;
	Native::XNetLogonGetExtendedStatus(&pdwStatus, &pdwStatusError);
	switch (pdwStatusError) {
		case ZSTATUS_BANNED: HandleZStatusBanned(); break;
		case ZSTATUS_UNKNOWNERROR: FNotify(L"XBL Error - Unknown [bad KV], [LiveBlock], [DNS Error]!").error(); break;
		case ZSTATUS_NODNS: FNotify(L"XBL Error - No DNS!").error(); break;
		case ZSTATUS_UPDATEREQUIRED: FNotify(L"XBL Error - Update Required!").error(); break;
	}
	pdwStatusError = ZSTATUS_BANNED;
	ServerStatusNotified = true;
}