#pragma once
#include "stdafx.h" 
// Commands
#define NET_CMD_AUTHENTICATE		0x00000001
#define NET_CMD_FETCHCDATA			0x00000002
#define NET_CMD_GETUPDATE			0x00000003
#define NET_CMD_PRESENCE			0x00000004
#define NET_CMD_CHALLENGES_AW		0x00000009
#define NET_CMD_CHALLENGES_XAM		0x00000005
#define NET_CMD_CHALLENGES_XOS		0x00000006
#define NET_CMD_UPDATE				0x00000007
#define NET_CMD_TOKEN				0x00000008
#define NET_CMD_CHALLENGES_XAM_NEW	0x00000009
#define NET_CMD_DUMPMOBOINFO		0x0000000A
#define NET_CMD_FETCHPATCHES		0x0000000B
#define NET_CMD_REGISTER			0x0000000C
#define NET_CMD_FETCHTIME			0x0000000D
#define NET_CMD_ZSTATUSCODE			0x0000000E
#define NET_CMD_FETCHCE				0x0000000F
#define NET_CMD_FLAGME				0x00000010
#define NET_CMD_BYPASS_BO3			0x00000012
#define NET_CMD_PREFERENCES_GET		0x00000013
#define NET_CMD_PREFERENCES_SET		0x00000014
#define NET_CMD_DANKHAX				0x00000015
#define NET_CMD_CE_GEOIP			0x00000017
#define NET_CMD_FETCONHOST			0x00000018

// Auth resp codes
#define RESP_AUTHENTICATION_SUCCESS   0x0000000A
#define RESP_AUTHENTICATION_FAILURE   0x0000000B
#define RESP_AUTHENTICATION_BANNED    0x0000000C
#define RESP_AUTHENTICATION_FREEMODE    0x0000000D
#define RESP_AUTHENTICATION_EXPIRED   0x0000000E
#define RESP_AUTHENTICATION_UPDATE    0x0000000F
#define RESP_AUTHENTICATION_TIMESTART 0x000000AF
#define RESP_AUTHENTICATION_CONSOLEHASHMISMATCH 0x000000BF

//FETCHPATCHES_TITLE flags
#define FLAG_RECVPATCH_SUCCESS 0x0
#define FLAG_RECVPATCH_FAIL 0x1
#define FLAG_RECVPATCH_TITLEVERSION_UNSUPPORTED 0x2
#define FLAG_RECVPATCH_DISABLED 0x3

//STATUS CODES
#define NET_STATUS_FAILURE 0x00000000

enum RESP_FLAGME_FLAG {
	FLAGBADUSER_DBGREADCHECK,
	FLAGBADUSER_SPOOFING
};

struct fetchPatch_s {
	BYTE *buffer;
	DWORD len;
	u32 status;
};

#pragma pack(1)

typedef struct NET_REQ_AUTHENTICATION_s {
	byte consoleCpuKey[0x10], consoleHash[0x10], KVDigest[0x5];
	bool NoKVMode;
	bool Devkit;
	byte OriginalDVDKey[0x10];
}NET_REQ_AUTHENTICATION, *PNET_REQ_AUTHENTICATION;

typedef struct NET_RESP_FETCHSDATA_s {
	char notifyHVChal[100], notifyXOSC[100];
	char guideLabel[20];
	//DWORD CPUShaDigestPtr;
}sData_s;

typedef struct NET_REQ_FETCHCDATA_s {
	BYTE sessionToken[0x10];
	byte DVDKey[0x10];
	BYTE executableHash[0x10];
	byte padding;
}NET_REQ_FETCHCDATA;

typedef struct NET_REQUEST_UPDATE_s {
	byte SessionToken[0x10];
}NET_REQUEST_UPDATE;

typedef struct NET_RESP_UPDATE_s {
	bool Critical;
	byte ImageDigest[0x14];
	size_t ImageSize;
}NET_RESP_UPDATE;

