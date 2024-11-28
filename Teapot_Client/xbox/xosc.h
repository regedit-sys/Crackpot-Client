#pragma once
#include "stdafx.h"

class xosc {
public:
	void computeSecurityHash(PBYTE out);
	void execute(PBYTE resp);
private:
	void getSMCVersion(LPVOID version);
};