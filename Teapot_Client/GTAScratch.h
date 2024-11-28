#pragma once
#include "stdafx.h"
#include "common.h"

bool CrashPatchBeta0(void* r3, unsigned long* r4, unsigned long r5, unsigned long r6);
detour<bool> TourCrashBLRDetection;
detour<bool> TourCrashPatchBeta0;
detour<bool> TourCrashPatchBeta1;
detour<long> TourCrashPatchBeta2;
detour<bool> TourCrashPatchBeta3;
detour<void> TourCrashPatchBeta4;
detour<void> TourCrashPatchBeta5;
detour<void> TourCrashPatchBeta6;
bool player_block_list[18];


// This blocks multiple heap overruns that happen when the crash is activated.
bool CrashPatchBeta0(void* r3, unsigned long* r4, unsigned long r5, unsigned long r6) {
	unsigned long lr = NULL;
	__asm { mflr lr };
	bool ret = ((bool(*)(void*, unsigned long*, unsigned long, unsigned long))0x826275D0)(r3, r4, r5, r6);
	if (lr == 0x830229DC && *r4 > 8) {
		*r4 = NULL;
	}
	if (lr == 0x8302BCEC && *r4 > 10) {
		*r4 = NULL;
	}
	if (lr == 0x830BABA4 && *r4 > 1) {
		*r4 = NULL;
	}
	return ret;
}

// This blocks bad objects
bool CrashPatchBeta1(void* r3, unsigned char* r4, unsigned long r5, unsigned short r6, unsigned short r7, unsigned long r8, unsigned long r9, int r10) {
	unsigned char pid = r4[0x29];
	if (pid >= 0x00 && pid < 0x10) {
		if (player_block_list[pid]) {
			return false;
		}
		if (r6 < 0 || r6 > 13) {
			player_block_list[pid] = true;
			return false;
		}
	}
	return TourCrashPatchBeta1.callOriginal(r3, r4, r5, r6, r7, r8, r9, r10);
}

// This blocks bad objects
static unsigned long(*CrashBLRDetection)(void*, unsigned char*, unsigned long, unsigned short, unsigned short, unsigned long, unsigned long, int);
unsigned long CrashBLRDetectionHook(void * r3, unsigned char * r4, unsigned long r5, unsigned short r6, unsigned short r7, unsigned long r8, unsigned long r9, int r10) {
	if (r6 < 0 || r6> 13) {//Need to make so it shows the name not just Unknown
		debug_sys("Unknown has sent a Lobby Crash", "");
		return 6;
	}
	return TourCrashBLRDetection.callOriginal(r3, r4, r5, r6, r7, r8, r9, r10);
}

unsigned long CrashPatchBeta2(void* r3, unsigned char* r4, unsigned long r5, unsigned short r6, unsigned short r7, unsigned long r8, unsigned long r9, int r10) {
	unsigned char pid = r4[0x29];
	if (pid >= 0x00 && pid < 0x10) {
		if (player_block_list[pid]) {
			return 6;
		}
		if (r6 < 0 || r6 > 13) {
			player_block_list[pid] = true;
			return 6;
		}
	}
	return TourCrashPatchBeta2.callOriginal(r3, r4, r5, r6, r7, r8, r9, r10);
}

// This blocks bad objects
bool CrashPatchBeta3(void* r3, unsigned char* r4, unsigned long r5, unsigned short r6, unsigned long r7) {
	unsigned char pid = r4[0x29];
	if (pid >= 0x00 && pid < 0x10) {
		if (player_block_list[pid]) {
			return false;
		}
	}
	return TourCrashPatchBeta3.callOriginal(r3, r4, r5, r6, r7);
}

// This blocks bad objects
void CrashPatchBeta4(void* r3, unsigned short* r4, bool r5, unsigned char* r6, unsigned long r7) {
	if (!r5) {
		unsigned char pid = r6[0x29];
		if (pid >= 0x00 && pid < 0x10) {
			if (player_block_list[pid]) {
				return;
			}
			if (r4[0x1B] < 0 || r4[0x1B] > 13) {
				player_block_list[pid] = true;
				return;
			}
		}
		TourCrashPatchBeta4.callOriginal(r3, r4, r5, r6, r7);
	}
}

// This blocks bad events
void CrashPatchBeta5(void* r3, void* r4, void* r5, unsigned char* r6, unsigned long r7, unsigned long r8, unsigned long r9, unsigned long r10) {
	unsigned char pid = r6[0x29];
	if (pid >= 0x00 && pid < 0x10) {
		if (player_block_list[pid]) {
			*(unsigned long*)(0x83E373AC) = NULL;
			return;
		}
	}
	TourCrashPatchBeta5.callOriginal(r3, r4, r5, r6, r7, r8, r9, r10);
}

// This removes the block
void CrashPatchBeta6(unsigned char* r3) {
	unsigned char pid = r3[0x29];
	if (pid >= 0x00 && pid < 0x10) {
		player_block_list[pid] = false;
	}
	TourCrashPatchBeta6.callOriginal(r3);
}


void SetupHooks() {
	debug("GTAV Hooks applied");

	*(unsigned long*)(0x82028580) = (unsigned long)(CrashPatchBeta0);
	//TourCrashBLRDetection.setupDetour(0x83068690, (PVOID)CrashBLRDetectionHook);
	TourCrashPatchBeta1.setupDetour(0x83068690, (PVOID)CrashPatchBeta2);
	TourCrashPatchBeta2.setupDetour(0x83068A60, (PVOID)CrashPatchBeta3);
	TourCrashPatchBeta3.setupDetour(0x830EA8B8, (PVOID)CrashPatchBeta4);
	TourCrashPatchBeta4.setupDetour(0x835DB018, (PVOID)CrashPatchBeta5);
	TourCrashPatchBeta5.setupDetour(0x82FEA388, (PVOID)CrashPatchBeta6);
}