#include "stdafx.h"
#include "xosc.h"
#include "util.h"
#include "challenges.h"
#include "globals.h"
#include "HvPeekPoke.h"

int xamAllocatedData = 0;

NTSTATUS(*SysChall_SetupSerialNumberHash)(PBYTE chalResp) = (NTSTATUS(*)(PBYTE))0x900153D8;

extern "C" {
	NTSTATUS XamAlloc(int dest, int size, int data);
	void XamFree(int data);
	NTSTATUS IoSynchronousDeviceIoControlRequest(int io_request, void** deviceObject, void* bufferInput, int length, int r7, int r8, void* output);
	void HalReadWritePCISpace(int r3, int r4, int r5, int r6, void* buffer, int length, bool WritePCISpace);
}

NTSTATUS SysChall_GetDeviceSize(char* path, char* chalResp) {
	*(int*)chalResp = 0;

	HANDLE file = 0;
	NTSTATUS status = STATUS_SUCCESS;

	ANSI_STRING deviceName = { 0 };
	OBJECT_ATTRIBUTES pObjAttr = { 0 };
	IO_STATUS_BLOCK statusBlock = { 0 };
	FILE_FS_SIZE_INFORMATION sizeInfo = { 0 };

	RtlInitAnsiString(&deviceName, path);

	InitializeObjectAttributes(&pObjAttr, &deviceName, OBJ_CASE_INSENSITIVE, 0);

	if (NT_SUCCESS(NtOpenFile(&file, (SYNCHRONIZE | 1), &pObjAttr, &statusBlock, FILE_SHARE_READ, XOSC_DEVICESIZE_OPEN_OPTIONS)))
		if (NT_SUCCESS(status = NtQueryVolumeInformationFile(file, &statusBlock, &sizeInfo, 0x18, FileFsSizeInformation)))
			*(int*)chalResp = sizeInfo.TotalAllocationUnits.LowPart;

	NtClose(file);
	return status;
}

NTSTATUS SysChall_GetStorageDeviceSizes(int task, char* tableName, int tableSize, PBYTE chalResp, int bufferSize) {
	*(int*)(chalResp + 0x8) |= 0x10;

	SysChall_GetDeviceSize("\\Device\\Mu0\\", (char*)(chalResp + 0x2A8));
	SysChall_GetDeviceSize("\\Device\\Mu1\\", (char*)(chalResp + 0x2AC));
	SysChall_GetDeviceSize("\\Device\\BuiltInMuSfc\\", (char*)(chalResp + 0x2B0));
	SysChall_GetDeviceSize("\\Device\\BuiltInMuUsb\\Storage\\", (char*)(chalResp + 0x2B4));
	SysChall_GetDeviceSize("\\Device\\Mass0PartitionFile\\Storage\\", (char*)(chalResp + 0x2B8));
	SysChall_GetDeviceSize("\\Device\\Mass1PartitionFile\\Storage\\", (char*)(chalResp + 0x2BC));
	SysChall_GetDeviceSize("\\Device\\Mass2PartitionFile\\Storage\\", (char*)(chalResp + 0x2C0));

	return STATUS_SUCCESS;
}

NTSTATUS SysChall_GetConsoleCertificate(int task, char* tableName, int tableSize, PBYTE chalResp, int bufferSize) {
	memset((char*)chalResp, 0, 0x140);

	int certBuffer = 0;
	int certificateSize = 0x140;

	NTSTATUS status = STATUS_SUCCESS;
	if (!NT_SUCCESS(status = XeKeysGetKey(XEKEY_XEIKA_CERTIFICATE, &certBuffer, (PDWORD)&certificateSize)))
		return *(int*)(chalResp + 0x10) = status;

	/*if (certificateSize <= 0x110 || *(int*)(certBuffer + 0x110) != 0x4F534947 || *(int*)(chalResp + 0x114) < 1) {
		debug("SysChall_GetConsoleCertificate: certficate error\n");
		return *(int*)(chalResp + 0x10) = STATUS_INVALID_PARAMETER_1;
	}*/

	*(long long*)(chalResp + 0x8) |= 2;
	memcpy((void*)(chalResp + 0x114), (void*)(chalResp + 0x118), 0x24);

	*(int*)(chalResp + 0x80) = *(char*)(certBuffer + 0x117);
	*(int*)(chalResp + 0x14) = status;
	return status;
}

