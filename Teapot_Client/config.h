#pragma once

//#define LOCAL
//#define DEBUG_PRINT 
//#define DEBUG_DEV_PORT 94
//#define DEBUG_DEV_PORT 4880
//#define DEBUG_FIXPDBLOAD //NEVER FUCKING COMPILE PRODUCTION WITH THIS!
#define BUILD_XDK 

#define TITLE_DASH_SUPPORTED_CHECKSUM 0x009CE2C3

#define CFG_TWEAKXUI

#define NAME_SERVER					"Crackpot"
#define NAME_KV						"KV.bin"

#define DRIVE_TEAPOT				NAME_SERVER ":\\"
#define PATH_KV						DRIVE_TEAPOT NAME_KV
#define PATH_CPUKEY					DRIVE_TEAPOT "CPUKey.bin"
#define PATH_LOG					DRIVE_TEAPOT NAME_SERVER ".log"
#define PATH_LOG_DEBUG				PATH_LOG//DRIVE_TEAPOT NAME_SERVER "_Dbg.log"

#ifndef BUILD_XDK
#define XexGetModuleHandleAddr		 0x3DBAE2C2 /*0x8007D320*/
#define XamLoaderGetDvdTrayStateAddr 0xCDBE84C5 /*0x8169CEB0*/
#define XexGetProcedureAddressAddr   0xCDBBE2C2 /*0x8007D1B0*/
#else
#define XexGetModuleHandleAddr		 0x7D62E4C2 /*0x800A2AE0*/
#define XamLoaderGetDvdTrayStateAddr 0x95EE98C5 /*0x81759EB8*/
#define XexGetProcedureAddressAddr   0x2D71E4C2 /*0x800A1C10*/
#endif

#define XamExecuteChallengeAddr (DWORD)(Teapot::Devkit ? Native::DecVal(0x3DD19BC5) /*0x8172BC20*/ : Native::DecVal(0x9DF582C5) /*0x81679780*/)

#ifdef LOCAL 
const byte localIP[4] = { 69, 69, 69, 69 };
#endif

//Titles are strongly encouraged to use port 1000, or ports 1001-1255