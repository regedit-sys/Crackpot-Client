#pragma once

namespace Devkit {
	DWORD XamFindOrCreateInternalPassportAccountHook(PBYTE bCountryId, FILETIME fileTime, PWCHAR pwchGamertag, PVOID PassportSessionToken);

	bool SetIsDevkit();
	void InstallHooks();
}