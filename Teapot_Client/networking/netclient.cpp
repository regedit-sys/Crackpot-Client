#include "stdafx.h"
#include "common.h"
#include <unordered_map>
#include "../util/XeCrypt.h"
using namespace NET;

#pragma region PreprocessorDefinitions
#define SEND_RECV_SIZE 1024
#define CFG_DEFAULT_SOCK_BUFFER_SIZE 128
#define CFG_MAX_SIMULTANEOUS_RESPONSES 16
#define CFG_SOCKET_TIMEOUT 3500

#define LINKSTATUSRETRY_MAX 10

SHORT iGetLinkStatusRetry = 0;

HRESULT CLIENT::Initialize() {
	XNetStartupParams xnsp = { 0 };
	WSADATA WsaData;
	HRESULT WaitForLink;

	xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
	xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
	xnsp.cfgQosSrvMaxSimultaneousResponses = 16; //uh c0z??? Qos flag? w.e
	srand(static_cast<long int>(time(NULL)));

	if (FAILED(WaitForLink = WaitForLinkStatus())) {
		debug("[NET] LINK: Unable to verify status, attempting to resolve DNS anyway.");
	}

	if (NetDll_XNetStartup(XNCALLER_SYSAPP, &xnsp) != S_OK) {
		debug("[NET] XNet Startup Failed!");
	}

	if (NetDll_WSAStartupEx(XNCALLER_SYSAPP, MAKEWORD(2, 2), &WsaData, 2) != S_OK) {
		debug("[NET] WSA Startup Failed!");
	}

	debug("[NET] Initalization Complete!");
	return ERROR_SUCCESS;
}

bool CLIENT::IsActive() {
	XNADDR XNAddr;
	DWORD dwStatus = XNetGetEthernetLinkStatus();
	if (!((dwStatus & XNET_ETHERNET_LINK_ACTIVE) != 0)) return false;
	return  XNetGetTitleXnAddr(&XNAddr) != XNET_GET_XNADDR_PENDING;
}

HRESULT CLIENT::WaitForLinkStatus() {
	while (!IsActive()) {
		if (iGetLinkStatusRetry++ > LINKSTATUSRETRY_MAX - 1) return E_FAIL;
		debug("[NET] LINK: Not Yet Found, retrying(%i/5)...", iGetLinkStatusRetry, LINKSTATUSRETRY_MAX);
		Native::Sleep(1000);
	}
	return ERROR_SUCCESS;
}

HRESULT CLIENT::create(bool MainGateway, bool ThrowError) {
	int timeout = CFG_SOCKET_TIMEOUT;
	bool optInsecureSockets = true;

	this->close(); //close any existing handles, in case we are in a retry

	if ((hSocket = NetDll_socket(XNCALLER_SYSAPP, AF_INET, SOCK_STREAM, IPPROTO_TCP)) == SOCKET_ERROR) {
		debug("[NET::CONNECT] Setsockopt error: %d\n", WSAGetLastError());
		return E_FAIL;
	}

	NetDll_setsockopt(XNCALLER_SYSAPP, hSocket, SOL_SOCKET, SO_MARKINSECURE, (PCSTR)&optInsecureSockets, sizeof(BOOL));
	NetDll_setsockopt(XNCALLER_SYSAPP, hSocket, SOL_SOCKET, SO_RCVTIMEO, (PCSTR)&timeout, sizeof(DWORD));
	NetDll_setsockopt(XNCALLER_SYSAPP, hSocket, SOL_SOCKET, SO_SNDTIMEO, (PCSTR)&timeout, sizeof(DWORD));

	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(4880);
	addr_in.sin_addr.S_un.S_un_b.s_b1 = 0x42;
	addr_in.sin_addr.S_un.S_un_b.s_b2 = 0x17;
	addr_in.sin_addr.S_un.S_un_b.s_b3 = 0xEC;
	addr_in.sin_addr.S_un.S_un_b.s_b4 = 0xEC;

	if (FAILED(connect())) {
		if (ThrowError) {
			if (AppMgr.HasDashLoaded) {
				if (!IsActive()) FNotify(L"Crackpot - Network Link Unplugged/Disconnected!\nRebooting Console!").error(true);
				else FNotify(L"Crackpot - Server Connection Lost\nRestarting Console...").error(true);
			}
			else FNotify(L"Crackpot - Connection Failed.").error();
		}
		return E_FAIL;
	}

	return ERROR_SUCCESS;
}

