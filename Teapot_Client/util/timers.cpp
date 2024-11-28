#include "stdafx.h"
#include "timer.h"

std::vector<timer_s> timers_db;

timer::timer(void *callback, int waitTime, LPVOID param) {
	std::vector<timer_s>::iterator it = find_if(timers_db.begin(), timers_db.end(), timer_s(callback, waitTime));
	if (it != timers_db.end()) {
		int index = std::distance(timers_db.begin(), it);
		if (timers_db[index].intervals++ > timers_db[index].waitTime / 15) {
			if(param) (((void(*)(LPVOID))callback)(param));
			else ((void(*)(void))callback)();
			timers_db[index].intervals = 0;
		}
	}else {
		timer_s tmp(callback, waitTime);
		timers_db.push_back(tmp);
	}
}

void timers::CallBackOnInterval(void *callBack, int waitTime, LPVOID param) {
	timer(callBack, waitTime, param);
}