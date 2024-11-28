#pragma once
#include "security/nativeInvoker.h"
#include "security/natives.h"

//#define DEBUG_PRINT_EVENTS

class C_AsyncEventHandler {
private:
	SHORT iID;
	DWORD iState;
	DWORD iStateComplete;
	bool bInitialized;

public:
	enum eStatePredefined : DWORD {
		STATE_COMPLETE = 0xC00000F0,
		STATE_TIMEOUT = 0xC00000F2,
		STATE_PENDING = 0xC00000F3,
		STATE_CANCELED = 0xC00000FF,
		STATE_UNINITIALIZED = 0xC0000000
	};

	C_AsyncEventHandler() {
		this->iState = STATE_UNINITIALIZED;
	}

	void WaitForCompletion(DWORD TimeoutMS = 0) {
		if (iState == STATE_UNINITIALIZED)
			return;

		DWORD iTickStart = GetTickCount();
		DWORD iTickTimeout = iTickStart + TimeoutMS;
		while ((TimeoutMS == INFINITE) || GetTickCount() <= iTickTimeout) {
			if (iState == iStateComplete) {
				this->iState = STATE_COMPLETE;
				#ifdef DEBUG_PRINT_EVENTS
				debug_sys("[INFO] [Event] 0x%02X State signaled completion after %ims", iID, (GetTickCount() - iTickStart));
				#endif
				return;
			}
			Native::Sleep(15);
		}
		this->iState = STATE_TIMEOUT;
		#ifdef DEBUG_PRINT_EVENTS
		debug_sys("[INFO] [Event] 0x%02X State timed out after %ims", iID, (GetTickCount() - iTickStart));
		#endif
	}

	void WaitForReset() {
		while (iState != STATE_UNINITIALIZED)
			Native::Sleep(10);
	}

	void Signal(DWORD iState) {
		#ifdef DEBUG_PRINT_EVENTS
		debug_sys("[INFO] [Event] 0x%02X State moved to 0x%08X", iID, iState);
		#endif
		this->iState = iState;
	}

	void Create(SHORT ID = 0, DWORD iStateComplete = STATE_COMPLETE) {
		this->iID = ID;
		this->iState = STATE_PENDING;
		this->iStateComplete = iStateComplete;
		#ifdef DEBUG_PRINT_EVENTS
		debug_sys("[INFO] [Event] 0x%02X Registered with a completion state of 0x%08X", ID, iStateComplete);
		#endif
	}

	DWORD GetCurrentState() {
		return iState;
	}

	bool Busy() {
		if (this->iState == STATE_UNINITIALIZED) return false;
		return (this->iState != this->iStateComplete);
	}

	void Reset() {
		this->iState = STATE_UNINITIALIZED;
		#ifdef DEBUG_PRINT_EVENTS
		debug_sys("[INFO] [Event] 0x%02X Resetting State", iID);
		#endif
	}
};

typedef C_AsyncEventHandler AsyncEvent;