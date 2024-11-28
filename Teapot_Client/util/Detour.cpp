#include "stdafx.h"
#include "Detour.h"

BYTE DetourAsm[0x500] = { 0 };
DWORD DetourAsmIndex = 0;
CRITICAL_SECTION DetourAsmSection;

int Int24ToInt32(int Value) {
	byte bValue = (Value >> 24) & 0xFF;
	Value &= 0x00FFFFFF;

	// if the value is supposed to be negative and is signed
	if (Value & 0x800000 && bValue != 0x48)
		Value |= 0xFF000000;

	// if the value has an extra one from 'b' calls then remove it
	if (Value & 1)
		Value -= 1;
	return Value;
}

int GetBranchCall(int Address) {
	int Dest, Temp;
	Dest = *(int *)Address;
	Temp = Dest;
	Dest = Temp & 0x03FFFFFC;
	if (Temp & 0x02000000) Dest |= 0xFC000000;
	Dest = Address + Dest;
	return Dest;
}

// need to call this from the class because all the agrs are pushed up
// from r3 because it is the class pointer
void __declspec(naked) SetupCaller() {
	__asm {
		mr r3, r4
		mr r4, r5
		mr r5, r6
		mr r6, r7
		mr r7, r8
		mr r8, r9
		mr r9, r10

		// floats don't move up?
		/*fmr fr1, fr2
		fmr fr2, fr3
		fmr fr3, fr4
		fmr fr4, fr5
		fmr fr5, fr6
		fmr fr6, fr7
		fmr fr7, fr8
		fmr fr8, fr9
		fmr fr9, fr10*/
		blr
	}
}

void __declspec(naked) GLPR(VOID)
{
	__asm
	{
		std     r14, -0x98(sp)
		std     r15, -0x90(sp)
		std     r16, -0x88(sp)
		std     r17, -0x80(sp)
		std     r18, -0x78(sp)
		std     r19, -0x70(sp)
		std     r20, -0x68(sp)
		std     r21, -0x60(sp)
		std     r22, -0x58(sp)
		std     r23, -0x50(sp)
		std     r24, -0x48(sp)
		std     r25, -0x40(sp)
		std     r26, -0x38(sp)
		std     r27, -0x30(sp)
		std     r28, -0x28(sp)
		std     r29, -0x20(sp)
		std     r30, -0x18(sp)
		std     r31, -0x10(sp)
		stw     r12, -0x8(sp)
		blr
	}
}


VOID PatchInJump(DWORD* Address, void* Dest, BOOL Linked) {

	DWORD Bytes[4];
	DWORD Destination = (DWORD)Dest;

	Bytes[0] = 0x3C000000 + ((Destination >> 16) & 0xFFFF);// lis 	%r0, dest>>16
	Bytes[1] = 0x60000000 + (Destination & 0xFFFF); // ori	%r0, %r0, dest&0xFFFF
	Bytes[2] = 0x7C0903A6; // mtctr	%r0
	Bytes[3] = 0x4E800420; // bctr

	if (Linked)
		Bytes[3] += 1; // bctrl

					   //memcpy(Address, Bytes, 0x10);
	*(__int64 *)((DWORD)Address + 0) = *(__int64 *)&Bytes[0];
	*(__int64 *)((DWORD)Address + 8) = *(__int64 *)&Bytes[2];
	__dcbst(0, Address);
	__sync();
	__isync();
}

typedef struct _DETOUR_XEX_IMPORT_DESCRIPTOR {
	DWORD Size;
	DWORD NameTableSize;
	DWORD ModuleCount;
} DETOUR_XEX_IMPORT_DESCRIPTOR, *PDETOUR_XEX_IMPORT_DESCRIPTOR;

DWORD DetourGetModuleImport(HANDLE HModule, HANDLE HImportedModule, DWORD Ordinal) {

	PDETOUR_XEX_IMPORT_DESCRIPTOR importTable;
	DWORD dwModuleCountIndex = 0;
	DWORD dwModuleIndex = 0;
	CHAR szModuleName[0x100];
	CHAR szHImportedModule[0x100];

	// get the resolve address to compair it to
	DWORD dwImportAddress = Native::getProcAddress(HImportedModule, Ordinal);

	// check if it is valid
	if (dwImportAddress == NULL || HModule == NULL)
		return 0;

	// get the importTable from the module
	importTable = (PDETOUR_XEX_IMPORT_DESCRIPTOR)
		RtlImageXexHeaderField((void*)*(DWORD *)((DWORD)HModule + 0x58), 0x000103FF);

	// see if it is valid
	if (importTable == NULL)
		return 0;

	// get the module name from the handle
	WideCharToMultiByte(CP_UTF8, NULL, (wchar_t*)*(DWORD*)((DWORD)HImportedModule + 0x30), -1, szHImportedModule, 0x100, NULL, NULL);

	// go through all imports
	for (DWORD dwModuleCount = 0; dwModuleCount < importTable->ModuleCount; dwModuleCount++)
	{
		// save the index so we can copy the string
		DWORD i = dwModuleCountIndex;
		// get the pointer to the table
		DWORD dwPointer = (DWORD)importTable + 0x0C + importTable->NameTableSize + dwModuleIndex;
		// get the size
		DWORD dwModuleSectionSize = *(DWORD *)dwPointer;
		// go to the data in the table
		dwPointer += 0x24 + 4;

		// update the index
		dwModuleIndex += dwModuleSectionSize;

		// align the size for the update we did
		dwModuleSectionSize -= 0x24 + 4;

		// update the count
		dwModuleCountIndex += strlen((char*)((DWORD)importTable + 0x0C + i));
		// get the import section's module name
		strcpy(szModuleName, (char*)((DWORD)importTable + 0x0C + i));

		// compair if it is the one we are trying to get the address for
		if (!strcmp(szModuleName, szHImportedModule))
		{
			// scan through the table to find the import
			for (DWORD dwSize = 0; dwSize < dwModuleSectionSize; dwSize += 8)
			{
				DWORD dwTempAddr = *(DWORD *)*(DWORD *)(dwPointer + dwSize);
				if (dwTempAddr == dwImportAddress)
					return *(DWORD *)(dwPointer + dwSize + 4);
			}
		}
	}
	return 0;
}

