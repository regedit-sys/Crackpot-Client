#include "stdafx.h"
#include "common.h"
#include "pluginManager.h"
#include "../../util/SimpleIni.h"
#include "../../util/io.h"
#include "../hud.h"

using namespace hud;

pluginManager::PluginList_s pluginManager::PluginList;
pluginManager::QuickLaunchPluginList_s pluginManager::qPluginList;
wchar_t PluginBuffer[85] = L"null";
char launchbuffer[50] = "null";
char DLaunchPluginBuffer[50] = "None";

void hud::pluginManager::ParseQuicklaunchINI() {
	if (!DirectoryExists("Crackpot:\\CrackpotCfg\\"))
		CreateDirectory("Crackpot:\\CrackpotCfg\\", 0);

	if (!fileExists("Crackpot:\\CrackpotCfg\\Quicklaunch.ini"))
		WrteQuickLaunchINI();


	CSimpleIni ReadINI(true, false, true);
	SI_Error res = ReadINI.LoadFile("Crackpot:\\CrackpotCfg\\Quicklaunch.ini");
	if (res >= SI_OK && !ReadINI.IsEmpty()) {
		mbstowcs(qPluginList.Plugin1, ReadINI.GetValue("Plugins", "plugin1"), 50);
		mbstowcs(qPluginList.Plugin2, ReadINI.GetValue("Plugins", "plugin2"), 50);
		mbstowcs(qPluginList.Plugin3, ReadINI.GetValue("Plugins", "plugin3"), 50);
	}else debug("Error - pluginManager::ParseQuicklaunchINI(%08X)", res);
}

void hud::pluginManager::WrteQuickLaunchINI() {
	SI_Error res;
	CSimpleIni WriteINI(true, false, true);
	WriteINI.SetValue("Plugins", "plugin1", "Change_Me!");
	WriteINI.SetValue("Plugins", "plugin2", "Change_Me!");
	WriteINI.SetValue("Plugins", "plugin3", "Change_Me!");
	res = WriteINI.SaveFile("Crackpot:\\CrackpotCfg\\Quicklaunch.ini");
	if(res != SI_OK) debug("Error - hud::pluginManager::WrteQuickLaunchINI(%08X)", res);
}

void hud::pluginManager::ParseLaunchINI() {
	CSimpleIni ReadINI(true, false, true);
	SI_Error res = ReadINI.LoadFile(&DLaunch.Vars.sConfigPath[0]);
	if (res >= SI_OK && !ReadINI.IsEmpty()) {
		//debug("STRLEN BS = %i || str = '%s'", strlen(ReadINI.GetValue("Plugins", "plugin3")), ReadINI.GetValue("Plugins", "plugin3"));
		mbstowcs(PluginList.Plugin1, ReadINI.GetValue("Plugins", "plugin1"), 50);
		mbstowcs(PluginList.Plugin2, ReadINI.GetValue("Plugins", "plugin2"), 50);
		mbstowcs(PluginList.Plugin3, ReadINI.GetValue("Plugins", "plugin3"), 50);
		mbstowcs(PluginList.Plugin4, ReadINI.GetValue("Plugins", "plugin4"), 50);
		mbstowcs(PluginList.Plugin5, ReadINI.GetValue("Plugins", "plugin5"), 50);
	}else debug("Error - hud::pluginManager::ParseLaunchINI(%08X)", res);

}

void hud::pluginManager::WriteLaunchINI(int plugin) {
	CSimpleIni WriteINI(true, false, true);
	SI_Error res;
	res = WriteINI.LoadFile(&DLaunch.Vars.sConfigPath[0]);
	if (res >= SI_OK) {
		switch (plugin) {
		case 1: WriteINI.SetValue("Plugins", "plugin1", DLaunchPluginBuffer); break;
		case 2: WriteINI.SetValue("Plugins", "plugin2", DLaunchPluginBuffer); break;
		case 3: WriteINI.SetValue("Plugins", "plugin3", DLaunchPluginBuffer); break;
		case 4: WriteINI.SetValue("Plugins", "plugin4", DLaunchPluginBuffer); break;
		case 5: WriteINI.SetValue("Plugins", "plugin5", DLaunchPluginBuffer); break;
		} 
		if (WriteINI.SaveFile("Hdd:\\launch.ini") != SI_OK) debug("hud::pluginManager::WriteLaunchINI(SaveFile)");
	}else debug("Error - hud::pluginManager::WriteLaunchINI(%08X)", res);
}