NTSTATUS SysChall_GetDeviceControlRequest(int task, char* tableName, int tableSize, PBYTE chalResp, int bufferSize) {
	*(long long*)(chalResp + 0x8) |= 1;

	NTSTATUS status = STATUS_SUCCESS;
	STRING objectPath = { 0xE, 0xF, "\\Device\\Cdrom0" };

	void* deviceObject = 0;
	char partitionInfo[0x24] = { 0 };

	if (!NT_SUCCESS(status = ObReferenceObjectByName(&objectPath, 0, 0, 0, &deviceObject))) {
		*(int*)(chalResp + 0x10) = status;
		return status;
	}

	*(long long*)(chalResp + 0xF0) = -1;
	*(long long*)(chalResp + 0xF8) = -1;
	*(long long*)(chalResp + 0x100) = -1;
	*(long long*)(chalResp + 0x108) = -1;
	*(int*)(chalResp + 0x110) = -1;

	*(short*)((int)deviceObject + 0x20) = *(short*)(chalResp + 0xF0);

	*(char*)partitionInfo = 0x24;
	*(char*)(partitionInfo + 0x7) = 1;
	*(int*)(partitionInfo + 0x8) = 0x24;
	*(char*)(partitionInfo + 0x14) = 0x12;
	*(char*)(partitionInfo + 0x18) = 0x24;
	*(char*)(partitionInfo + 0x19) = 0xC0;

	status = IoSynchronousDeviceIoControlRequest(IOCTL_DISK_VERIFY, &deviceObject, partitionInfo, 0x24, 0, 0, 0);
	ObDereferenceObject(deviceObject);

	*(int*)(chalResp + 0x10) = status;
	return status;
}

NTSTATUS SysChall_SetupSataDiskHash(PBYTE chalResp) {
	PQWORD input = 0;
	XECRYPT_RSA rsa = { 0 };

	char hash[0x14] = { 0 };
	int rsaSecuritySize = 0x110;

	*(long long*)(chalResp + 0x8) |= 0x80;

	int* r30 = (int*)(chalResp + 0x1D4);

	memset((int*)(chalResp + 0x1D4), 0, 4);
	memset((int*)(chalResp + 0x1D8), 0, 4);
	memset((int*)(chalResp + 0x1DC), 0, 4);
	memset((int*)(chalResp + 0x1E0), 0, 4);
	memset((int*)(chalResp + 0x1E4), 0, 4);

	memset((long long*)(chalResp + 0x1E8), 0, 8);
	memset((long long*)(chalResp + 0x1F0), 0, 8);
	memset((long long*)(chalResp + 0x1F8), 0, 8);
	memset((long long*)(chalResp + 0x200), 0, 8);
	memset((long long*)(chalResp + 0x208), 0, 8);
	memset((long long*)(chalResp + 0x210), 0, 8);

	memset((int*)(chalResp + 0x218), 0, 4);
	memset((int*)(chalResp + 0x1D0), XboxHardwareInfo->Flags, 4);

	if ((XboxHardwareInfo->Flags & 0x20) == 0)
		return STATUS_SUCCESS;

	for (int i = 0; i < 5; i++) {
		memcpy((void*)xamAllocatedData, (void*)0x8E038400, 0x15C);
		XeCryptSha((PBYTE)xamAllocatedData, 0x5C, 0, 0, 0, 0, (PBYTE)hash, 0x14);

		if (NT_SUCCESS(XeKeysGetKey(XEKEY_CONSTANT_SATA_DISK_SECURITY_KEY, &rsa, (PDWORD)&rsaSecuritySize))) {
			if (rsaSecuritySize == 0x110 || rsa.cqw == 0x20) {
				input = (PQWORD)(xamAllocatedData + 0x5C);
				XeCryptBnQw_SwapDwQwLeBe(input, input, 0x20);

				if (XeCryptBnQwNeRsaPubCrypt(input, input, &rsa)) {
					XeCryptBnQw_SwapDwQwLeBe(input, input, 0x20);

					if (XeCryptBnDwLePkcs1Verify((PBYTE)hash, (PBYTE)input, 0x100))
						break;
				}
			}
		}
	}
	if (xamAllocatedData == 0)
		return STATUS_SUCCESS;

	for (int i = 5; i != 0; i--)
		r30[i] = *(int*)(xamAllocatedData + i);

	memcpy((int*)(chalResp + 0x1E8), (int*)(xamAllocatedData + 0x14), 4);
	memcpy((int*)(*(int*)(chalResp + 0x1E8) + 0x4), (int*)(xamAllocatedData + 0x14), 4);
	memcpy((int*)(chalResp + 0x1F0), (void*)(xamAllocatedData + 0x1C), 0x28);

	return STATUS_SUCCESS;
}

