#pragma once
#include "xbox/kv.h"
#pragma warning(disable:4172)
#pragma warning(disable:4078)
#pragma warning(disable:4254)
#pragma warning(disable:4800)
#pragma warning(disable:4244)

extern wchar_t wcharStackBuffer[512];
extern char charStackBuffer[512];

#define XNOTIFY_MAX_LEN 100
#define SECDATA_SIZE 0x285
#define MAGIC_THREAD_SPEED 0x18000424 //c0z-infernus??? im not sure.

typedef enum _AUTHFLAG : short {
	AUTHFLAG_ACTIVE,
	AUTHFLAG_EXPIRED,
	AUTHFLAG_NOEXIST,
	AUTHFLAG_REBOOTPENDING,
	AUTHFLAG_NOTCONNECTED,
	AUTHFLAG_UPDATING
}AUTHFLAG;

namespace ini {
	extern bool NoKVMode;
	extern bool UseNandKV;
	extern bool KVProtection;
	extern bool cheats;
	extern bool customUI;
	extern bool offline;
	extern bool MSPSpoofing;
	extern bool onHost_MW2, CE_GTAV;
	extern bool BP_AW, BP_GHOSTS, BP_BO2, BP_GTAV, BP_BO3;
	extern char Passport_Email[20], Passport_Password[20];
}

extern KEY_VAULT kv;
extern byte MACAddress[0x6];
extern PVOID pSecData;
extern sData_s sData;
extern cData_s cData;
extern NET_RESP_FETCHTIME remainingTime;
