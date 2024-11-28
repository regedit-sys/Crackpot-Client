#include "stdafx.h"
#include "common.h"

wchar_t wcharStackBuffer[512];
char charStackBuffer[512];

bool ini::KVProtection = true;
bool ini::cheats = false;
bool ini::customUI = true;
bool ini::offline = false;
bool ini::MSPSpoofing = true;
bool ini::NoKVMode = false;
bool ini::UseNandKV = false;
bool ini::BP_AW = true, ini::BP_GHOSTS = true, ini::BP_BO2 = true, ini::BP_GTAV = true, ini::BP_BO3 = true;
bool ini::onHost_MW2 = false, ini::CE_GTAV = false;
char ini::Passport_Email[20], ini::Passport_Password[20];

KEY_VAULT kv;
byte MACAddress[0x6];
PVOID pSecData;

sData_s sData;
cData_s cData;
NET_RESP_FETCHTIME remainingTime;