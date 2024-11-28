#pragma once

namespace DLaunch_Defs {
	typedef enum {
		DL_ORDINALS_LDAT = 1,
		DL_ORDINALS_STARTSYSMOD,
		DL_ORDINALS_SHUTDOWN,
		DL_ORDINALS_FORCEINILOAD,
		DL_ORDINALS_GETNUMOPTS,
		DL_ORDINALS_GETOPTINFO,
		DL_ORDINALS_GETOPTVAL,
		DL_ORDINALS_SETOPTVAL,
		DL_ORDINALS_GETOPTVALBYNAME,
		DL_ORDINALS_SETOPTVALBYNAME,
		DL_ORDINALS_GETDRIVELIST,
		DL_ORDINALS_GETDRIVEINFO,
	} DL_ORDINALS;

	typedef enum {
		// DWORD containing a value between 0 and 1
		DL_OPT_TYPE_BOOL = 0,
		// WORD containing a value between 0 and 0xFFFF
		DL_OPT_TYPE_WORD,
		DL_OPT_TYPE_WORDREGION, // 0-0x7FFF
		DL_OPT_TYPE_WORDPORT, // 1 - 0xFFFF
		// DWORD containing a value between 0 and 0xFFFFFFFF
		DL_OPT_TYPE_DWORD,
		DL_OPT_TYPE_DWORDTIME, // in seconds
		// anything >= this can't be changed easily as it is a pkey pointer
		DL_OPT_TYPE_MAX_ACCESS,
		// DWORD containing a memory address/pkeydata element
		DL_OPT_TYPE_PATH = DL_OPT_TYPE_MAX_ACCESS, // generic path
		DL_OPT_TYPE_PATHQLB, // quick launch button
		DL_OPT_TYPE_PATHPLUGIN,
	} DL_OPT_TYPES;

	// opt categories
	enum {
		OPT_MIN = 0,
		OPT_CAT_PATHS, // various paths including quick launch buttons
		OPT_CAT_BEHAVIOR, // settings that change xbox behavior
		OPT_CAT_NETWORK, // settings that impact network
		OPT_CAT_TIMERS, // settings for things that run on timers
		OPT_CAT_PLUGINS, // plugin paths
		OPT_CAT_EXTERNAL, // options for the configurator
		OPT_CAT_MAX // not a real category, just a placeholder for array stuff
	};

	typedef struct _ldata {
		DWORD ID;
		DWORD ltype;
		char link[MAX_PATH];
		char dev[MAX_PATH];
		USHORT versionMaj;
		USHORT versionMin;
		USHORT targetKernel;
		USHORT svnVer;
		DWORD options; // for external apps that want to know what dash launch has set/parsed
		DWORD DebugRoutine; // for external apps that want to recursively hook and call the first/last chance exception trap on their own
		DWORD DebugStepPatch; // address to path single step exception to not be skipped (write 0x60000000/nop to this address to enable it)
		PBYTE tempData; // DL will monitor temps, a copy of the smc temp data is placed here, 0x10 bytes in len
		DWORD iniPathSel; // the path corresponding to this number can be gotten via dlaunchGetDriveList, 0xFF is none, 0xFE is forced
	} ldata, *pldata;

	typedef struct _keydata {
		char launchpath[MAX_PATH];
		DWORD flags;
		DWORD dev;
		DWORD rootDev;
	} keydata, *pkeydata;

	// totalOpts - OUT if provided a pointer to a int it will provide the number of total options
	// returns the total number of options
	typedef int(*DLAUNCHGETNUMOPTS)(int* totalOpts);
	// opt - IN, the number of the option you want info for
	// optType - OUT when given a pointer to a DWORD will copy type as enumerated above in DL_OPT_TYPES
	// outStr - OUT when given a pointer to a char array (will never need larger than 20 bytes) dash launch will copy the option ini name there
	// currVal - OUT when given a pointer to a DWORD it will copy the current value of that option
	// defValue - OUT when given a pointer to a DWORD it will copy the default/no ini found value of that option
	// optCategory - OUT when given a pointer to a DWORD it will copy the category that is set for installer use (see above enum, some categories will have no opts)
	typedef int(*DLAUNCHGETOPTINFO)(int opt, PDWORD optType, PCHAR outStr, PDWORD currVal, PDWORD defValue, PDWORD optCategory);
	// lets you get/set option values by their number, returns TRUE if successful and FALSE if not
	// use dlaunchGetNumOpts and dlaunchGetOptInfo to parse option numbers which will likely change from release to release
	typedef BOOL(*DLAUNCHGETOPTVAL)(int opt, PDWORD val);
	typedef BOOL(*DLAUNCHSETOPTVAL)(int opt, PDWORD val);
	// lets you get/set option values by their ini name, returns TRUE if successful and FALSE if not
	typedef BOOL(*DLAUNCHGETOPTVALBYNAME)(char* optName, PDWORD val);
	typedef BOOL(*DLAUNCHSETOPTVALBYNAME)(char* optName, PDWORD val);
	// causes dash launch first run sequence to start, where it scans for an ini file, parses it an applies it
	// path - OUT when given a pointer to a device path it will attempt to load that ini immediately
	// note that runtime ini loads do not process ini items that have no use after boot time, including patches and plugins
	// an acceptable path is a mount point like "\\Device\\Mass0\\", launch.ini will be sought at the root of that mount point
	typedef VOID(*DLAUNCHFORCEINILOAD)(PCHAR path);
	// can be used to start a system module from a system thread, returns the NTSTATUS of XexLoadModule
	typedef DWORD(*DLAUNCHSTARTSYSMODULE)(char* modPath);
	// causes dash launch to remove all its hooks in preparation for unloading
	typedef VOID(*DLAUNCHSHUTDOWN)(VOID);
	// allows one to fetch the drive list that dash launch uses internally, dev corresponds to keydata dev as well
	// dev - IN the number of the device in the list
	// devDest - OUT OPTIONAL destination where the path will be copied
	// mountName - OUT OPTIONAL the mount name that corresponds to ini file path settings for this device (multiple devices of the same type share the same name)
	// friendlyName - OUT OPTIONAL a unique mount name based on mountname when more than one type of a device is present
	// returns total number in list, or 0 on error
	typedef DWORD(*DLAUNCHGETDRIVELIST)(DWORD dev, PCHAR devDest, PCHAR mountName, PCHAR friendlyName);
	// gets info about the drive list in DashLaunch
	// maxIniDrives - OUT OPTIONAL total number of drives in the list from first item capable of being scanned for ini files by dashlaunch
	// maxDevLen - OUT OPTIONAL returns the longest strlen() of the device mount paths
	// returns total number of drives in the list
	typedef DWORD(*DLAUNCHGETDRIVEINFO)(PDWORD maxIniDrives, PDWORD maxDevLen);
}