void hud::pluginManager::SetPlugin(int pluginid) {
	int error;
	const char* temp;
	switch (pluginid) {
	case 1: temp = XamShowKeyboard(L"Hdd:\\", L"Crackpot - Set Plugin 1", L"Enter the path to the plugin", 50, error); break;
	case 2: temp = XamShowKeyboard(L"Hdd:\\", L"Crackpot - Set Plugin 2", L"Enter the path to the plugin", 50, error); break;
	case 3: temp = XamShowKeyboard(L"Hdd:\\", L"Crackpot - Set Plugin 3", L"Enter the path to the plugin", 50, error); break;
	case 4: temp = XamShowKeyboard(L"Hdd:\\", L"Crackpot - Set Plugin 4", L"Enter the path to the plugin", 50, error); break;
	case 5: temp = XamShowKeyboard(L"Hdd:\\", L"Crackpot - Set Plugin 5", L"Enter the path to the plugin", 50, error); break;
	}
	if (error == 0) {
		sprintf(DLaunchPluginBuffer, "%s", temp);
		WriteLaunchINI(pluginid);
	}
	wchar_t tmp[50];
	Native::Sleep(1000);
	Native::XuiSceneCreate(LinkWChar(tmp, L"section://%X,Teapot#", AppMgr.Self.hand), L"TeapotScene.xur", NULL, &hud::SettingsScene);
	Native::XuiSceneNavigateForward(hud::hGuideMain, FALSE, hud::SettingsScene, 0xFF);
	XuiSceneSetBackScene(hud::SettingsScene, hud::hGuideMain);
}

void pluginManager::InjectNewPlugin(){
	int error;
	const char* temp = XamShowKeyboard(L"HDD:\\", L"Crackpot - Load Plugin", L"Enter the plugin path using (HDD:\\ or USB:\\)", 50, error);
	if (error == 0) {
		sprintf(launchbuffer, "%s", temp);
		if (fileExists(launchbuffer)) {
			Native::XexLoadImage(launchbuffer, XEX_LOADIMG_FLAG_DLL, NULL, nullptr);
			FNotify(L"Crackpot - Injected Plugin!").show();
		}else FNotify(L"Crackpot - Plugin not found!").error();
	}
	wchar_t tmp[50];
	Native::Sleep(1000);
	Native::XuiSceneCreate(LinkWChar(tmp, L"section://%X,Teapot#", AppMgr.Self.hand), L"TeapotScene.xur", NULL, &hud::SettingsScene);
	Native::XuiSceneNavigateForward(hud::hGuideMain, FALSE, hud::SettingsScene, 0xFF), XuiSceneSetBackScene(hud::SettingsScene, hud::hGuideMain);
}

void pluginManager::LaunchPlugin(int passed) {
	switch (passed) {
	case 1: sprintf(launchbuffer, "%ws", qPluginList.Plugin1); break;
	case 2: sprintf(launchbuffer, "%ws", qPluginList.Plugin2); break;
	case 3: sprintf(launchbuffer, "%ws", qPluginList.Plugin3); break;
	}

	if (fileExists(launchbuffer)) {
		Native::XexLoadImage(launchbuffer, XEX_LOADIMG_FLAG_DLL, NULL, nullptr);
		FNotify(L"Crackpot - Injected Plugin!").show();
	}else FNotify(L"Crackpot - Plugin not found!").error();
}

bool doOnce = false;

