#pragma once

#define MODULE_XOSC "xosc9v2.xex"

#define REQUEST_SMC_VERSION 0x12
#define IOCTL_DISK_VERIFY 0x4D014

#define STATUS_INVALID_PARAMETER_1 0xC00000EF
#define STATUS_INVALID_PARAMETER_7 0xC00000F5

#define XOSC_DEVICESIZE_OPEN_OPTIONS FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | 0x800000

enum xoscStatusFlags : long long {
	STATUS_TASK_SHOULD_EXIT = 0x2000000000000000,
	STATUS_FLAG_TITLE_TERMINATED = 0x4000000000000000
};

struct digest_s {
	char cpu[0x10];
	char x60[0x10];
	char x70[0x10];
};

struct kvDriveData_s {
	char _1[0x24];
	char _2[0x24];
};

struct challengeResponse_s {
	int result; //0x0-0x4
	short x4[2]; //0x4-0x8
	long long operations; //0x8-0x10
	int status; //0x10-0x14
	char x14[0x24]; //0x14-0x38
	XEX_EXECUTION_ID xexExecutionId; //0x38-0x50
	digest_s digests; //0x50-0x80
	char x84[0x3]; //0x80-0x83
	char drive_phase_level; //0x83-0x84
	char x80[0x6C]; //0x84-0xF0
	kvDriveData_s kvDriveData; //0xF0-0x138
	char kvSerial[0xC]; //0x138-0x144
	short xF0; //0x144-0x146
	short bootLdrFlags; //0x146-0x148
	short xam_region; //0x148-0x14A
	short xam_odd; //0x14A-0x14C
	int x14C; //0x14C-0x150
	int PolicyFlashSize; //0x150-0x154
	int x154; //0x154-0x158
	int hvStatusFlags; //0x158-0x15C
	char x15C[0x44]; //0x15C-0x1A0
	char consoleID[5]; //0x1A0-0x1A5 
	char x1A5[0x2B]; //0x1A5-0x1D0
	int hardwareFlags; //0x1D0-0x1D4
	char x1D4[0x104]; //0x1D4-0x2D8
	int footerMagic; //0x2B8-0x2DC
};

extern int xamAllocatedData;

typedef NTSTATUS(*SysChall_ExecuteCall_t)(int task, char* tableName, int tableSize, challengeResponse_s* chalResp, int bufferSize);
NTSTATUS XamLoaderExecuteAsyncChallengeHook(int challengeAddress, int task, char *tableName, int tableSize, PBYTE chalResp, int bufferSize);