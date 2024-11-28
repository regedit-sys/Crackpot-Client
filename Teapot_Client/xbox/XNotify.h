#pragma once

struct FNotifyParams_s {
	wchar_t *msg;
	bool reboot;
	bool thread;
	bool persistent;
	int iWait;
	XNOTIFYQUEUEUI_TYPE icon;
};

class FNotify { //check allocation for safety
public:
	wchar_t *msg;
	FNotifyParams_s *params;
	FNotify(const wchar_t *msg) {
		const size_t iMsgSize = lstrlenW(msg) + 1;
		this->params = new FNotifyParams_s();
		this->msg = new wchar_t[iMsgSize];
		wcsncpy(this->msg, msg, iMsgSize);
	}

	FNotify(const char *msg) {
		const size_t iMsgSize = strlen(msg) + 1;
		this->params = new FNotifyParams_s();
		this->msg = new wchar_t[iMsgSize];
		mbstowcs(this->msg, msg, iMsgSize);
	}

	static void toggleNotify(bool on);
	static void render(FNotifyParams_s *params);
	static void execute(FNotifyParams_s *params);
	void SetParams(bool reboot, bool thread, int iWait, bool persistent = false);
	void show(bool reboot = false, bool thread = true, int iWait = 0);
	void error(bool reboot = false, bool thread = true, int iWait = 0);
	void persistent(bool reboot = false, bool thread = true, int iWait = 0);
};