void SysChall_GetPCIEState(int task, char* tableName, int tableSize, PBYTE chalResp, int bufferSize) {
	char data[0x100] = { 0 };
	HalReadWritePCISpace(0, 2, 0, 0, data, 0x100, 0);

	long long r9 = (((*(char*)(data + 0x8) & ~0xFFFF00) | ((*(short*)(data + 0x2) << 8) & 0xFFFF00) << 8) & 0xFFFFFFFFFFFFFFFF);
	long long r10 = (((*(char*)(data + 0xB) & ~0xFFFF00) | ((*(short*)(data + 0x4) << 8) & 0xFFFF00) << 8) & 0xFFFFFFFFFFFFFFFF);

	*(int*)(chalResp + 0x34) = 0x40000012;
	*(long long*)(chalResp + 0x8) = (*(long long*)(chalResp + 0x8) | 0x100);
	*(long long*)(chalResp + 0x170) = ((((r9 | XboxHardwareInfo->PCIBridgeRevisionID) << 32) | r10) | *(char*)(data + 0xA));
}

NTSTATUS SysChall_SetupChallengeHashes(int task, char* tableName, int tableSize, PBYTE chalResp, int bufferSize) {
	int xConfigDevFlags = 0;
	WORD settingSize = 6;
	byte unknownBuffer[0x10] = { 0 };
	byte securityDigest[0x14];
	byte macAddress[0x6];
	byte smcVersion[0x5];

	PLDR_DATA_TABLE_ENTRY hxam = (PLDR_DATA_TABLE_ENTRY)GetModuleHandle(MODULE_XAM);
	PLDR_DATA_TABLE_ENTRY hkrnl = (PLDR_DATA_TABLE_ENTRY)GetModuleHandle(MODULE_KERNEL);
	PLDR_DATA_TABLE_ENTRY hcurrent = (PLDR_DATA_TABLE_ENTRY)GetModuleHandle(0);

	*(PWORD)(chalResp + 0x148) = HvPeekWORD(kvAddr + 0xC8);
	*(PWORD)(chalResp + 0x14A) = HvPeekWORD(kvAddr + 0x1C);
	*(PDWORD)(chalResp + 0x150) = kv.PolicyFlashSize;
	*(PDWORD)(chalResp + 0x158) = challenges::HVStatusFlags();
	memcpy((int*)(chalResp + 0x14C), (int*)0x8E03861C, 4); //x14C
	memcpy((int*)(chalResp + 0x154), (int*)0x8E038614, 4); //x154
	memcpy((long long*)(chalResp + 0x180), (long long*)0x8E038630, 8); //x15C[0x24]
	memcpy((long long*)(chalResp + 0x188), (long long*)0x8E038638, 8); //x15C[0x2C]
	memcpy((long long*)(chalResp + 0x190), (long long*)0x8E038640, 8); //x15C[0x34]
	memcpy((long long*)(chalResp + 0x198), (long long*)0x8E038678, 8); //x15C[0x3C]
	memcpy((long long*)(chalResp + 0x160), (int*)0x8E038704, 8); //x15C[0x4]
	memcpy((int*)(chalResp + 0x168), (int*)0x8E038708, 4); //x15C[0xC]
	memcpy((int*)(chalResp + 0x178), (int*)0x8E03870C, 4); //x15C[0x1C]
	memcpy((int*)(chalResp + 0x17C), (int*)0x8E038710, 4); //x15C[0x20]
	memcpy((int*)(chalResp + 0x2C4), (int*)0x8E03FEAC, 4); //x15C[0xF0]
	memcpy(securityDigest, (byte*)0x8E03AA40, 0x14);
	unknownBuffer[0xE] &= 0xF8;

	if (hxam && hkrnl && hcurrent) {
		IMAGE_XEX_HEADER* xamHeader = (IMAGE_XEX_HEADER*)(hxam->XexHeaderBase);
		IMAGE_XEX_HEADER* krnlHeader = (IMAGE_XEX_HEADER*)(hkrnl->XexHeaderBase);
		IMAGE_XEX_HEADER* currentHeader = (IMAGE_XEX_HEADER*)(hcurrent->XexHeaderBase);

		if (xamHeader && krnlHeader && currentHeader) {
			XEX_SECURITY_INFO* securityInfo;
			DWORD dataSize;

			securityInfo = (XEX_SECURITY_INFO*)(xamHeader->SecurityInfo);
			dataSize = ((xamHeader->SizeOfHeaders - (DWORD)&securityInfo->AllowedMediaTypes) + (DWORD)xamHeader);
			XeCryptSha((PBYTE)&securityInfo->AllowedMediaTypes, dataSize, securityDigest, 0x14, unknownBuffer, 0x10, securityDigest, 0x14);
			xConfigDevFlags = XCONFIG_DEVKIT_USBD_ROOT_HUB_PORT_DISABLE_MASK;

			if (NT_SUCCESS(ExGetXConfigSetting(XCONFIG_SECURED_CATEGORY, XCONFIG_SECURED_MAC_ADDRESS, macAddress, 0x6, &settingSize))) {
				securityInfo = (XEX_SECURITY_INFO*)(krnlHeader->SecurityInfo);
				dataSize = ((krnlHeader->SizeOfHeaders - (DWORD)&securityInfo->AllowedMediaTypes) + (DWORD)krnlHeader);
				XeCryptSha((PBYTE)&securityInfo->AllowedMediaTypes, dataSize, securityDigest, 0x14, macAddress, 0x6, securityDigest, 0x14);
				xConfigDevFlags |= (XCONFIG_DEVKIT_XAM_FEATURE_ENABLE_DISABLE_MASK & 0xFF);
			}

			//getSMCVersion(smcVersion);
			securityInfo = (XEX_SECURITY_INFO*)(currentHeader->SecurityInfo);
			dataSize = ((currentHeader->SizeOfHeaders - (DWORD)&securityInfo->AllowedMediaTypes) + (DWORD)currentHeader);
			XeCryptSha((PBYTE)&securityInfo->AllowedMediaTypes, dataSize, securityDigest, 0x14, smcVersion, 0x5, securityDigest, 0x14);
			xConfigDevFlags |= (XCONFIG_DEVKIT_MAX & 0xFF);
		}
	}

	XeCryptSha((PBYTE)0x900101A3, 0x8E59, (PBYTE)securityDigest, 0x14, 0, 0, (PBYTE)securityDigest, 0x14);
	securityDigest[0] = xConfigDevFlags;
	memcpy((void*)(chalResp + 0x60), securityDigest, 0x10);
	memcpy((void*)(chalResp + 0x70), (void*)0x8E03AA50, 0x10);
	return STATUS_SUCCESS;
}


