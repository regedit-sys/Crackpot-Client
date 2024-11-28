#pragma once

namespace NET {
	class CLIENT {
	private:
		HRESULT StatusConnect;
		sockaddr_in addr_in;

	public:
		SOCKET hSocket;

	public:
		bool NoEncrypt;

		static HRESULT Initialize();
		static bool IsActive();
		static HRESULT WaitForLinkStatus();

		DWORD id;
		CLIENT(const DWORD id = 0);
		~CLIENT();

		HRESULT create(bool MainGateway = false, bool ThrowError = true);
		HRESULT connect();
		HRESULT receive(VOID* Buffer, DWORD BytesExpected);
		DWORD receiveDWORD();
		BOOL receiveBOOL();
		HRESULT SendCommand(DWORD CommandId, VOID* CommandData, DWORD DataLen);
		HRESULT SendCommand(DWORD CommandId, VOID* CommandData, DWORD CommandLength, VOID* Responce, DWORD ResponceLength = NULL);
		VOID close();
	};
}