HRESULT pluginManager::scnTabPlugins::OnInit(XUIMessageInit* pInitData, BOOL& bHandled) {
	ParseLaunchINI();
	ParseQuicklaunchINI();
	this->GetChildById(L"btnPlugin1", &btnPlugin1);
	this->GetChildById(L"btnPlugin2", &btnPlugin2);
	this->GetChildById(L"btnPlugin3", &btnPlugin3);
	this->GetChildById(L"btnPlugin4", &btnPlugin4);
	this->GetChildById(L"btnPlugin5", &btnPlugin5);
	this->GetChildById(L"btnInjectPlugin", &btnInjectPlugin);
	this->GetChildById(L"btnQLaunch1", &btnQLaunch1);
	this->GetChildById(L"btnQLaunch2", &btnQLaunch2);
	this->GetChildById(L"btnQLaunch3", &btnQLaunch3);
	this->GetChildById(L"btnResetXBDM", &btnResetXBDM);
	fillPluginSlots();
	this->SetTimer(0, 5000);

	bHandled = TRUE;
	return ERROR_SUCCESS;
}

HRESULT pluginManager::scnTabPlugins::OnTimer(XUIMessageTimer* pTimer, BOOL& bHandled) {
	if (pTimer->nId == 0) {
		ParseLaunchINI();
		ParseQuicklaunchINI();
		fillPluginSlots();
	}
	bHandled = true;
	return ERROR_SUCCESS;
}

void pluginManager::scnTabPlugins::fillPluginSlots() {
	swprintf(PluginBuffer, L"Plugin 1: %ws", CheckWC(PluginList.Plugin1) ? L"None" : PluginList.Plugin1);
	btnPlugin1.SetText(PluginBuffer);

	swprintf(PluginBuffer, L"Plugin 2: %ws", CheckWC(PluginList.Plugin2) ? L"None" : PluginList.Plugin2);
	btnPlugin2.SetText(PluginBuffer);

	swprintf(PluginBuffer, L"Plugin 3: %ws", CheckWC(PluginList.Plugin3) ? L"None" : PluginList.Plugin3);
	btnPlugin3.SetText(PluginBuffer);

	swprintf(PluginBuffer, L"Plugin 4: %ws", CheckWC(PluginList.Plugin4) ? L"None" : PluginList.Plugin4);
	btnPlugin4.SetText(PluginBuffer);

	swprintf(PluginBuffer, L"Plugin 5: %ws", CheckWC(PluginList.Plugin5) ? L"None" : PluginList.Plugin5);
	btnPlugin5.SetText(PluginBuffer);

	swprintf(PluginBuffer, L"Quicklaunch 1: %ws", CheckWC(qPluginList.Plugin1) ? L"None" : qPluginList.Plugin1);
	btnQLaunch1.SetText(PluginBuffer);

	swprintf(PluginBuffer, L"Quicklaunch 2: %ws", CheckWC(qPluginList.Plugin2) ? L"None" : qPluginList.Plugin2);
	btnQLaunch2.SetText(PluginBuffer);

	swprintf(PluginBuffer, L"Quicklaunch 3: %ws", CheckWC(qPluginList.Plugin3) ? L"None" : qPluginList.Plugin3);
	btnQLaunch3.SetText(PluginBuffer);
}

HRESULT pluginManager::scnTabPlugins::OnNotifyPress(HXUIOBJ hObjPressed, BOOL& bHandled) {

	if (hObjPressed == btnPlugin1) Native::CreateThreadExQ(SetPlugin, (void*)1);
	if (hObjPressed == btnPlugin2) Native::CreateThreadExQ(SetPlugin, (void*)2);
	if (hObjPressed == btnPlugin3) Native::CreateThreadExQ(SetPlugin, (void*)3);
	if (hObjPressed == btnPlugin4) Native::CreateThreadExQ(SetPlugin, (void*)4);
	if (hObjPressed == btnPlugin5) Native::CreateThreadExQ(SetPlugin, (void*)5);

	if (hObjPressed == btnQLaunch1) LaunchPlugin(1);
	if (hObjPressed == btnQLaunch2) LaunchPlugin(2);
	if (hObjPressed == btnQLaunch3) LaunchPlugin(3);
	if (hObjPressed == btnInjectPlugin) Native::CreateThreadExQ(InjectNewPlugin);

	if (hObjPressed == btnResetXBDM) FNotify(L"Crackpot - Reset XBDM!").show();

	bHandled = TRUE;
	return ERROR_SUCCESS;
}