NTSTATUS SysChall_Execute(int task, char* tableName, int tableSize, PBYTE chalResp, int bufferSize) {
	if (*chalResp == 0 || bufferSize == 0 || bufferSize < 0x2E0) {
		debug("[SysChall_Execute] Gay ass shit failed\n");
		return E_INVALIDARG;
	}

	NTSTATUS exeIdStatus = 0;
	PXEX_EXECUTION_ID executionId;

	memset((char*)chalResp, 0, bufferSize);
	memset((char*)chalResp, 0xAA, 0x2E0);
	memset((long long*)(chalResp + 0x8), 0, 8);
	memset((char*)(chalResp + 0x4), 9, 4);
	memset((int*)(chalResp + 0x2D8), 0x5F534750, 4);

	XamAlloc(0x20000, 0x8000, xamAllocatedData);

	SysChall_GetDeviceControlRequest(task, tableName, tableSize, chalResp, bufferSize);
	SysChall_GetConsoleCertificate(task, tableName, tableSize, chalResp, bufferSize);
	SysChall_SetupChallengeHashes(task, tableName, tableSize, chalResp, bufferSize);
	SysChall_GetStorageDeviceSizes(task, tableName, tableSize, chalResp, bufferSize);
	SysChall_GetPCIEState(task, tableName, tableSize, chalResp, bufferSize);

	debug("[SysChal_Execute] Filled response\n");

	if (NT_SUCCESS(exeIdStatus = XamGetExecutionId(&executionId))) {
		*(NTSTATUS*)(chalResp + 0x18) = exeIdStatus;
		memcpy((void*)(chalResp + 0x38), executionId, 0x18);
		XamLoaderGetMediaInfo((PDWORD)(chalResp + 0x84), (PDWORD)(chalResp + 0x88));
		*(long long*)(chalResp + 0x8) |= 0x4;
	}

	int taskId = (((((*(int*)(chalResp + 0x88) - *(int*)(chalResp + 0x44)) + 1)) * 2) & task);

	if (XamLoaderIsTitleTerminatePending()) {
		__asm {
			li r12, 1
			ld r11, 8(r6)
			stw r22, 0(r6)
			extldi r12, r12, 64, 62
			or r11, r11, r12
			std r11, 8(r6)
		}
	}

	if (XamTaskShouldExit()) {
		__asm {
			li r12, 1
			ld r11, 8(r6)
			stw r22, 0(r6)
			extldi r12, r12, 64, 61
			or r11, r11, r12
			std r11, 8(r6)
		}
	}

	if (xamAllocatedData)
		XamFree(xamAllocatedData);

	memset((int*)chalResp, taskId, 4);
	return STATUS_SUCCESS;
}

