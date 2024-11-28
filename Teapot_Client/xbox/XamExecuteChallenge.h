#pragma once

#define STATUS_INVALID_PARAMETER 0xC000000D
#define STATUS_INSUFFICIENT_RESOURCES 0xC000009A
#define XAMCHALLENGE_HVMAGIC Native::DecVal(0xF33EEB42) /*0x4E4E*/

//xdk getpyhsaddr 8172BDC8  48 5E 38 A5
//retail getpyhsaddr 816798EC  48 3F 8B F9

extern detour<NTSTATUS> XamExecuteChallengeStub;
extern DWORD ChallengeCount;
extern bool ChalRan;

#pragma pack(push, 1)
struct XAM_CHAL_BUFFER {
	DWORD Status; // 0x0
	BYTE bReserved[4]; // 0x4
	WORD wHvMagic; // 0x8
	WORD wHvVersion; // 0xA
	WORD wHvQfe; // 0xC
	WORD wBldrFlags; // 0xE
	DWORD dwBaseKernelVersion; // 0x10
	DWORD dwUpdateSequence; // 0x14
	DWORD dwHvKeysStatusFlags; // 0x18
	DWORD dwConsoleTypeSeqAllow; // 0x1C
	QWORD qwRTOC; // 0x20
	QWORD qwHRMOR; // 0x28
	BYTE bHvECCDigest[XECRYPT_SHA_DIGEST_SIZE]; // 0x30
	BYTE bCpuKeyDigest[XECRYPT_SHA_DIGEST_SIZE]; // 0x44
	BYTE rsaMemoryKey[0x80]; // 0x58
	WORD hvExAddr; // 0xD8 (bits 16-32 of hvex executing addr)
	BYTE bHvDigest[0x6]; // 0xDA (last 6 bytes of first hv hash)
	DWORD dwTickCount; // 0xE0 
	BYTE bSaltHmacDigest[XECRYPT_SHA_DIGEST_SIZE]; // 0xE4
	XBOX_KRNL_VERSION XboxKrnlVersion; // 0xF8
};
#pragma pack(pop)


namespace Xbox {
	namespace Challenges {
		namespace Xam {
			extern byte IntegrityDigest[0xC];

			NTSTATUS XamExecuteChallengeHook(PBYTE Page, DWORD dwPage, PBYTE Salt, DWORD dwSalt, XAM_CHAL_BUFFER *Responce, DWORD dwResponce);
			__forceinline NTSTATUS ExecuteRequest(XAM_CHAL_BUFFER *Response, PBYTE Salt);
		}
	}
}