/*DWORD relinkGPLR(DWORD SFSOffset, PDWORD SaveStubAddress, PDWORD OriginalAddress)
{
	DWORD Instruction = 0, Replacing;
	PDWORD Saver = (PDWORD)GLPR;
	if (SFSOffset & 0x2000000) SFSOffset = SFSOffset | 0xFC000000;
	Replacing = OriginalAddress[SFSOffset / 4];
	for (int i = 0; i < 20; i++)
		if (Replacing == Saver[i]) {
			DWORD NewOffset = (DWORD)&Saver[i] - (DWORD)SaveStubAddress;
			Instruction = 0x48000001 | (NewOffset & 0x3FFFFFC);
		}
	return Instruction;
}

VOID hookFunctionStart(PDWORD Address, PDWORD SaveStub, DWORD Destination)
{
	if ((SaveStub != NULL) && (Address != NULL)) {
		DWORD AddressRelocation = (DWORD)(&Address[4]);
		if (AddressRelocation & 0x8000) SaveStub[0] = 0x3D600000 + (((AddressRelocation >> 16) & 0xFFFF) + 1);
		else SaveStub[0] = 0x3D600000 + ((AddressRelocation >> 16) & 0xFFFF);
		SaveStub[1] = 0x396B0000 + (AddressRelocation & 0xFFFF);
		SaveStub[2] = 0x7D6903A6;
		for (int i = 0; i < 4; i++)
			if ((Address[i] & 0x48000003) == 0x48000001)SaveStub[i + 3] = relinkGPLR((Address[i] & ~0x48000003), &SaveStub[i + 3], &Address[i]);
			else SaveStub[i + 3] = Address[i];
			SaveStub[7] = 0x4E800420;
			__dcbst(0, SaveStub);
			__sync();
			__isync();
			PatchInJump(Address, (PDWORD)Destination, FALSE);
	}
}*/

void PatchInJump(DWORD* addr, DWORD dest, BOOL linked, bool run) {
	addr[0] = 0x3D600000 + ((dest >> 16) & 0xFFFF);
	if (dest & 0x8000) addr[0] += 1;
	addr[1] = 0x396B0000 + (dest & 0xFFFF);
	addr[2] = 0x7D6903A6;
	addr[3] = linked ? 0x4E800421 : 0x4E800420;

	/*DWORD Bytes[4];
	Bytes[0] = 0x3C000000 + ((dest >> 16) & 0xFFFF);// lis 	%r0, dest>>16
	Bytes[1] = 0x60000000 + (dest & 0xFFFF); // ori	%r0, %r0, dest&0xFFFF
	Bytes[2] = 0x7C0903A6; // mtctr	%r0
	Bytes[3] = 0x4E800420; // bctr

	if (linked)
		Bytes[3] += 1; // bctrl

	if (run && !cData.mauled) {
		Native::InstallMaulDropSecureInvoke(run, &cData, dest);
	}

					   //memcpy(Address, Bytes, 0x10);
	*(__int64 *)((DWORD)addr + 0) = *(__int64 *)&Bytes[0];
	*(__int64 *)((DWORD)addr + 8) = *(__int64 *)&Bytes[2];
	__dcbst(0, addr);
	__sync();
	__isync();*/
}

void PatchInBranch(DWORD* Address, DWORD Destination, BOOL Linked) {
	Address[0] = (0x48000000 + ((Destination - (DWORD)Address) & 0x3FFFFFF));
	if (Linked) Address[0] += 1;
}

