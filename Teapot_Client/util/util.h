#pragma once
#include "stdafx.h"
#include "config.h"

typedef enum _LEDState
{
	OFF = 0x00,
	RED = 0x08,
	GREEN = 0x80,
	ORANGE = 0x88
}LEDState;

#ifdef DEBUG_PRINT
void debug(const char * fmt, ...);
VOID debug_tmp(const CHAR* strFormat, ...);
VOID debug_sys(const CHAR* strFormat, ...);
VOID debug_newsession();
BOOL dumpBuffer(const char *name, const PVOID buffer, DWORD size, DWORD version = 0);
BOOL dumpModule(const char *name, const PVOID buffer, DWORD size, DWORD version = 0);
void dumpSysHV();
void dumpMoboInfo(byte hash[0x10]);
void dbgBreak();
#else
#define debug
#define debug_sys
#define debug_newsession()
#define dumpBuffer
#define dumpModule
#define dumpSysHV
#define dumpMoboInfo
#define formatBytes
#define hardwareFlagsMoboStr
#define dbgBreak
#endif
//void log_newsession();
//void prtlog(const CHAR* strFormat, ...);

std::string PrintBool(bool val);
std::string ParseIPAddr(IN_ADDR ip, bool secure = true);
std::wstring ParseIPAddrW(IN_ADDR ip, bool secure = true);
std::string GetTimeAsString();
bool STDStrReplace(std::string& str, const std::string& from, const std::string& to);
std::string STDStrGetBaseName(const char *path);
void buildCharStr(char *buf, ...);
size_t hex2bin(byte* target, const char* src);
size_t hex2bin_sec(byte* target, byte key, const char* src);
std::vector<char> HexToBytes(const std::string& hex);
DWORD ModRTU_CRC(byte* buf, int len);
void _memcpy(void *dest, const void *src, size_t size);
void randomBytes(PBYTE pb, size_t num_bytes);
wchar_t* charToWChar(const char* text, wchar_t *stackPtr = NULL);
char *wcharToChar(const wchar_t *text, char *stackPtr = NULL);
//std::string WCharToChar(std::wstring Input);
//std::wstring CharToWChar(std::string Input);
PWCHAR LinkWChar(PWCHAR pStorage, PWCHAR Text, ...);
std::string BytesToHexStr(PBYTE bytes, DWORD len);
void randStr(char *s, const int len);
bool isValidHexStr(const std::string hex);
std::vector<char> HexToBytes(const std::string& hex);
void SetFanSpeed(int speed);
void ReturnToDash();
void xorBlock(PBYTE data, DWORD len, DWORD xorVal);
void clearCache(bool _XAM_CACHE_ALL = false);
std::string GetSelfPath();
std::string SplitFilename(std::string path);
HANDLE CreateWorkerExQ(LPVOID dest, LPVOID param = NULL);
__forceinline DWORD reverseInt(DWORD value);
DWORD DecVal(DWORD val);
QWORD DecQWORD(QWORD val);
u32 ApplyPatches(PVOID buffer, int subtract = 0);
void PatchSpoofGold();
bool CheckC(const char* c);
bool CheckWC(wchar_t c[55]);
const char* XamShowKeyboard(LPCWSTR wseDefaultText, LPCWSTR wszTitleText, LPCWSTR wszDescriptionText, DWORD Length, INT& err);
//void DateRapeConsole();

class Invoke {
public:
	template<typename T>
	static T Call(DWORD dwAddress) { return ((T(*)())dwAddress)(); }

	template<typename T, typename P1>
	static T Call(DWORD dwAddress, P1 p1) { return ((T(*)(P1))dwAddress)(p1); }

	template<typename T, typename P1, typename P2>
	static T Call(DWORD dwAddress, P1 p1, P2 p2) { return ((T(*)(P1, P2))dwAddress)(p1, p2); }

	template<typename T, typename P1, typename P2, typename P3>
	static T Call(DWORD dwAddress, P1 p1, P2 p2, P3 p3) { return ((T(*)(P1, P2, P3))dwAddress)(p1, p2, p3); }

	template<typename T, typename P1, typename P2, typename P3, typename P4>
	static T Call(DWORD dwAddress, P1 p1, P2 p2, P3 p3, P4 p4) { return ((T(*)(P1, P2, P3, P4))dwAddress)(p1, p2, p3, p4); }

	template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5>
	static T Call(DWORD dwAddress, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) { return ((T(*)(P1, P2, P3, P4, P5))dwAddress)(p1, p2, p3, p4, p5); }

	template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	static T Call(DWORD dwAddress, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) { return ((T(*)(P1, P2, P3, P4, P5, P6))dwAddress)(p1, p2, p3, p4, p5, p6); }

	template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	static T Call(DWORD dwAddress, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) { return ((T(*)(P1, P2, P3, P4, P5, P6, P7))dwAddress)(p1, p2, p3, p4, p5, p6, p7); }

	template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	static T Call(DWORD dwAddress, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8) { return ((T(*)(P1, P2, P3, P4, P5, P6, P7, P8))dwAddress)(p1, p2, p3, p4, p5, p6, p7, p8); }

	template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
	static T Call(DWORD dwAddress, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9) { return ((T(*)(P1, P2, P3, P4, P5, P6, P7, P8, P9))dwAddress)(p1, p2, p3, p4, p5, p6, p7, p8.p9); }

	template<typename T, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
	static T Call(DWORD dwAddress, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10) { return ((T(*)(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10))dwAddress)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); }
};