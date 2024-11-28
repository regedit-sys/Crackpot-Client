#include "stdafx.h"
#include "Services.h"
#include "hud/dashboard/dash.h"
#include "xbox/XamExecuteChallenge.h"
#include "xbox/challenges.h"
#include "security/XMitSecMsg.h"
#include "util/timer.h"
#include "xbox/XBL.h"

DWORD iDelayedRGB2 = 0;
DWORD iDelayedKVStatusCheck = 0;
bool KVStatusCheckHasStarted = false;
bool Services::HasRan = false;

/*namespace XBL {
	static DWORD ZStatusOffset = 0xC13D0004;

	enum ZSTATUSCODES : DWORD {
		ZSTATUS_ISERROR = 0x8015,
		ZSTATUS_BANNED = 0x8015190D,
		ZSTATUS_UNKNOWNERROR = 0x80151907,
		ZSTATUS_NODNS = 0x80151904,
		ZSTATUS_UPDATEREQUIRED = 0x80151007
	};

	void ZStatusMonitor();
};*/

void Services::CreatePresence() {
	AsyncPresenceEvent.Create();
	LPVOID PresenceExecAddress = Native::Ptr_Presence();
	Native::CreateThreadExQ(PresenceExecAddress);
}

void DoChallengeAntiTamper() {
	//XeCryptAes::VerifyIntegrity();

	if (Mem::Compare((PDWORD)XamExecuteChallengeAddr, Xbox::Challenges::Xam::IntegrityDigest, Native::DecVal(0x318DEB42) /*0x0C*/) != 0) {
		Native::report(AntiTamper::INTEG_CHALLENGE_XEC_HOOK, true);
	}

	if (KV::CRL) {
		PVOID XOSCHookAddr = (PVOID)Xbox::Challenges::XOS::GetCachedEncryptedXOSAddress();
		if (Mem::Compare(XOSCHookAddr, Xbox::Challenges::XOS::IntegrityDigest, Native::DecVal(0x318DEB42) /*0x0C*/) != 0) {
			Native::report(AntiTamper::INTEG_CHALLENGE_XOS_HOOK, true);
		}
	}
}

void Services::WorkerThread() {
	DWORD XNetLogonGetExtendedStatusPtr = (DWORD)Native::resolveFunction(getString(STR_XAM).cstr, Native::DecVal(0xE68BEB42) /*0x13B*/);
	Native::setupSegmentHashing();

	for (;;) {
		timers::CallBackOnInterval(Native::ChallengeSecurityTamperCheck, Native::DecVal(0x618AEB42) /*700ms*/);
		timers::CallBackOnInterval(Native::HashTextSection, Native::DecVal(0x518CEB42) /*0x12C*/);

		if (Teapot::Initialized && AppMgr.HasDashLoaded && !AsyncPresenceEvent.Busy()) {
			timers::CallBackOnInterval(CreatePresence, Native::DecVal(0xED58EC42)); /*210000 (3.5m)*/
		}

		if (Teapot::Initialized) {
			timers::CallBackOnInterval(DoChallengeAntiTamper, Native::DecVal(0x618AEB42) /*700ms*/);
		}

		DWORD iDelayedKVStatusCheckTime = Native::DecVal(0xA579EB42) /*0x1388*/ / 15;
		if (!KVStatusCheckHasStarted && iDelayedKVStatusCheck > iDelayedKVStatusCheckTime) {
			KVStatusCheckHasStarted = true;
			debug("KVStatus Reporter has started!");
		}else if (iDelayedKVStatusCheck <= iDelayedKVStatusCheckTime) iDelayedKVStatusCheck++;

		if (Teapot::Initialized && AppMgr.HasDashLoaded && KVStatusCheckHasStarted) {
			timers::CallBackOnInterval(XBL::ZStatusMonitor, Native::DecVal(0xD17FEB42) /*0xDAC*/);
		}

		dashboard::DoRGB();
		if (iDelayedRGB2 > 700 / 15) {
			dashboard::DoRGB2();
		}else iDelayedRGB2++;

		if (!HasRan) 
			HasRan = true;
	}
}

HRESULT Services::Init() {
	Native::CreateThreadExQ(Native::Ptr_ServicesWorkerThread());
	return 0;
}