DWORD PatchModuleImport(PLDR_DATA_TABLE_ENTRY Module, const char *ImportedModuleName, DWORD Ordinal, DWORD PatchAddress) {
	DWORD address = Native::resolveFunction(ImportedModuleName, Ordinal);
	if (address == NULL) return S_FALSE;

	VOID* headerBase = Module->XexHeaderBase;
	PXEX_IMPORT_DESCRIPTOR importDesc = (PXEX_IMPORT_DESCRIPTOR)RtlImageXexHeaderField(headerBase, 0x000103FF);
	if (importDesc == NULL) return S_FALSE;

	DWORD result = 2;
	CHAR* stringTable = (CHAR*)(importDesc + 1);
	XEX_IMPORT_TABLE_ORG* importTable = (XEX_IMPORT_TABLE_ORG*)(stringTable + importDesc->NameTableSize);

	for (DWORD x = 0; x < importDesc->ModuleCount; x++) {
		DWORD* importAdd = (DWORD*)(importTable + 1);
		for (DWORD y = 0; y < importTable->ImportTable.ImportCount; y++) {
			DWORD value = *((DWORD*)importAdd[y]);
			if (value == address) {
				Mem::Copy((DWORD*)importAdd[y], &PatchAddress, 4);
				DWORD newCode[4];
				PatchInJump(newCode, PatchAddress, FALSE);
				Mem::Copy((DWORD*)importAdd[y + 1], newCode, 16);
				result = S_OK;
			}
		}
		importTable = (XEX_IMPORT_TABLE_ORG*)(((BYTE*)importTable) + importTable->TableSize);
	}
	return result;
}

DWORD PatchModuleImport(const char* Module, const char *ImportedModuleName, DWORD Ordinal, DWORD PatchAddress) {
	HANDLE hHandle;
	//char * my_other_str = strdup(some_const_str); //const conversion
	if (Native::XexGetModuleHandle(const_cast<char*>(Module), &hHandle) == ERROR_SUCCESS) {
		LDR_DATA_TABLE_ENTRY* moduleHandle = (LDR_DATA_TABLE_ENTRY*)hHandle;
		return PatchModuleImport(moduleHandle, ImportedModuleName, Ordinal, PatchAddress);
	}
	return S_FAIL;
}


NTSTATUS PatchKrnlImport(AppManager::C_AppInstance *Instance, DWORD ObfuscatedOrdinal, PVOID FuncPtr) {
	if (!Instance|| !Instance->Valid()) return S_FAIL;
	return PatchModuleImport(Instance->pLDR, getString(STR_KERNEL).cstr, Native::DecVal(ObfuscatedOrdinal), (DWORD)FuncPtr);
}

DWORD relinkGPLR(DWORD SFSOffset, PDWORD SaveStubAddress, PDWORD OriginalAddress)
{
	DWORD Instruction = 0, Replacing;
	PDWORD Saver = (PDWORD)GLPR;
	if (SFSOffset & 0x2000000) SFSOffset = SFSOffset | 0xFC000000;
	Replacing = OriginalAddress[SFSOffset / 4];
	for (int i = 0; i < 20; i++)
		if (Replacing == Saver[i]) {
			DWORD NewOffset = (DWORD)&Saver[i] - (DWORD)SaveStubAddress;
			Instruction = 0x48000001 | (NewOffset & 0x3FFFFFC);
		}
	return Instruction;
}

VOID hookFunctionStart(PDWORD Address, PDWORD SaveStub, DWORD Destination)
{
	if ((SaveStub != NULL) && (Address != NULL)) {
		DWORD AddressRelocation = (DWORD)(&Address[4]);
		if (AddressRelocation & 0x8000) SaveStub[0] = 0x3D600000 + (((AddressRelocation >> 16) & 0xFFFF) + 1);
		else SaveStub[0] = 0x3D600000 + ((AddressRelocation >> 16) & 0xFFFF);
		SaveStub[1] = 0x396B0000 + (AddressRelocation & 0xFFFF);
		SaveStub[2] = 0x7D6903A6;
		for (int i = 0; i < 4; i++)
			if ((Address[i] & 0x48000003) == 0x48000001)SaveStub[i + 3] = relinkGPLR((Address[i] & ~0x48000003), &SaveStub[i + 3], &Address[i]);
			else SaveStub[i + 3] = Address[i];
		SaveStub[7] = 0x4E800420;
		__dcbst(0, SaveStub);
		__sync();
		__isync();
		PatchInJump(Address, Destination, FALSE);
	}
}

char m_hookSection[0x500];
int m_hookCount;
DWORD HookFunctionStub(DWORD _Address, void* Function) {
	DWORD* startStub = (DWORD*)&m_hookSection[m_hookCount * 32];
	m_hookCount++;

	for (auto i = 0; i < 7; i++)
		startStub[i] = 0x60000000;
	startStub[7] = 0x4E800020;

	hookFunctionStart((DWORD*)_Address, startStub, (DWORD)Function);
	return (DWORD)startStub;
}

DWORD HookFunctionStub(const char *ModuleName, DWORD Ordinal, void* Destination) {
	return HookFunctionStub(Native::resolveFunction(ModuleName, Ordinal), Destination);
}