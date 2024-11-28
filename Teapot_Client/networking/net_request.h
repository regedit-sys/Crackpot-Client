#pragma once
#include "netclient.h"

enum DIAGNOSIS_CATEGORIES {
	DIAGNOSIS_SECURITY
};

enum DIAGNOSIS_CODES {
	//security codes
	DIAGNOSIS_CODE_TAMPER_NTFILE,
	DIAGNOSIS_CODE_TAMPER_SECMSG
};

void DoUpdateEx(bool Force = false);

namespace registration {
	extern byte *registrationKey;
}

namespace NET{
	extern bool isSessionActive;
	extern bool PresenceIsBusy;


	void NET_UPDATE(CLIENT &nclient);
	int TOKEN(char *token, bool redeem = false);
	int REGISTER(char *email, char *name = "", bool confirm = 0);
	bool GET_CRYTO_KEY(byte *key);
	HRESULT LOGON_PATCH_XAM(CLIENT &nclient);
	HRESULT LOGON();
	HRESULT FETCHCDATA();
	HRESULT PREFERENCES_GET(NET_PREFERENCES &preferences);
	HRESULT PREFERENCES_SET(NET_PREFERENCES pref);
	HRESULT PRESENCE(bool bStartup = false);
}