#pragma once
#include "stdafx.h"

struct timer_s {
	void *callBack;
	int waitTime;
	int intervals;
	timer_s(void* callBack, int waitTime) : callBack(callBack) { this->callBack = callBack; this->waitTime = waitTime; }
	bool operator () (const timer_s& m) const { return m.callBack == this->callBack; }
};

class timer {
public:
	timer(void *callback, int waitTime, LPVOID param = NULL);
};

namespace timers {
	void CallBackOnInterval(void *callBack, int waitTime, LPVOID param = NULL);
}