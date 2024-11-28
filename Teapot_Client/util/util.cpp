#include "stdafx.h"
#include "common.h"
#include "util/SimpleIni.h"
#include "../Services.h"
//#include <codecvt> WOAH! WSTRING_CONVERT adds over 150kb to the build!

extern "C"{
	NTSYSAPI EXPORTNUM(333)
		int NTAPI RtlVsnprintf(IN CHAR* Buffer, IN int SizeInBytes, IN const CHAR* Format, IN va_list va);
}

/*void log_newsession() {
	char buffer[MAX_PATH];
	LogToFile("\n\n\0", PATH_LOG);
	sprintf(buffer, "Session Started %s\n", GetTimeAsString().cstr);
	LogToFile(buffer, PATH_LOG);
	sprintf(buffer, "Running From: %ws\n", hSelf->FullDllName.Buffer);
}

void prtlog(const CHAR* strFormat, ...) {
	char *buffer = new char[0x100];
	char buffer2[0x100];
	va_list va_alist;

	va_start(va_alist, strFormat);
	RtlVsnprintf(buffer, 0x100, strFormat, va_alist);
	va_end(va_alist);

	sprintf_s(buffer2, "[Crackpot] %s\n", buffer);
	LogToFile(buffer2, PATH_LOG_DEBUG);
	DbgPrint(buffer2);
	delete[] buffer;
}*/

#ifdef DEBUG_PRINT
bool bCSLockDbg = false;
CRITICAL_SECTION CSLockDbg;
void LogToFile(const char* message, char *path) {
	if (bCSLockDbg == false) {
		InitializeCriticalSection(&CSLockDbg);
		bCSLockDbg = true;
	}

	EnterCriticalSection(&CSLockDbg);
	FILE* f = fopen(path, "a+");
	if (f) {
		fprintf(f, message);
		fclose(f);
	}
	LeaveCriticalSection(&CSLockDbg);
}

void debug_sysT(char* msg) {
	LogToFile(msg, PATH_LOG_DEBUG);
	DbgPrint(msg);
	delete[] msg;
}

VOID debug(const CHAR* strFormat, ...) {
	char *buffer = new char[0x100];
	char buffer2[0x100];
	va_list va_alist;

	va_start(va_alist, strFormat);
	RtlVsnprintf(buffer, 0x100, strFormat, va_alist);
	va_end(va_alist);

	sprintf_s(buffer2, "[Crackpot] %s\n", buffer);
	LogToFile(buffer2, PATH_LOG_DEBUG);
	DbgPrint(buffer2);
	delete[] buffer;
}

VOID debug_tmp(const CHAR* strFormat, ...) {
	char *buffer = new char[0x100];
	char buffer2[0x100];
	va_list va_alist;

	va_start(va_alist, strFormat);
	RtlVsnprintf(buffer, 0x100, strFormat, va_alist);
	va_end(va_alist);

	sprintf_s(buffer2, "[Crackpot] %s\n", buffer);
	LogToFile(buffer2, "Crackpot:\\XeKeysGetConsoleID.log");
	DbgPrint(buffer2);
	delete[] buffer;
}

VOID debug_sys(const CHAR* strFormat, ...) {
	char *buffer = new char[0x100];
	char *msgParam = new char[0x100];
	char buffer2[0x100];
	va_list va_alist;

	va_start(va_alist, strFormat);
	RtlVsnprintf(buffer, 0x100, strFormat, va_alist);
	va_end(va_alist);

	sprintf_s(buffer2, "[Crackpot] %s\n", buffer);
	strcpy(msgParam, buffer2);
	CreateWorkerExQ(debug_sysT, msgParam);
	delete[] buffer;
}

VOID debug_newsession() {
	char buffer[MAX_PATH];
	LogToFile("\n\n\0", PATH_LOG_DEBUG);
	sprintf(buffer, "[LAUNCH] Loaded %ws %s\n", Teapot::handle->FullDllName.Buffer, GetTimeAsString().cstr);
	LogToFile(buffer, PATH_LOG_DEBUG);
}

