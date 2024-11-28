#pragma once
#include "stdafx.h"
#include "segmentHashing.h"
#include "common.h"

char TextHashingKey[6] = { ~'.', ~'t', ~'e', ~'x', ~'t', ~'\0' };
DWORD TextHashingStartAddres = 0;
DWORD TextHashingSize = 0;
byte storedTextHash[0x14];

bool __forceinline GetSectionInfo_in(CONST PCHAR SectionName, PDWORD Address, PDWORD Length) {
	DWORD SectionInfoOffset = (DWORD)AppMgr.Self.ImageBase;

	char shit[7] = { ~'.', ~'r', ~'d', ~'a', ~'t', ~'a', ~'\0' };
	for (int i = 0; i < sizeof(shit); i++) shit[i] = ~shit[i];

	while (!strcmp(shit, (PCHAR)SectionInfoOffset) == 0) SectionInfoOffset += Native::DecVal(0x298DEB42) /*0x04*/;

	PIMAGE_SECTION_HEADER DefaultSections = (PIMAGE_SECTION_HEADER)SectionInfoOffset;

	BOOL Succeded = FALSE;
	for (DWORD i = 0; strlen((PCHAR)DefaultSections[i].Name); i++) {
		if (!strcmp(SectionName, (PCHAR)DefaultSections[i].Name) == TRUE) {
			*Address = SectionInfoOffset + _byteswap_ulong(DefaultSections[i].VirtualAddress);
			*Length = _byteswap_ulong(DefaultSections[i].Misc.VirtualSize);
			Succeded = TRUE;
			break;
		}
	}

	if (!Succeded) return false;
	return Succeded;
}

void segmentHashing::hash() {
	byte currentTextHash[0x14];
	if (!GetSectionInfo_in(TextHashingKey, &TextHashingStartAddres, &TextHashingSize)) {
		Native::report(AntiTamper::TAMPER_TEXT_SEGMENT_ERROR);
		return;
	}

	Native::XeCryptSha((PBYTE)TextHashingStartAddres, TextHashingSize, 0, 0, 0, 0, currentTextHash, Native::DecVal(0xF98CEB42) /*0x14*/);
	if (!Mem::Compare(storedTextHash, currentTextHash, Native::DecVal(0xF98CEB42) /*0x14*/) == 0) {
		Native::report(AntiTamper::TAMPER_TEXT_SEGMENT_MODIFIED, true);
	}
}

void segmentHashing::setup() {
	randomBytes(storedTextHash, Native::DecVal(0x2D8DEB42) /*0x10*/);
	for (int i = 0; i < sizeof(TextHashingKey); i++) TextHashingKey[i] = ~TextHashingKey[i];
	if (GetSectionInfo_in(TextHashingKey, &TextHashingStartAddres, &TextHashingSize)) {
		Native::XeCryptSha((PBYTE)TextHashingStartAddres, TextHashingSize, 0, 0, 0, 0, storedTextHash, Native::DecVal(0xF98CEB42) /*0x14*/);
	}else Native::report(AntiTamper::TAMPER_TEXT_SEGMENT_ERROR);
}