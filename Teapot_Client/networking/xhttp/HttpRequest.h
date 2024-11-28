#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#pragma once

#include "stdafx.h"
#include <xhttp.h>


/*void httpTest() {
	string path("/file.php");

	BOOL requestStatus = FALSE;
	HttpRequest req;
	string s = req.SendRequest("cdn.teapot.services", path, FALSE, &requestStatus);

	if (requestStatus) {
		debug("XHTTP Request success! %s", s.c_str());
	}else debug("XHTTP Request failed :(");
}*/


/*debug("XHttpQueryHeaders = 0x%08X", (PDWORD)XHttpQueryHeaders);
debug("XHttpStartup = 0x%08X", (DWORD)XHttpStartup);
debug("XHttpOpen = 0x%08X", (DWORD)XHttpOpen);
debug("XHttpConnect = 0x%08X", (DWORD)XHttpConnect);
debug("XHttpOpenRequest = 0x%08X", (DWORD)XHttpOpenRequest);
debug("XHttpSendRequest = 0x%08X", (DWORD)XHttpSendRequest);
debug("XHttpSetStatusCallback = 0x%08X", (DWORD)XHttpSetStatusCallback);
debug("XHttpReceiveResponse = 0x%08X", (DWORD)XHttpReceiveResponse);
debug("XHttpQueryHeaders = 0x%08X", (DWORD)XHttpQueryHeaders);
debug("XHttpReadData = 0x%08X", (DWORD)XHttpReadData);
debug("XHttpDoWork = 0x%08X", (DWORD)XHttpDoWork);
debug("XHttpCloseHandle = 0x%08X", (DWORD)XHttpCloseHandle);
debug("XHttpShutdown = 0x%08X", (DWORD)XHttpShutdown);*/

#define RECEIVE_BUFFER_SIZE 16384
#define MAX_HOSTNAME_LENGTH 128 
#define MAX_HEADER_LENGTH   16384 
#define MAX_URL_LENGTH      1024 

enum HttpState
{
	HTTP_STATE_IDLE,
	HTTP_STATE_REQUEST_SENT,
	HTTP_STATE_RECEIVE_RESPONSE,
	HTTP_STATE_WAITING_FOR_RESPONSE,
	HTTP_STATE_RESPONSE_RECEIVED,
	HTTP_STATE_WAITING_FOR_DATA,
	HTTP_STATE_DATA_RECEIEVD,
	HTTP_STATE_COMPLETED,
	HTTP_STATE_ERROR,
};

struct AsyncHttpRequest
{
	char ServerName[XHTTP_REQUEST_MAX_HOST];
	char Path[XHTTP_REQUEST_MAX_URL];
	BYTE *Buffer;
	BOOL Https;
	DWORD Size;
	DWORD Port;
};

class HttpRequest
{
public:
	HttpRequest();
	string SendRequest(CHAR *ServerName, string Path, BOOL https, BOOL *success, DWORD port = 80, DWORD dataLength = RECEIVE_BUFFER_SIZE);

	void MoveToState(DWORD state);

	AsyncHttpRequest AsyncRequest;
	DWORD dwBytesRead;
	
private:
	static VOID CALLBACK StatusCallback(HINTERNET hInternet,
		DWORD_PTR dwpContext,
		DWORD dwInternetStatus,
		LPVOID lpvStatusInformation,
		DWORD dwStatusInformationSize);
	DWORD GetHTTPStatusCode();
	HRESULT GetResponseStatusCode(
		HINTERNET hRequest,
		DWORD * pdwStatusCode
	);
	HRESULT HttpRequest::GetResponseETagHeader(
		HINTERNET hRequest
	);
	HRESULT GetResponseContentLength(
		HINTERNET hRequest,
		DWORD * pdwContentLength
	);
	HRESULT GetResponseHeader(
		HINTERNET hRequest,
		DWORD dwInfoLevel,
		CONST CHAR * pszHeader,
		VOID * pvBuffer,
		DWORD * pdwBufferLength
	);

	VOID ResetRequest();

	PBYTE receiveBuffer;
	DWORD dwState;
	BOOL isRequestBusy;
	HINTERNET hSession, hConnect, hRequest;
	DWORD dwContentLength;
	DWORD dwBufferLength;
	BOOL hasContentLengthHeader;
	DWORD dwHttpStatusCode;
};

#endif