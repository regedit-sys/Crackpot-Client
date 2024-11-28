#pragma once

#define S_FAIL -1
#define cstr c_str()

#include <xtl.h>
#include <xboxmath.h>
#include <stdio.h>
#include <fstream>
#include <xkelib.h>

#include <time.h>
#include <xui.h>

#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>
#include <xuiapp.h>
#include <map>
#include <string>
#include <array>

#include "util/util.h"
#include "util/Events.h"
#include "networking/net.h"
#include "networking/net_request.h"
#include "networking/netclient.h"

#include "security/nativeInvoker.h"
#include "security/natives.h"
#include "strings.h"

#include "appmanager/AppManager.h"
#include "dlaunch/dlaunch.h"
#include "Teapot.h"
#include "Memory.h"



#define DEVICE_NAME_HDD				"\\Device\\Harddisk0\\Partition1"
#define DEVICE_NAME_USB				"\\Device\\Mass0"
#define WCompare lstrcmpW
using namespace std;

typedef LONG ENGINESTATUS;

#define ERROR_ENGINE_TITLECHANGED 0xC0000017
#define ERROR_ENGINE_CONFLICT 0xC0000018
#define ERROR_ENGINE_DISABLED 0xC0000019
#define ERROR_ENGINE_SERVER   0xC0000020
#define ERROR_ENGINE_OUDATED  0xC0000021

#define ERROR_INVALID_OBJECT 0x10000023

#define E_ENGINE_OK 0xB000000
#define E_ENGINE_FAIL_PLDR 0xB0000020
#define E_ENGINE_FAIL_MEMORYNOTPRESENT 0xB0000021
#define E_ENGINE_FAIL_UNLOAD_HANDLE_NOTVALID 0xB00000FF
#define E_ENGINE_OK_UNLOAD_HANDLE 0xB000000

#define MAKE_HRESULT_BOOL(x) x?ERROR_SUCCESS:E_FAIL

typedef enum _XBOX_GAMES : DWORD {
	IHELPER = 0xF5D10000,
	DASHBOARD = 0xFFFE07D1,
	FREESTYLEDASH = 0xF5D20000,
	XEXMENU = 0xFFFF0055,
	XEXMENUALT = 0xC0DE9999,
	XSHELXDK = 0xFFFE07FF,
	DASHLAUNCH = 0xFFFF011D,
	COD_BLACK_OPS_2 = 0x415608C3,
	COD_BLACK_OPS_3 = 0x4156091D,
	COD_GHOSTS = 0x415608FC,
	COD_AW = 0x41560914,
	GTA_V = 0x545408A7,
	DESTINY = 0x415608F8,
	COD_MW2 = 0x41560817,
	COD_MW3 = 0x415608CB,
	COD_BO1 = 0x41560855,
	COD_BO3 = 0x4156091D,
	COD_WAW = 0x4156081C,
	YouTube = 0x423607D3
} XBOX_GAMES;