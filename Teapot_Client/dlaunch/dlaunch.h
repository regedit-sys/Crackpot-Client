#pragma once
#include "dlaunch_defs.h"

#define MODULE_LAUNCH "launch.xex"

class C_DLaunch {
public:
	//Dashlaunch exported function defintions
	DLaunch_Defs::DLAUNCHSTARTSYSMODULE StartSysModule;
	DLaunch_Defs::DLAUNCHSHUTDOWN Shutdown;
	DLaunch_Defs::DLAUNCHFORCEINILOAD ForceIniLoad;
	DLaunch_Defs::DLAUNCHGETNUMOPTS GetNumOpts;
	DLaunch_Defs::DLAUNCHGETOPTINFO GetOptInfo;
	DLaunch_Defs::DLAUNCHGETOPTVAL GetOptVal;
	DLaunch_Defs::DLAUNCHSETOPTVAL SetOptVal;
	DLaunch_Defs::DLAUNCHGETOPTVALBYNAME GetOptValByName;
	DLaunch_Defs::DLAUNCHSETOPTVALBYNAME SetOptValByName;
	DLaunch_Defs::DLAUNCHGETDRIVELIST GetDriveList;
	DLaunch_Defs::DLAUNCHGETDRIVEINFO GetDriveInfo;

	//Boot configured variables
	struct Vars_s {
		DWORD iPlugins;
		bool bDefaultPathSet;
		std::string sDefaultPath;
		std::string sConfigPath;
	};

	Vars_s Vars;
	bool Initalized;

private:
	void ResolveOrdinals() {
		using namespace DLaunch_Defs;
		std::string launchxex = getString(STR_LAUNCH);
		StartSysModule = (DLAUNCHSTARTSYSMODULE)Native::resolveFunction(launchxex.cstr, DL_ORDINALS_STARTSYSMOD);
		Shutdown = (DLAUNCHSHUTDOWN)Native::resolveFunction(launchxex.cstr, DL_ORDINALS_SHUTDOWN);
		ForceIniLoad = (DLAUNCHFORCEINILOAD)Native::resolveFunction(launchxex.cstr, DL_ORDINALS_FORCEINILOAD);
		GetNumOpts = (DLAUNCHGETNUMOPTS)Native::resolveFunction(launchxex.cstr, DL_ORDINALS_GETNUMOPTS);
		GetOptInfo = (DLAUNCHGETOPTINFO)Native::resolveFunction(launchxex.cstr, DL_ORDINALS_GETOPTINFO);
		GetOptVal = (DLAUNCHGETOPTVAL)Native::resolveFunction(launchxex.cstr, DL_ORDINALS_GETOPTVAL);
		SetOptVal = (DLAUNCHSETOPTVAL)Native::resolveFunction(launchxex.cstr, DL_ORDINALS_SETOPTVAL);
		GetOptValByName = (DLAUNCHGETOPTVALBYNAME)Native::resolveFunction(launchxex.cstr, DL_ORDINALS_GETOPTVALBYNAME);
		SetOptValByName = (DLAUNCHSETOPTVALBYNAME)Native::resolveFunction(launchxex.cstr, DL_ORDINALS_SETOPTVALBYNAME);
		GetDriveList = (DLAUNCHGETDRIVELIST)Native::resolveFunction(launchxex.cstr, DL_ORDINALS_GETDRIVELIST);
		GetDriveInfo = (DLAUNCHGETDRIVEINFO)Native::resolveFunction(launchxex.cstr, DL_ORDINALS_GETDRIVEINFO);
	}

	void SetupVariables() {
		//return; //wattwatttwatt-WATTTTT
		this->Vars.sDefaultPath = this->GetDefaultPath();
		if (this->Vars.sDefaultPath.length() > 0) {
			if (fileExists(std::string("HDD:").append(this->Vars.sDefaultPath).c_str())) {
				this->Vars.bDefaultPathSet = true;
			}else if (fileExists(std::string("USB:").append(this->Vars.sDefaultPath).c_str())){
				this->Vars.bDefaultPathSet = true;
			}
		}

		std::array<std::string, 2> Paths = { "USB:\\Launch.ini", "HDD:\\Launch.ini" };
		for (size_t i = 0; i < Paths.size(); i++) {
			if (fileExists(Paths[i].c_str())) {
				this->Vars.sConfigPath = Paths[i];
				break;
			}
		}

		for (int i = 1; i <= 5; i++) {
			char buffer[10];
			sprintf(buffer, "Plugin%i", i);
			if (strlen(GetConfValuByName(buffer)) > 0x3) this->Vars.iPlugins++;
		}

		debug("[LAUNCH] Config File: %s", this->Vars.sConfigPath.c_str());
	}

public:
	HRESULT Init() {
		this->ResolveOrdinals();
		this->SetupVariables();
		this->Initalized = true;
		return ERROR_SUCCESS;
	}

	char *GetConfValuByName(char *ElemName) {
		char *DLPathPtr;
		if (GetOptValByName(ElemName, (PDWORD)&DLPathPtr)) { //always allocated with fixed size, null if not specified
			return DLPathPtr;
		}
		return DLPathPtr;
	}

	std::string GetPluginsList() {
		std::string plugins;

		debug("Plugin Count: %i", this->Vars.iPlugins);
		for (DWORD i = 1; i <= this->Vars.iPlugins; i++) {
			char buffer[10];
			sprintf(buffer, "Plugin%i", i);
			std::string plugin = GetConfValuByName(buffer);
			plugins.append(plugin.erase(0, 1));
			if (i != this->Vars.iPlugins) plugins.append(",");
		}
		return plugins.cstr;
	}

	std::string GetDefaultPath() {
		std::string Path("");
		char *DLPathPtr;
		if (GetOptValByName("default", (PDWORD)&DLPathPtr)) { //always allocated with fixed size, null if not specified
			Path = std::string(DLPathPtr);
		}
		return Path;
	}

	NTSTATUS SetOptBoolValByName(char *optKey, bool state) {
		if (!this->Initalized) return S_OK;
		DWORD value = state ? 1 : 0;
		if (!SetOptValByName(optKey, &value)) return E_FAIL;
		return S_OK;
	}

	NTSTATUS SetLiveBlock(bool state) {
		if (ini::offline) {
			debug("[WARN] OFFLINE MODE IS ENABLED!");
		}

		return SetOptBoolValByName(&getString(STR_LIVEBLOCK)[0], ini::offline ? true : state);
	}

	NTSTATUS SetXHTTP(bool state) {
		return SetOptBoolValByName(&getString(STR_DLOPT_XHTTP)[0], state);
	}

	NTSTATUS ConfigSetBool(const char* Section, const char *Opt, const bool State) {
		if (!this->Initalized) return S_OK;

		CSimpleIniA DLConfig(true, false, true);
		if (DLConfig.LoadFile(this->Vars.sConfigPath.c_str()) >= 0) {
			DLConfig.SetBoolValue(Section, Opt, State, NULL, TRUE);
			DLConfig.SaveFile(this->Vars.sConfigPath.c_str());
			return S_OK;
		}
		return S_FAIL;
	}

	void SabotageXHTTP() {
		this->ConfigSetBool(getString(STR_INI_SETTINGS).cstr, getString(STR_DLOPT_XHTTP).cstr, true);
		this->SetXHTTP(false);
	}
};

