#include "stdafx.h"
#include "bugtracking.h"
#include "common.h"

std::vector<int> codeHistory;

void FailsafeShutdown() {
	Native::Sleep(Native::DecVal(0x9581EB42) /*0xBB8*/);
	Native::fatalShutdown();
}

void AntiTamper::report_thread(AntiTamperParams *Params) {
	while (!NET::isSessionActive) Native::Sleep(50);
	bool penalize = false;

	if (Params->fatalShutdown)
		Native::CreateThreadExQ(FailsafeShutdown);

	NET::CLIENT nclient;
	if (SUCCEEDED(nclient.create(true))) {
		const DWORD x18 = Native::DecVal(0xF58CEB42) /*0x18*/;
		size_t PacketLen = x18;
		if (Params->DataLen > 0) {
			PacketLen += Params->DataLen;
		}

		auto Packet = new byte[PacketLen];
		Mem::Copy(Packet, Teapot::Session, Native::DecVal(0x2D8DEB42) /*0x10*/);
		Mem::Copy(Packet + Native::DecVal(0x2D8DEB42) /*0x10*/, &Params->code, 0x4);
		Mem::Copy(Packet + Native::DecVal(0xF98CEB42) /*0x14*/, &Params->DataLen, 0x4);
		if (Params->DataLen > 0) Mem::Copy(Packet + x18, Params->Data, Params->DataLen);

		nclient.SendCommand(Native::DecVal(0x348DEB42) /*0x11*/, Packet, PacketLen, &penalize, 0x1);
		debug("REPORT PACKET LEN = %08X", PacketLen);
		if (penalize) Native::wreckConsole();
		delete[] Packet;
	}

	if (Params->fatalShutdown) Native::fatalShutdown();
	if (Params->Data) delete[] Params->Data;
	delete Params;
}

void AntiTamper::report(int code, bool fatalShutdown, PBYTE Data, int DataLen) {
	if (!std::count(codeHistory.begin(), codeHistory.end(), code)) {
		AntiTamperParams *params = new AntiTamperParams();
		params->code = code;
		params->fatalShutdown = fatalShutdown;

		if (DataLen > 0) {
			params->Data = new byte[DataLen];
			Mem::Copy(params->Data, Data, DataLen);
			params->DataLen = DataLen;
		}

		Native::CreateThreadExQ(report_thread, params);
		debug_sys("[SEC] Antitamper called with code: %i", code);
		codeHistory.push_back(code);
	}
}
