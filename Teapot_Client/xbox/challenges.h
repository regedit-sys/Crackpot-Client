#pragma once
#define REQUEST_SMC_VERSION 0x12

static BYTE ProductID_16D2S[5] = { 0x31, 0x36, 0x44, 0x32, 0x53 };
static BYTE ProductID_16D4S[5] = { 0x31, 0x36, 0x44, 0x34, 0x53 }, ProductRevisionCheck[4] = { 0x39, 0x35, 0x30, 0x34 };
static BYTE ProductID_VAD6038[7] = { 0x56, 0x41, 0x44, 0x36, 0x30, 0x33, 0x38 }, BenQVersion[5] = { 0x30, 0x34, 0x34, 0x32, 0x31 };

struct CHAL_RESP_XOS_DIGESTS_s {
	byte cpu[0x10]; 
	byte x60[0x10]; 
	byte x70[0x10]; 
};

struct CHAL_RESP_XOS_KVDRIVE_DATA_s{
	BYTE _1[0x24];
	BYTE _2[0x24];
};

typedef struct CHAL_RESP_HV_s {
	BYTE bReserved1[8]; //0x0
	WORD HvMagic;//0x8
	WORD HvVersion; //0xA
	WORD HvQfe; //0xC
	WORD BldrFlags; //0xE
	DWORD BaseKernelVersion; //0x10
	DWORD UpdateSequence; //0x14
	DWORD hvKeysStatusFlags; //0x18
	DWORD ConsoleTypeSeqAllow; //0x1gbC
	QWORD RTOC; //0x20
	QWORD HRMOR; //0x28
	BYTE HvECCDigest[XECRYPT_SHA_DIGEST_SIZE]; //0x30
	BYTE CpuKeyDigest[XECRYPT_SHA_DIGEST_SIZE]; //0x44
	BYTE Signature[0x80]; //0x58
	WORD hvExAddr; //0xD8 (bits 16-32 of hvex executing addr)
	BYTE HvDigest[0x6]; //0xDA (last 6 bytes of first hv hash)
} CHAL_RESP_HV, *PCHAL_RESP_HV;

typedef struct CHAL_RESP_XOS_s{
	DWORD result; //0x0-0x4
	DWORD x4; //0x4-0x8
	QWORD operations; //0x8-0x10
	DWORD x10; //0x10-0x14
	BYTE x14[0x24]; //0x14-0x38
	XEX_EXECUTION_ID xexExecutionId; //0x38-0x50
	CHAL_RESP_XOS_DIGESTS_s digests; //0x50-0x80
	byte x84[0x3]; //0x80-0x83
	byte drive_phase_level; //0x83-0x84
	byte x80[0x6C]; //0x84-0xF0
	CHAL_RESP_XOS_KVDRIVE_DATA_s kvDriveData; //0xF0-0x138
	byte kvSerial[0xC]; //0x138-0x144
	WORD xF0; //0x144-0x146
	WORD bootLdrFlags; //0x146-0x148
	WORD xam_region; //0x148-0x14A
	WORD xam_odd; //0x14A-0x14C
	DWORD x14C; //0x14C-0x150
	DWORD PolicyFlashSize; //0x150-0x154
	DWORD x154; //0x154-0x158
	DWORD hvStatusFlags; //0x158-0x15C
	byte x15C[0x44]; //0x15C-0x1A0
	byte consoleID[5]; //0x1A0-0x1A5	
	byte x1A5[0x2B]; //0x1A5-0x1D0
	DWORD hardwareFlags; //0x1D0-0x1D4
	byte x1D4[0x104]; //0x1D4-0x2D8
	DWORD footerMagic; //0x2B8-0x2DC
}CHAL_RESP_XOS, *PCHAL_RESP_XOS;

extern "C" {
	void HalReadWritePCISpace(int r3, int r4, int r5, int r6, void* buffer, int length, bool WritePCISpace);
}

typedef struct _PCI_CONF_HDR {
	SHORT DeviceID; // 0
	SHORT VendorID; // 2
	SHORT Status; // 4
	SHORT Command; // 6
	BYTE ClassBase; // 7
	BYTE ClassSub; // 8
	BYTE ClassIf; // 9
	BYTE RevisionID; // 0xB
	BYTE BIST; // 0xC
	BYTE HType; // 0xD
	BYTE LatencyTmr; // 0xE
	BYTE CachelineSz; // 0xF
} PCI_CONF_HDR, *PPCI_CONF_HDR;
C_ASSERT(sizeof(PCI_CONF_HDR) == 0x10);

namespace Xbox {
	namespace Challenges {
		namespace XOS {
			extern byte IntegrityDigest[0xC];
			extern DWORD dwCachedXOSAddrEnc;


			static void CacheXOSAddress(DWORD AddrInp) { dwCachedXOSAddrEnc = (AddrInp ^ Native::DecVal(0x418DEB42) /*0x1C*/); }
			static DWORD GetCachedEncryptedXOSAddress() { return dwCachedXOSAddrEnc ^ Native::DecVal(0x418DEB42) /*0x1C*/; }
			static void InitIntegrityDigest() { Mem::Copy(Xbox::Challenges::XOS::IntegrityDigest, (PVOID)GetCachedEncryptedXOSAddress(), Native::DecVal(0x318DEB42) /*0x0C*/); }
			DWORD XamLoaderExecuteAsyncChallengeHook(DWORD ExecuteSupervisorChallengeAddress, DWORD dwTaskParam1, char* pbDaeTableName, DWORD cbDaeTableName, PBYTE challenge, DWORD size);
		} 
	}
}