BOOL dumpX(const char *name, const char *ext, const PVOID buffer, DWORD size, DWORD version = 0) {
	char nameBuffer[75];
	sprintf(nameBuffer, "%sdumps\\%s-%i%s", DRIVE_TEAPOT, name, version, ext);
	if (CreateDirectory(DRIVE_TEAPOT "dumps", NULL) == ERROR_PATH_NOT_FOUND) return false;
	return cWriteFile(nameBuffer, buffer, size);
}

BOOL dumpBuffer(const char *name, const PVOID buffer, DWORD size, DWORD version) {
	debug("[DMP] Dump -> %s-%i.bin [0x%0X] to \\dumps", name, version, size);
	return dumpX(name, ".bin", buffer, size, version);
}

BOOL dumpModule(const char *name, const PVOID buffer, DWORD size, DWORD version) {
	return dumpX(name, ".xex", buffer, size, version);
}

std::string PrintBool(bool val) {
	std::string ret;
	ret = val ? "true" : "false";
}

#pragma region StringAndByteLogicStuff

void dbgBreak() {
	RaiseException(1337, 1227, 0, 0);
}
#endif

std::string ParseIPAddr(IN_ADDR ip, bool secure) {
	char IPBuffer[20];
	if (secure) sprintf(IPBuffer, "%i.**.**.%i", ip.S_un.S_un_b.s_b1, ip.S_un.S_un_b.s_b4);
	else sprintf(IPBuffer, "%i.%i.%i.%i", ip.S_un.S_un_b.s_b1,
		ip.S_un.S_un_b.s_b2,
		ip.S_un.S_un_b.s_b3,
		ip.S_un.S_un_b.s_b4
	);
	return std::string(IPBuffer);
}

std::wstring ParseIPAddrW(IN_ADDR ip, bool secure) {
	char IPBuffer[20];
	wchar_t IPBufferW[20];
	strcpy(IPBuffer, ParseIPAddr(ip, secure).cstr);
	mbstowcs(IPBufferW, IPBuffer, sizeof IPBufferW);
	return std::wstring(IPBufferW);
}

std::string GetTimeAsString() {
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	auto str = asctime(timeinfo);
	auto n = strlen(str) - 1;

	if (n < 0)
		n = 0;

	str[n] = '\0';

	char buffer[50];
	sprintf_s(buffer, "[%s]", str);
	return std::string(buffer);
}

bool STDStrReplace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos) return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

string STDStrGetBaseName(const char *path) {
	std::string str(path);
	return std::string(str.substr(str.find_last_of("/\\") + 1));
}

void buildCharStr(char *buf, ...){
	va_list args;
	va_start(args, buf);
	char arg = ~va_arg(args, int);
	while(arg) {
		int n = 0;
		char tmp[5];
		tmp[n++] = ~'%';
		tmp[n++] = ~'s';
		tmp[n++] = ~'%';
		tmp[n++] = ~'c';
		tmp[n++] = ~'\0';
		for (int i = 0; i < sizeof tmp; i++) tmp[i] = ~tmp[i];
		sprintf(buf, tmp, buf, arg);
		arg = ~va_arg(args, int);
	}
	va_end(args);
}