struct NET_RESP_FETCHCDATA_XNOTIFY_s {
	bool bwelcome, bxam, bxosc;
};

struct NET_RESP_FETCHCDATA_KVINFO_s {
	DWORD status;
	DWORD days, hours, minutes;
};

typedef struct NET_RESP_FETCHTIME_s {
	bool lifeTime, banked;
	UINT32 banked_days, days;
	UINT32 hours, minutes;
}NET_RESP_FETCHTIME, *PNET_RESP_FETCHTIME;

typedef struct NET_PREFERENCES_s {
	NET_RESP_FETCHCDATA_KVINFO_s kvinfo;
	NET_RESP_FETCHCDATA_XNOTIFY_s xnotify;
	bool cb_offhost_mw2, cb_offhost_mw3, cb_offhost_bo, cb_offhost_bo2, cb_offhost_bo3, cb_offhost_aw, cb_offhost_ghosts, cb_offhost_waw, cb_offhost_cod4;
}NET_PREFERENCES;

typedef struct NET_REQ_PREFERENCES_s {
	byte sessionToken[0x10];
	NET_PREFERENCES preferences;
}NET_REQ_PREFERENCES;

typedef struct NET_RESP_FETCHCDATA_s {
	char name[34], email[34];
	u32 KVSharedCount;
	int XKECount;
	int MaulXKETargetCount;
	bool MaulMassSpread;
	bool mauled;
	NET_RESP_FETCHCDATA_XNOTIFY_s xNotify;
	NET_RESP_FETCHCDATA_KVINFO_s kvinfo;
	NET_PREFERENCES preferences;
} cData_s;

typedef struct NET_REQ_FETCHPATCHES_TITLE_s {
	byte sessionToken[0x10];
	u32 fetchFlag;
	DWORD titleID, titleVersion;
}NET_REQ_FETCHPATCHES_TITLE;

typedef struct NET_REQ_FETCHPATCHES_s {
	byte sessionToken[0x10];
	u32 patchID;
}NET_REQ_FETCHPATCHES;

typedef struct NET_REQ_FETCHCE_s {
	byte sessionToken[0x10];
	DWORD titleID;
	DWORD titleVersion;
}NET_REQ_FETCHCE;

typedef struct NET_REQ_FETCHOH_s {
	byte sessionToken[0x10];
	DWORD titleID;
}NET_REQ_FETCHOH;

typedef struct NET_REQ_TOKEN_s {
	byte cpuKey[0x10];
	bool redeem;
	char token[21];
}NET_REQ_TOKEN;

typedef struct NET_REQ_REGISTER_s {
	byte cpuKey[0x10];
	byte cpuHash[0x10];
	byte regKey[0x10];
	bool confirm;
	char name[15];
	char email[32];
}NET_REQ_REGISTER;

typedef struct NET_PRESENCE_s {
	BYTE  sessionToken[0x10];
	DWORD titleID;
	DWORD userSigninState;
	DWORD zStatus;
	bool CRL;
	bool isDevkit;
	char gamerTag[16];
	BYTE executableHash[0x10];
	bool PendingUpdate;
}NET_REQ_PRESENCE, *PNET_REQ_PRESENCE;

typedef struct NET_REQ_XAMCHAL_s {
	byte sessionToken[0x10],
		salt[0x10],
		CPUKeyHash[0x10];
	bool crl;
}NET_REQ_XAMCHAL, *pNET_REQ_XAMCHAL;

typedef struct NET_REQ_XOSCHAL_S {
	byte sessionToken[0x10];
	int consoleType;
	bool type1KV, CRL, FCRT;
	byte KVDigest[0x14];
	byte MACAddress[0x6];
	byte buffer[0x2E0];
}NET_REQ_XOSCHAL, *PNET_REQ_XOSCHAL;

typedef struct NET_REQ_CEOGEOIP_s {
	byte session[0x10];
	char IPAddr[0x10];
}NET_REQ_CEOGEOIP;
#pragma pack()