HRESULT CLIENT::connect() {
	if (NetDll_connect(XNCALLER_SYSAPP, hSocket, (struct sockaddr*)&addr_in, sizeof(sockaddr_in)) == SOCKET_ERROR) {

		debug("[IP Address], %d.%d.%d.%d:%d", addr_in.sin_addr.S_un.S_un_b.s_b1, addr_in.sin_addr.S_un.S_un_b.s_b2, addr_in.sin_addr.S_un.S_un_b.s_b3, addr_in.sin_addr.S_un.S_un_b.s_b4, addr_in.sin_port);

		debug("[Net] Can#t connect");
	}
	else {
		return ERROR_SUCCESS;
	}
	hSocket = INVALID_SOCKET;
	return E_FAIL;
}

HRESULT CLIENT::SendCommand(DWORD CommandId, VOID* CommandData, DWORD DataLen) {
	if (hSocket == INVALID_SOCKET) return E_FAIL;

	DWORD packSize = 8 + DataLen;
	PBYTE buffer = (PBYTE)Native::Malloc(packSize);
	if (!buffer) {
		debug("[NET] Send CMD - Malloc FAILED!");
		return E_FAIL;
	}

	Mem::Copy(buffer, &CommandId, sizeof(DWORD));
	Mem::Copy(buffer + 4, &DataLen, sizeof(DWORD));

	Mem::Copy(buffer + 8, CommandData, DataLen);

	DWORD bytesLeft = packSize;
	CHAR* curPos = (CHAR*)buffer;
	while (bytesLeft > 0) {
		DWORD sendSize = min(SEND_RECV_SIZE, bytesLeft);
		DWORD cbSent = NetDll_send(XNCALLER_SYSAPP, hSocket, curPos, sendSize, NULL);
		if (cbSent == SOCKET_ERROR) {
			debug("[NET] SEND CMD Error %d", WSAGetLastError());
			Mem::Null(buffer, packSize);
			free(buffer);
			return E_FAIL;
		}
		bytesLeft -= cbSent;
		curPos += cbSent;
	}


	Mem::Null(buffer, packSize);
	Native::Free(buffer);
	return ERROR_SUCCESS;
}

HRESULT CLIENT::receive(VOID* Buffer, DWORD BytesExpected) {
	if (hSocket == INVALID_SOCKET) return E_FAIL;

	DWORD bytesLeft = BytesExpected;
	DWORD bytesRecieved = 0;
	while (bytesLeft > 0) {
		DWORD recvSize = min(SEND_RECV_SIZE, bytesLeft);
		DWORD cbRecv = NetDll_recv(XNCALLER_SYSAPP, hSocket, (CHAR*)Buffer + bytesRecieved, recvSize, NULL);
		if (cbRecv == SOCKET_ERROR) {
			debug("[NET]: Receive Error %d | Bytes Left: 0x%08X", WSAGetLastError(), bytesLeft);
			return E_FAIL;
		}
		if (cbRecv == 0) {
			debug("[NET] Receive Bytes: 0 | Expected = %i", BytesExpected);
			break;
		}
		bytesLeft -= cbRecv;
		bytesRecieved += cbRecv;
	}

	if (bytesRecieved != BytesExpected) return E_FAIL;
	return ERROR_SUCCESS;
}

DWORD CLIENT::receiveDWORD() {
	DWORD tmp;
	if (FAILED(receive(&tmp, 0x4))) return 0;
	return tmp;
}

BOOL CLIENT::receiveBOOL() {
	BOOL tmp;
	receive(&tmp, 0x1);
	return tmp;
}

HRESULT CLIENT::SendCommand(DWORD CommandId, VOID* CommandData, DWORD CommandLength, VOID* Responce, DWORD ResponceLength) {
	if (hSocket == INVALID_SOCKET) return E_FAIL;
	if (SendCommand(CommandId, CommandData, CommandLength) != ERROR_SUCCESS) return E_FAIL;
	if (ResponceLength)
		if (receive(Responce, ResponceLength) != ERROR_SUCCESS) {
			debug("[NET] Send CMD - Receive Failed!");
			return E_FAIL;
		}
	return ERROR_SUCCESS;
}

VOID CLIENT::close() {
	if (hSocket != INVALID_SOCKET) {
		NetDll_shutdown(XNCALLER_SYSAPP, hSocket, SD_BOTH);
		NetDll_closesocket(XNCALLER_SYSAPP, hSocket);
		hSocket = INVALID_SOCKET;
	}
}

CLIENT::CLIENT(const DWORD id) {
	hSocket = INVALID_SOCKET;
	this->id = id;

}

CLIENT::~CLIENT() {
	close();
}