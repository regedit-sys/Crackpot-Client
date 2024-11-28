#include "stdafx.h"
#include "common.h"

AsyncEvent eFNotifyQueue;
bool isNotifyQueueBusy = false;
BOOL pfShow = (BOOL)0xBEEFC0DE;
BOOL pfShowMovie;
BOOL pfPlaySound;
BOOL pfShowIPTV;

void FNotify::toggleNotify(bool on) {
	if ((int)pfShow == 0xBEEFC0DE) XNotifyUIGetOptions(&pfShow, &pfShowMovie, &pfPlaySound, &pfShowIPTV);
	if (!on) XNotifyUISetOptions(pfShow, pfShowMovie, pfPlaySound, pfShowIPTV);
	else XNotifyUISetOptions(true, true, true, true);
}

void FNotify::render(FNotifyParams_s *params) {	
	toggleNotify(true);
	ULONGLONG qwAreas = params->persistent ? XNOTIFYUI_PRIORITY_PERSISTENT : XNOTIFYUI_PRIORITY_HIGH;
	XNotifyQueueUI(params->icon, XUSER_INDEX_ANY, qwAreas, params->msg, NULL);
	toggleNotify(false);

	if(params->msg) delete[] params->msg;
	eFNotifyQueue.Reset();
	delete params;
}

void FNotify::execute(FNotifyParams_s *params) {
	while (!AppMgr.HasDashLoaded) Native::Sleep(15);
	eFNotifyQueue.WaitForCompletion(10000);
	eFNotifyQueue.Create();

	if (params->iWait != 0)
		Native::Sleep(params->iWait);

	bool reboot = params->reboot;
	UCHAR ProcessType = KeGetCurrentProcessType();
	if (ProcessType != PROC_USER) {
		if (!Native::CreateThreadQ(render, params)) {
			debug("[FNotify] Failed to create thread!");
			if (params->msg) delete[] params->msg;
			delete params;
			return;
		}
	}else render(params);

	if (reboot) {
		Native::Sleep(9500);
		Native::HalReturnToFirmware(HalRebootQuiesceRoutine);
	}
}

void FNotify::SetParams(bool reboot, bool thread, int iWait, bool persistent) {
	params->msg = this->msg;
	params->reboot = reboot;
	params->thread = thread;
	params->iWait = iWait;
	params->persistent = persistent;
}

void FNotify::show(bool reboot, bool thread, int iWait) {
	params->icon = XNOTIFYUI_TYPE_PREFERRED_REVIEW;
	SetParams(reboot, thread, iWait);
	debug("[FNotify] [Show]: \"%ws\"", this->msg);
	Native::CreateThreadExQ(this->execute, params);
}

void FNotify::error(bool reboot, bool thread, int iWait) {
	params->icon = XNOTIFYUI_TYPE_AVOID_REVIEW;
	SetParams(reboot, thread, iWait);
	debug("[FNotify] [Error]: \"%ws\"", this->msg);
	Native::CreateThreadExQ(this->execute, params);
}

void FNotify::persistent(bool reboot, bool thread, int iWait) {
	params->icon = XNOTIFYUI_TYPE_PREFERRED_REVIEW;
	SetParams(reboot, thread, iWait, true);
	debug("[FNotify] [Persistent]: \"%ws\"", this->msg);
	Native::CreateThreadExQ(this->execute, params);
}