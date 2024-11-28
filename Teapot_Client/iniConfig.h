#pragma once
#include "util/SimpleIni.h"

namespace Config {
	bool ModifyINIBoolValue(const char *sub, const char *name, const bool value);
	void Apply();
}