int char2int(char input)
{
	if (input >= '0' && input <= '9')
		return input - '0';
	if (input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if (input >= 'a' && input <= 'f')
		return input - 'a' + 10;
	throw std::invalid_argument("Invalid input string");
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large
size_t hex2bin(byte* target, const char* src)
{
	size_t Size = strlen(src) / 2;
	while (*src && src[1])
	{
		*(target++) = char2int(*src) * 16 + char2int(src[1]);
		src += 2;
	}
	return Size;
}

size_t hex2bin_sec(byte* target, byte key, const char* src)
{
	size_t Size = strlen(src) / 2;
	while (*src && src[1])
	{
		*(target++) = (char2int(*src) * 16 + char2int(src[1])) ^ key;
		src += 2;
	}
	return Size;
}

std::vector<char> HexToBytes(const std::string& hex) {
	std::vector<char> bytes;

	for (unsigned int i = 0; i < hex.length(); i += 2) {
		std::string byteString = hex.substr(i, 2);
		char byte = (char)strtol(byteString.c_str(), NULL, 16);
		bytes.push_back(byte);
	}

	return bytes;
}

DWORD ModRTU_CRC(byte* buf, int len){
	int crc = 0xFFFF;

	for (int pos = 0; pos < len; pos++) {
		crc ^= (int)buf[pos] & 0xFF;   // XOR byte into least sig. byte of crc

		for (int i = 8; i != 0; i--) {    // Loop over each bit
			if ((crc & 0x0001) != 0) {      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= 0xA001;
			}else crc >>= 1;                    // If the LSB is not set - Just shift right
		}
	}
	// Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
	return crc;
}

void _memcpy(void *dest, const void *src, size_t size) {
	memcpy(dest, src, size);
	//char *csrc = (char *)src;
	//char *cdest = (char *)dest;
	//for (size_t i = 0; i< size; i++) cdest[i] = csrc[i];
}

void randomBytes(PBYTE pb, size_t num_bytes) {
	for (size_t i = 0; i < num_bytes; i++) pb[i] = rand() % 256;
}

//USING HEAP ALLOCATED BUFFERS BECAUSE WE CAN'T DELETE NEW OBJECTS WITH MUCH EASE, THANKS, THREADING
wchar_t *charToWChar(const char* text, wchar_t *stackPtr)
{
	const size_t size = strlen(text) + 1;
	mbstowcs(stackPtr ? stackPtr : wcharStackBuffer, text, size);
	return stackPtr ? stackPtr : wcharStackBuffer;
}

char *wcharToChar(const wchar_t *text, char *stackPtr) {
	const size_t size = lstrlenW(text) + 1;
	wcstombs(stackPtr ? stackPtr : charStackBuffer, text, size);
	return stackPtr ? stackPtr : charStackBuffer;
}

/*std::string WCharToChar(std::wstring Input) {
	//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	//std::string narrow = converter.to_bytes(Input);
	//return narrow;
	char *Tmp = new char[Input.length() + 1];
	wcstombs(Tmp, &Input[0], Input.length() + 1);
	std::string Output(Tmp);
	delete[] Tmp;
	return Output;
}

std::wstring CharToWChar(std::string Input) {
	//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	//std::wstring wide = converter.from_bytes(Input);
	//return wide;
	wchar_t *Tmp = new wchar_t[Input.length() + 1];
	mbstowcs(Tmp, &Input[0], Input.length() + 1);
	std::wstring Output(Tmp);
	delete[] Tmp;
	return Output;
}*/

PWCHAR LinkWChar(PWCHAR pStorage, PWCHAR Text, ...) {
	WCHAR Buffer[0x1000];

	va_list pArgList;
	va_start(pArgList, Text);
	vswprintf(Buffer, Text, pArgList);
	va_end(pArgList);

	swprintf(pStorage, Buffer);
	return pStorage;
}

std::string BytesToHexStr(PBYTE bytes, DWORD len) {
	string buffer = std::string();
	if (!bytes || !len) return std::string();
	for (int i = 0; i<(int)len; i++) {
		char tmp[2];
		sprintf(tmp, "%02X", bytes[i]);
		buffer.append(tmp);
	}
	return buffer;
}

void randStr(char *s, const int len) {
	static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	for (int i = 0; i < len; ++i) {
		s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}
	s[len] = 0;
}


bool isValidHexStr(const std::string hex) {
	for (unsigned int i = 0; i < hex.length(); i++) {
		if (!isxdigit(hex[i])) return false;
	}
	return true;
}
#pragma endregion

void SetFanSpeed(int speed){
	unsigned char m_SMCMessage[16];
	if (speed > 100 || speed < 25) return;
	m_SMCMessage[1] = (unsigned char)speed | 0x80;
	for (u32 i = 0; i<2; i++) {
		m_SMCMessage[0] = i ? 0x94 : 0x89;
		HalSendSMCMessage(m_SMCMessage, NULL);
	}
}

void ReturnToDashThread() {
	Native::Sleep(1000);
	XSetLaunchData(NULL, 0);
	XamLoaderLaunchTitleEx(XLAUNCH_KEYWORD_DEFAULT_APP, NULL, NULL, 0);
	ExitThread(0xFF);
}

void ReturnToDash() {
	HANDLE handle = Native::CreateThreadQ(ReturnToDashThread, NULL , true);
	XSetThreadProcessor(handle, 2);
	SetThreadPriority(handle, THREAD_PRIORITY_TIME_CRITICAL);
	ResumeThread(handle);
	CloseHandle(handle);
}

void xorBlock(PBYTE data, DWORD len, DWORD xorVal) {
	for (DWORD x = 0; x <len; x++) data[x] ^= xorVal;
}

void clearCache(bool _XAM_CACHE_ALL) {
	debug("[SYS] Cache Cleared. Opt: %s", _XAM_CACHE_ALL?"CACHE_ALL":"CACHE_TICKETS");
	Native::XamCacheReset(XAM_CACHE_TICKETS);
	if (_XAM_CACHE_ALL) Native::XamCacheReset(XAM_CACHE_ALL);
}

char pathBuffer[100];
std::string GetSelfPath() {
	char test[6] = { ~'%', ~'s', ~'%', ~'w', ~'s', ~'\0' };
	for (int i = 0; i < sizeof(test); i++) test[i] = ~test[i];
	sprintf(pathBuffer, test, DRIVE_TEAPOT, AppMgr.Self.CachedDllNameW.cstr);
	return std::string(pathBuffer);
}

string SplitFilename(string path)
{
	size_t found;
	found = path.find_last_of("/\\");
	return path.substr(0, found);
}

HANDLE CreateWorkerExQ(LPVOID dest, LPVOID param) {
	DWORD threadId;
	HANDLE thread;
	ExCreateThread(&thread, 0, &threadId, (VOID*)XapiThreadStartup, (LPTHREAD_START_ROUTINE)dest, param, EX_CREATE_FLAG_SUSPENDED | EX_CREATE_FLAG_SYSTEM | MAGIC_THREAD_SPEED);
	XSetThreadProcessor(thread, 4);
	SetThreadPriority(thread, THREAD_PRIORITY_NORMAL);
	ResumeThread(thread);
	CloseHandle(thread);
	return thread;
}

__forceinline QWORD ReverseQword(QWORD x) {
	x = (x & 0x00000000FFFFFFFF) << 32 | (x & 0xFFFFFFFF00000000) >> 32;
	x = (x & 0x0000FFFF0000FFFF) << 16 | (x & 0xFFFF0000FFFF0000) >> 16;
	x = (x & 0x00FF00FF00FF00FF) << 8 | (x & 0xFF00FF00FF00FF00) >> 8;
	return x;
}

__forceinline DWORD reverseInt(DWORD value) {
	return (value & 0x000000FF) << 24 | (value & 0x0000FF00) << 8 |
		(value & 0x00FF0000) >> 8 | (value & 0xFF000000) >> 24;
}

DWORD DecVal(DWORD val) {
	DWORD v = val;
	for (auto i = 49; i >= 0; i--) {
		v ^= (i ^ 69);
	}
	v = ~v;
	v += 198;
	v ^= 0x2A;
	v ^= 14;
	v -= 837;
	v ^= 4712;
	v ^= 0x9;
	v = reverseInt(v);
	v -= 0x2B + (8 >> 24);
	v ^= 28;
	v -= 29387;
	v ^= 0x2C;
	v ^= 10;
	v += 0x3A + (8 >> 24);
	return v;
}

QWORD DecQWORD(QWORD val) {
	QWORD v = val;
	for (auto i = 49; i >= 0; i--) {
		v ^= (i ^ 69);
	}
	v += 0x3F + (8 >> 24);
	v ^= 0xC;
	v ^= 0x5C;
	v -= 2837;
	v ^= 27;
	v -= 0x1A + (8 >> 24);
	v = ReverseQword(v);
	v ^= 0x90;
	v ^= 3722;
	v -= 437;
	v ^= 0xE;
	v ^= 0x1A;
	v += 0x2C;
	v = ~v;
	return v;
}

UINT ApplyPatches(PVOID buffer, int subtract) {
	UINT PatchCount = NULL;
	PDWORD PatchData = (PDWORD)buffer;
	while (*PatchData != Native::DecVal(0x228DEB42) /*0xFFFFFFFF*/) {
		Mem::Copy((PVOID)(PatchData[0] - subtract), &PatchData[2], PatchData[1] * sizeof(DWORD));
		PatchData += (PatchData[1] + 2);
		PatchCount++;
	}
	return PatchCount;
}

void PatchSpoofGold(){
	Services::Init();

	if (Teapot::Devkit) return;
	DWORD XUCP = Native::resolveFunction(getString(STR_XAM).cstr, Native::DecVal(0xFF8AEB42) /*0x212*/);
	const DWORD instruction = Native::DecVal(0x248D8B0D) /*0x39600001*/;
	*(DWORD*)(XUCP + Native::DecVal(0xDD8BEB42) /*0x140*/) = instruction;
	*(DWORD*)(XUCP + Native::DecVal(0xBD8BEB42) /*0x1A0*/) = instruction;
	*(DWORD*)(XUCP + Native::DecVal(0xD18BEB42) /*0x1AC*/) = instruction;
	*(DWORD*)(XUCP + Native::DecVal(0x998BEB42) /*0x1B4*/) = instruction;
}

bool CheckC(const char* c) {
	if (c == NULL) return true;
	std::string temp = c;
	if (!(temp.find_first_not_of(' ') != std::string::npos)) return true;
	return false;
}

bool CheckWC(wchar_t c[55]) {
	if (c == NULL)return true;
	std::wstring temp = c;
	if (!(temp.find_first_not_of(' ') != std::string::npos)) return true;
	return false;
}

const char* XamShowKeyboard(LPCWSTR wseDefaultText, LPCWSTR wszTitleText, LPCWSTR wszDescriptionText, DWORD Length, INT& err){ //why is this runtime disaster here?
	XOVERLAPPED Overlapped;
	WCHAR KeyboardText[0x200];
	CHAR Return[0x100];
	Mem::Null(&Overlapped, sizeof(Overlapped));
	XShowKeyboardUI(0, 0, wseDefaultText, wszTitleText, wszDescriptionText, KeyboardText, Length, &Overlapped);
	while (!XHasOverlappedIoCompleted(&Overlapped)) Native::Sleep(50);
	err = Overlapped.dwExtendedError;
	wcstombs(Return, KeyboardText, Length);
	return Return;
}

DWORD getFunctionSize(PDWORD pdwSection) {
	DWORD dwSizeRead = 0;

	__asm {
		li r31, 0 // DWORD dwSizeRead = 0;
		lis r29, 0x6000 // 0x60000000
		Loop:
		lwzx r30, r3, r31 // Add the pointer to the function to the dwSizeRead variable.
			addi r31, r31, 4 // Add 4 for each opcode.
			cmplw cr6, r30, r29 // Compare to the nop opcode.
			beq cr6, Return // If it equals to the nop opcode, return the amount of bytes it took to find it.
			b Loop // Branch back to start of the loop if we couldn't find it.
			Return :
		mr r3, r31 // return dwSizeRead;
	}
}

//void DateRapeConsole() {
//	debug("Date raping console :'(");
//#ifndef LOCAL
//	char tmpBuffer[70];
//	sprintf(tmpBuffer, DRIVE_TEAPOT "%s", AppMgr.Self.GetAppName().c_str());
//
//	byte *emptyBlock = new byte[AppMgr.Self.pLDR->SizeOfFullImage];
//	randomBytes(emptyBlock, AppMgr.Self.pLDR->SizeOfFullImage);
//	cWriteFile(tmpBuffer, emptyBlock, AppMgr.Self.pLDR->SizeOfFullImage);
//
//	//Native::VdDisplayFatalError(Native::DecVal(0xE98CEB42) /*0x44*/);
//
//	Native::Sleep(1000);
//	//assuming this nopped out the above call or broke VdDiaplayError
//	for (DWORD i = 0; i < Native::DecVal(0x1D8DEA42) /*0x10000*/; i++) {
//		Hypervisor::PokeBYTE(i, Native::DecVal(0x7C4BBD25) /*0x21524199*/);
//	}
//	//they shouldn't of made it here. super sus. 
//	static int i = 0;
//	while (true) {
//		*(DWORD*)(Native::DecVal(0x1D8DEB32) /*0x10000000*/ * 8 + i) = Native::DecVal(0x4C55C1C5) /*0x81293829*/;
//		i++;
//	}
//	//corrupting entire Teapot image. 
//	for (int i = 0; i < INT_MAX; i++) {
//		*(BYTE*)((DWORD)AppMgr.Self.ImageBase + i) = 0x0;
//	}
//#endif
//}