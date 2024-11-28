#include "stdafx.h"
#include "HttpRequest.h"
#include "common.h"
/*typedef struct _XAUTH_SETTINGS {
	DWORD SizeOfStruct;
	DWORD Flags;
} XAUTH_SETTINGS, *PXAUTH_SETTINGS;


VOID HttpRequest::ResetRequest(){
	isRequestBusy = FALSE;
	hSession = NULL;
	hConnect = NULL;
	hRequest = NULL;
	dwBytesRead = 0;
	dwContentLength = 0;
	dwBufferLength = 4;
	hasContentLengthHeader = FALSE;
	dwState = HTTP_STATE_IDLE;
	dwHttpStatusCode = 0;
	receiveBuffer = NULL;
}

HttpRequest::HttpRequest(){
	ResetRequest();
}

void HttpRequest::MoveToState(DWORD state){
	dwState = state;
}

VOID CALLBACK HttpRequest::StatusCallback(HINTERNET hInternet, DWORD_PTR dwpContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationSize){
	HttpRequest *request = (HttpRequest*)dwpContext;

	debug("Callback State = %08X", dwInternetStatus);

	switch (dwInternetStatus){
	case XHTTP_CALLBACK_STATUS_HANDLE_CLOSING:                       
	case XHTTP_CALLBACK_STATUS_REDIRECT:                                               
	case XHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE:              
	case XHTTP_CALLBACK_STATUS_WRITE_COMPLETE:                      
	case XHTTP_CALLBACK_STATUS_REQUEST_ERROR: request->MoveToState(HTTP_STATE_ERROR); break;
	case XHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE: request->MoveToState(HTTP_STATE_RESPONSE_RECEIVED); break;
	case XHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE: request->MoveToState(HTTP_STATE_RECEIVE_RESPONSE); break;
	case XHTTP_CALLBACK_STATUS_READ_COMPLETE:
		request->dwBytesRead = dwStatusInformationSize;
		request->MoveToState(HTTP_STATE_DATA_RECEIEVD);
		break;
	}
}

DWORD HttpRequest::GetHTTPStatusCode(){
	return dwHttpStatusCode;
}

HRESULT HttpRequest::GetResponseStatusCode(HINTERNET hRequest, DWORD * pdwStatusCode){
	HRESULT hr = S_OK;
	DWORD dwBufferLength = sizeof(DWORD);
	hr = GetResponseHeader(hRequest,
		(XHTTP_QUERY_STATUS_CODE | XHTTP_QUERY_FLAG_NUMBER),
		XHTTP_HEADER_NAME_BY_INDEX,
		pdwStatusCode,
		&dwBufferLength);
	return hr;
}

HRESULT HttpRequest::GetResponseETagHeader(HINTERNET hRequest){
	HRESULT hr = S_OK;
	DWORD dwBufferLength = MAX_HEADER_LENGTH;
	PCHAR ETag = new CHAR[MAX_HEADER_LENGTH];
	hr = GetResponseHeader(hRequest,
		XHTTP_QUERY_CUSTOM,
		"ETag",
		ETag,
		&dwBufferLength);
	delete[] ETag;
	return hr;
}

HRESULT HttpRequest::GetResponseContentLength(HINTERNET hRequest, DWORD * pdwContentLength){
	HRESULT hr = S_OK;
	DWORD dwBufferLength = sizeof(DWORD);
	hr = GetResponseHeader(hRequest, (XHTTP_QUERY_CONTENT_LENGTH | XHTTP_QUERY_FLAG_NUMBER), XHTTP_HEADER_NAME_BY_INDEX, pdwContentLength, &dwBufferLength);
	return hr;
}

HRESULT HttpRequest::GetResponseHeader(HINTERNET hRequest, DWORD dwInfoLevel, CONST CHAR * pszHeader, VOID * pvBuffer, DWORD * pdwBufferLength){
	HRESULT hr = S_OK;
	if (!XHttpQueryHeaders(hRequest, dwInfoLevel, pszHeader, pvBuffer, pdwBufferLength, XHTTP_NO_HEADER_INDEX))
		hr = HRESULT_FROM_WIN32(GetLastError());
	return hr;
}

string FAIL_STR = "";
string HttpRequest::SendRequest(CHAR *ServerName, string Path, BOOL https, BOOL *success, DWORD port, DWORD dataLength){
	if (!isRequestBusy){
		if (dataLength > RECEIVE_BUFFER_SIZE){
			DbgPrint("Error: Receive buffer size must not be greater than 0x%X bytes!\n", RECEIVE_BUFFER_SIZE);
			*success = FALSE;
			return FAIL_STR;
		}

		XHttpStartup(0, NULL);
		ResetRequest();
		isRequestBusy = TRUE;
		BOOL ret = FALSE;
		hSession = XHttpOpen("Xbox/TPAPI", XHTTP_ACCESS_TYPE_DEFAULT_PROXY, XHTTP_NO_PROXY_NAME, XHTTP_NO_PROXY_BYPASS, XHTTP_FLAG_ASYNC);
		if (hSession){
			DWORD portToUse = port == -1 ? INTERNET_DEFAULT_PORT : port;
			hConnect = XHttpConnect(hSession, ServerName, https ? INTERNET_DEFAULT_HTTPS_PORT : portToUse, https ? XHTTP_FLAG_SECURE : 0);
		}
		if (hConnect){
			hRequest = XHttpOpenRequest(hConnect, "GET", Path.c_str(), NULL, XHTTP_NO_REFERRER, NULL, 0);
		}
		if (hRequest){
			XHTTP_STATUS_CALLBACK callback = XHttpSetStatusCallback(hRequest, StatusCallback, XHTTP_CALLBACK_FLAG_ALL_COMPLETIONS, NULL);
			ret = XHttpSendRequest(hRequest, XHTTP_NO_ADDITIONAL_HEADERS, NULL, XHTTP_NO_REQUEST_DATA, NULL, NULL, (DWORD_PTR)this);
		}
		if (ret){
			MoveToState(HTTP_STATE_REQUEST_SENT);
			while (true){
				switch (dwState) {
				case HTTP_STATE_REQUEST_SENT: break;
				case HTTP_STATE_RECEIVE_RESPONSE:
					ret = XHttpReceiveResponse(hRequest, NULL);
					if (!ret) MoveToState(HTTP_STATE_ERROR);
					else MoveToState(HTTP_STATE_WAITING_FOR_RESPONSE);
					break;
				case HTTP_STATE_WAITING_FOR_RESPONSE: break;
				case HTTP_STATE_RESPONSE_RECEIVED:{
					HRESULT hr = GetResponseStatusCode(hRequest, &dwHttpStatusCode);
					if (FAILED(hr)) {
						MoveToState(HTTP_STATE_ERROR);
						break;
					}
					else if (dwHttpStatusCode >= HTTP_STATUS_BAD_REQUEST) {
						MoveToState(HTTP_STATE_ERROR);
						break;
					}
					else if (XHttpQueryHeaders(hRequest, XHTTP_QUERY_CONTENT_LENGTH | XHTTP_QUERY_FLAG_NUMBER, XHTTP_HEADER_NAME_BY_INDEX, &dwContentLength, &dwBufferLength, XHTTP_NO_HEADER_INDEX)) {
						if (dwContentLength > 0) {
							hasContentLengthHeader = TRUE;
							MoveToState(HTTP_STATE_WAITING_FOR_DATA);
						}
						else {
							MoveToState(HTTP_STATE_DATA_RECEIEVD);
							break;
						}
					}
					else if (GetLastError() == ERROR_XHTTP_HEADER_NOT_FOUND) {
						dwContentLength = dataLength;
						hasContentLengthHeader = FALSE;
					}
					else {
						MoveToState(HTTP_STATE_ERROR);
						break;
					}

					if (dwContentLength > RECEIVE_BUFFER_SIZE) {
						debug("Error: Content length must not be greater than 0x%X bytes!\n", RECEIVE_BUFFER_SIZE);
						*success = FALSE;
						return FAIL_STR;
					}

					receiveBuffer = (PBYTE)malloc(dwContentLength);
					if (receiveBuffer == NULL) {
						MoveToState(HTTP_STATE_ERROR);
						break;
					}
					ret = XHttpReadData(hRequest, receiveBuffer, dwContentLength, NULL);
					if ((dwBytesRead > 0 && !hasContentLengthHeader) || (dwBytesRead == dwContentLength && hasContentLengthHeader))
						MoveToState(HTTP_STATE_DATA_RECEIEVD);
					else MoveToState(HTTP_STATE_WAITING_FOR_DATA);
					break;
				}
				case HTTP_STATE_WAITING_FOR_DATA:
					if ((dwBytesRead > 0 && !hasContentLengthHeader) || (dwBytesRead == dwContentLength && hasContentLengthHeader))
						MoveToState(HTTP_STATE_DATA_RECEIEVD);
					break;
				case HTTP_STATE_DATA_RECEIEVD:
					MoveToState(HTTP_STATE_COMPLETED);
					break;
				case HTTP_STATE_COMPLETED:
				case HTTP_STATE_ERROR: goto exitLoop;
				}
				if (!XHttpDoWork(hSession, 0)){
					MoveToState(HTTP_STATE_ERROR);
				}
				Sleep(33);
			}
		}
	exitLoop:
		if (dwState == HTTP_STATE_COMPLETED && receiveBuffer != NULL){
			debug("Http request succeeded with status code %i", dwHttpStatusCode);
			receiveBuffer[dwBytesRead] = 0;
		}else debug("Http request failed with status code %i", dwHttpStatusCode);

		if (hRequest) XHttpCloseHandle(hRequest);
		if (hConnect) XHttpCloseHandle(hConnect);
		if (hSession) XHttpCloseHandle(hSession);
		
		XHttpShutdown();

		isRequestBusy = FALSE;

		if (dwState == HTTP_STATE_COMPLETED && receiveBuffer != NULL){
			string ret((PCHAR)receiveBuffer, dwContentLength);        
			*success = TRUE;
			free(receiveBuffer);
			return ret;
		}
	}

	debug("state = %08X", dwState);
	debug("state = %i", dwState);
	*success = FALSE;
	return FAIL_STR;
}*/