void XeCryptShaHook(const PBYTE pbInp1, DWORD cbInp1, const PBYTE pbInp2, DWORD cbInp2, const PBYTE pbInp3, DWORD cbInp3, PBYTE pbOut, DWORD cbOut) {
	int src = 0;
	_asm mflr src
	if (src == 0x90015A44 || src == 0x90015AD0 || src == 0x90015B40 || src == 0x90015B74) {
		char *tmp;
		strncpy(tmp, "0x%08X_pbInp1.bin", src);
		debug("\n\r\nXeCryptShaHook - @ 0x%08X", src);
		debug("pbInp1[0x%X] = %08X", cbInp1, pbInp1);
		debug("pbInp2[0x%X] = %s", cbInp2, formatBytes(pbInp2, cbInp2));
		debug("pbInp3[0x%X] = %s", cbInp3, formatBytes(pbInp3, cbInp3));
		debug("End of XeCryptShaHook-----------\n\n");
		dumpBuffer(tmp, pbInp1, cbInp1);
	}
	XeCryptSha(pbInp1, cbInp1, pbInp2, cbInp2, pbInp3, cbInp3, pbOut, cbOut);
}

NTSTATUS XamLoaderExecuteAsyncChallengeHook(int challengeAddress, int task, char *tableName, int tableSize, PBYTE chalResp, int bufferSize) {
	typedef DWORD(*ExecuteSupervisorChallenge_t)(DWORD task, char* tableName, DWORD tableSize, PBYTE chalResp, DWORD bufferSize);
	DWORD ret;
	ExecuteSupervisorChallenge_t ExecuteSupervisorChallenge = (ExecuteSupervisorChallenge_t)challengeAddress;
	PatchModuleImport("xosc9v2.xex", "xboxkrnl.exe", 402, (DWORD)XeCryptShaHook);
	PatchInJump((PDWORD)0x900158E8, (DWORD)SysChall_SetupChallengeHashes, false);
	ret = ExecuteSupervisorChallenge(task, tableName, tableSize, chalResp, bufferSize);

	//ret = SysChall_Execute(task, tableName, tableSize, chalResp, bufferSize);
	dumpBuffer("xosc", chalResp, bufferSize);
	xNotifyDashLoad(L"XOS Challenge Executed!");
	return ret;
}

void sub_2210() {
	XECRYPT_SHA_STATE sha;
	XeCryptShaInit(&sha);

	int var_38 = *(int*)((0x200000000 + 1) << 16);
	int r29 = (((0x200000000 + 1) << 16) + 0x710);
	int r10 = ((*(int*)(r29 + 4) + 0x7F) & 0xFFFFFF80);
	int r31 = ((*(int*)r29 + 0x7F) & 0xFFFFFF80);

	for (int i = 0; i < 6; i++) {
		if (r31 < r10) {
			int length = (r10 - r31);

			int r11 = (((r31 & 0xFFFFFFFF) >> 16) * (r10 >> 16));
			int r4 = ((length >> 7) & 0xFFFFFFFFFF);
			int r3 = (r11 + ((r31 & 0xFFFFFFFF) & 0xFFFFFFFFFFFF));
		}
		r29 += 8;
	}
}