#pragma once
#define AppMgrIsTitleApplication(x) (x->ImageBase==Native::DecVal(0x1D8DEAB8) /*0x82000000*/)
#define AppMgrIsTitleRealDash(x) (x->GetAppNameW() == L"dash.xex")
#define AppMgrIsTitleXShell(x) (x->GetAppNameW() == L"xshell.xex")

namespace AppManager {
	enum E_AppType {
		TYPE_APP_DASHBOARD,
		TYPE_APP_TITLE,
		TYPE_APP_DLL,
		TYPE_APP_SYSDLL,
		TYPE_APP_TITLE_IMPORTS
	};

	class C_AppInstance {
	public:
		NTSTATUS iStatus;
		PLDR_DATA_TABLE_ENTRY pLDR; //contains ptrs -- no deep cpy
		PXEX_EXECUTION_ID pExID; //contains ptrs -- no deep cpy
		DWORD Loadflags, ImageBase, NtHeaderBase, TimeDateStamp, Checksum, Version, TitleID; //local copies, incase we lose the module in ram
		std::wstring CachedDllNameW;
		E_AppType Type;
		HANDLE hand;

		C_AppInstance(std::string ModuleName);
		C_AppInstance(HANDLE Handle);
		C_AppInstance() { this->ImageBase = 0x0; this->pLDR = nullptr; this->pExID = nullptr; }

	private:
		NTSTATUS InitInstance(HANDLE Handle);
		PXEX_EXECUTION_ID Construct_ExID();
		E_AppType Construct_AppType();
		HRESULT InstallHooks();

	public:
		HRESULT Init();
		std::wstring GetAppNameW(bool FullPath = false);
		std::string GetAppName(bool FullPath = false);
		void RegisterActiveTitle();

		bool IsDefaultApp();
		bool IsWhiteListedApp();
		bool Valid();
	};
};