#include "stdafx.h"
#include "common.h"

#include "kv.h"
#include "../util/Detour.h"
#include "../util/XeCrypt.h"
#include "bannedKV.h"

byte KVCPU[0x10];
DWORD KV::updateSequence = 0x0000000B;
bool KV::type1KV = false;
bool KV::FCRT = false;
bool KV::CRL = false;
byte KV::CPUKeyDigest[0x14];
byte KV::KVDigest[0x14];
QWORD KV::HVAddr = 0;

KV::NoKV_s NoKV;

DWORD ComputeUpdateSequence() {
	BYTE macAddress[6];
	WORD SettingSize = 6;
	DWORD Sequence = 0x0000000B;
	DWORD SequenceHash = 0;

	Native::ExGetXConfigSetting(XCONFIG_SECURED_CATEGORY, XCONFIG_SECURED_MAC_ADDRESS, macAddress, 6, &SettingSize);
	Native::XeCryptSha(macAddress, 6, NULL, NULL, NULL, NULL, (BYTE*)&SequenceHash, 4);
	Sequence |= (SequenceHash & ~Native::DecVal(0x228CEB42) /*0xFF*/);
	return Sequence;
}

NTSTATUS HookKerbAddConsoleCertHashPrePreAuth(DWORD r3, BYTE* cert, DWORD r5) {
	DWORD linkReg;
	__asm mflr linkReg
	PLDR_DATA_TABLE_ENTRY pldrXam = (PLDR_DATA_TABLE_ENTRY)AppMgr.Xam.hand;
	PLDR_DATA_TABLE_ENTRY pldrKrnl = (PLDR_DATA_TABLE_ENTRY)AppMgr.Kernel.hand;
	bool sus = !(linkReg >= (DWORD)pldrXam->ImageBase && linkReg <= (DWORD)pldrXam->ImageBase + pldrXam->SizeOfFullImage);
	if (!sus) sus = !(linkReg >= (DWORD)pldrKrnl->ImageBase && linkReg <= (DWORD)pldrKrnl->ImageBase + pldrKrnl->SizeOfFullImage);
	if (sus) {
		if (cert) {
			Mem::Copy(cert, NoKV.data.ConsoleCertificate, Native::DecVal(0xC58BEB42) /*0x1A8*/);
			Native::RC4Custom(Teapot::Session, Native::DecVal(0x2D8DEB42) /*0x10*/, cert, Native::DecVal(0xC58BEB42) /*0x1A8*/);
		}
	}
	else Native::report(AntiTamper::TAMPER_KV_CERT_DECRYPT);
	return NoKV.DetourKerbAddConsoleCertHashPrePreAuth.callOriginal(r3, cert, r5);
}

NTSTATUS HookXeKeysGetConsoleCertificate(BYTE* cert) {
	DWORD linkReg;
	__asm mflr linkReg
	PLDR_DATA_TABLE_ENTRY pldrXam = (PLDR_DATA_TABLE_ENTRY)AppMgr.Xam.hand;
	PLDR_DATA_TABLE_ENTRY pldrKrnl = (PLDR_DATA_TABLE_ENTRY)AppMgr.Kernel.hand;
	bool sus = !(linkReg >= (DWORD)pldrXam->ImageBase && linkReg <= (DWORD)pldrXam->ImageBase + pldrXam->SizeOfFullImage);
	if (!sus) sus = !(linkReg >= (DWORD)pldrKrnl->ImageBase && linkReg <= (DWORD)pldrKrnl->ImageBase + pldrKrnl->SizeOfFullImage);
	if (sus) {//????????????????
		if (cert) {
			Mem::Copy(cert, NoKV.data.ConsoleCertificate, Native::DecVal(0xC58BEB42) /*0x1A8*/);
			Native::RC4Custom(Teapot::Session, Native::DecVal(0x2D8DEB42) /*0x10*/, cert, Native::DecVal(0xC58BEB42) /*0x1A8*/);
			return ERROR_SUCCESS;
		}
	}else Native::report(AntiTamper::TAMPER_KV_CERT_DECRYPT);
	return NoKV.DetourXeKeysGetConsoleCertificate.callOriginal(cert);
}

void HookFormatXenonConsoleCertificatePrincipalName(DWORD r3, char* r4, DWORD r5) {
	char* str = r4;
	str[0] = (byte)Native::DecVal(0xB58CEB42) /*0x58*/; // X
	str[1] = (byte)Native::DecVal(0xF08CEB42) /*0x45*/; // E
	str[2] = (byte)Native::DecVal(0x538DEB42) /*0x2E*/; // .
	str += 3;

	BYTE decryptedCert[0x1A8];
	Mem::Copy(decryptedCert, NoKV.data.ConsoleCertificate, Native::DecVal(0xC58BEB42) /*0x1A8*/);
	Native::RC4Custom(Teapot::Session, Native::DecVal(0x2D8DEB42) /*0x10*/, decryptedCert, Native::DecVal(0xC58BEB42) /*0x1A8*/);

	unsigned long long r11 = 0;
	for (int i = 0; i < 5; i++) {
		BYTE r9 = *(BYTE*)(decryptedCert + Native::DecVal(0x2F8DEB42) /*0x02*/ + i);
		r11 = r11 << 8;
		r11 = r9 + r11;
	}

	char buffer[10] = { ~'%', ~'0', ~'1', ~'1', ~'I', ~'6', ~'4', ~'u', ~'%', ~'u' };
	for (int i = 0; i < sizeof(buffer); i++) buffer[i] = ~buffer[i];
	_snprintf(str, r5, buffer, r11 >> 4, r11 & Native::DecVal(0x228DEB42) /*0xFFFFFFFF*/ & 0xF);

#ifdef BUILD_XDK
	if (Teapot::Devkit) {
		DWORD firstArg = *(DWORD*)(r3 + Native::DecVal(0xE55FEB42) /*0x2D48*/);
		DWORD ret = 0;

		__asm rlwinm ret, firstArg, 0, 13, 13

		if (ret == 0) {
			str[Native::DecVal(0x318DEB42) /*0x0C*/] = 0x0;
		}
		else {
			str[r5 - 1] = -1;
		}
	}
	else {

		str[Native::DecVal(0x328DEB42) /*0x0F*/] = 0x0;
	}
#else
	str[Native::DecVal(0x328DEB42) /*0x0F*/] = 0x0;
#endif

	return;
}

NTSTATUS HookGetSerialNumber(DWORD r3, BYTE* outSerial) {
	Mem::Copy(outSerial, NoKV.data.ConsoleSerialNumber, Native::DecVal(0x318DEB42) /*0x0C*/);
	return ERROR_SUCCESS;
}

HRESULT KV::NoKV_s::setMacAddress() {
	BYTE macAddress[6];
	WORD settingSize = 6;
	DWORD temp = 0;

	macAddress[0] = (byte)Native::DecVal(0x1D8DEB42) /*0x00*/;
	macAddress[1] = (byte)Native::DecVal(0x4F8DEB42) /*0x22*/;
	macAddress[2] = (byte)Native::DecVal(0xE58CEB42) /*0x48*/;
	macAddress[3] = data.consoleID.asBits.MacIndex3;
	macAddress[4] = data.consoleID.asBits.MacIndex4;
	macAddress[5] = data.consoleID.asBits.MacIndex5;

	Native::ExGetXConfigSetting(XCONFIG_SECURED_CATEGORY, XCONFIG_SECURED_MAC_ADDRESS, MACAddress, 6, &settingSize);
	if (memcmp(MACAddress, macAddress, 6) == 0) {
		Native::XeCryptSha(macAddress, 6, NULL, NULL, NULL, NULL, (BYTE*)&temp, 4);
		KV::updateSequence |= (temp & ~Native::DecVal(0x228CEB42) /*0xFF*/);
		debug("[NoKV] Set MAC: KV MAC Already set!");
	}else if (NT_SUCCESS(Native::ExSetXConfigSetting(XCONFIG_SECURED_CATEGORY, XCONFIG_SECURED_MAC_ADDRESS, macAddress, 6))) {
		clearCache(true);
	}else return E_FAIL;
	return ERROR_SUCCESS;
}

HRESULT KV::NoKV_s::ini(NET::CLIENT &nclient) {
	if (nclient.receive(&this->data, sizeof(NoKV_data_s)) != ERROR_SUCCESS) {
		debug("[NoKV] Init: Error querying KV data from server.");
		return E_FAIL;
	}

	if (FAILED(Hypervisor::PokeBytes(HVAddr + Native::DecVal(0x6D8CEB42) /*0xD0*/, this->data.ConsoleObfuscationKey, Native::DecVal(0x2D8DEB42) /*0x10*/))) return E_FAIL;
	if (FAILED(Hypervisor::PokeBytes(HVAddr + Native::DecVal(0x758AEB42) /*0x298*/, this->data.ConsolePrivateKey, Native::DecVal(0x6D8BEB42) /*0x1D0*/))) return E_FAIL;
	if (FAILED(Hypervisor::PokeBytes(HVAddr + Native::DecVal(0x589EB42) /*0x468*/, this->data.XeIkaPrivateKey, Native::DecVal(0xAD89EB42) /*0x390*/))) return E_FAIL;
	Mem::Copy((PVOID)Native::DecVal(0x3D0DEBB4) /*0x8E038020*/, &this->data.consoleID.abData, 5); 
	KV::updateSequence = ComputeUpdateSequence(); 

	/*debug("Dumping KV struct offsets\n..");
	debug("ConsoleCertificate 0x%X", offsetof(struct _KEY_VAULT, ConsoleCertificate));
	debug("ConsoleObfuscationKey 0x%X", offsetof(struct _KEY_VAULT, ConsoleObfuscationKey));
	debug("ConsolePrivateKey 0x%X", offsetof(struct _KEY_VAULT, ConsolePrivateKey));
	debug("XeikaPrivateKey 0x%X", offsetof(struct _KEY_VAULT, XeikaPrivateKey));
	debug("\\End of dump\n");*/

	Mem::Copy(CPUKeyDigest, this->data.CPUKeyDigest, Native::DecVal(0xF98CEB42) /*0x14*/);
	Mem::Copy(KVDigest, this->data.KVDigest, Native::DecVal(0xF98CEB42) /*0x14*/);

	KV::FCRT = (((WORD)this->data.XamOdd & ODD_POLICY_FLAG_CHECK_FIRMWARE) != 0); //changed 2021
	KV::type1KV = this->data.type1KV;

	this->DetourKerbAddConsoleCertHashPrePreAuth.setupDetour(Teapot::Devkit ? Native::DecVal(0x55A257C5) /*0x81B6EAF8*/ : Native::DecVal(0xAD2376C5) /*0x81946990*/, HookKerbAddConsoleCertHashPrePreAuth);
	this->DetourXeKeysGetConsoleCertificate.setupDetour(getString(STR_KERNEL).cstr, 31, HookXeKeysGetConsoleCertificate);
	this->DetourFormatXenonConsoleCertificatePrincipalName.setupDetour(Teapot::Devkit ? Native::DecVal(0x1DDF6CC5) /*0x8181AE00*/ : Native::DecVal(0x7D569BC5) /*0x817336E0*/, HookFormatXenonConsoleCertificatePrincipalName);
	this->DetourGetSerialNumber.setupDetour(Teapot::Devkit ? Native::DecVal(0xF5D86CC5) /*0x8181B418*/ : Native::DecVal(0x5439BC5) /*0x81734A68*/, HookGetSerialNumber);

	//setMacAddress();
	Native::XamCacheReset(XAM_CACHE_TICKETS);
	debug("[NoKV] Initalized!");
	return ERROR_SUCCESS;
}

BOOL XeKeysPkcs1Verify(const BYTE* pbHash, const BYTE* pbSig, XECRYPT_RSA* pRsa) {
	BYTE scratch[256];
	DWORD val = pRsa->cqw << 3;
	if (val <= Native::DecVal(0x1D8BEB42) /*0x200*/) {
		XeCryptBnQw_SwapDwQwLeBe((QWORD*)pbSig, (QWORD*)scratch, val >> 3);
		if (XeCryptBnQwNeRsaPubCrypt((QWORD*)scratch, (QWORD*)scratch, pRsa) == 0) return FALSE;
		XeCryptBnQw_SwapDwQwLeBe((QWORD*)scratch, (QWORD*)scratch, val >> 3);
		return XeCryptBnDwLePkcs1Verify((const PBYTE)pbHash, scratch, val);
	}
	else return FALSE;
}

HRESULT KV::setKeyVaultBytes(BYTE* KeyVault) {
	Mem::Copy(&kv, KeyVault, Native::DecVal(0x1D4DEB42) /*0x4000*/);

	Mem::Copy((PVOID)Native::DecVal(0x1DEDE6B4) /*0x8E03A000*/, &kv.ConsoleCertificate, Native::DecVal(0xC58BEB42) /*0x1A8*/);
	Mem::Copy((PVOID)Native::DecVal(0x3D0DEBB4) /*0x8E038020*/, &kv.ConsoleCertificate.ConsoleId.abData, 5);

	BYTE newHash[XECRYPT_SHA_DIGEST_SIZE];
	Native::XeCryptSha((BYTE*)Native::DecVal(0xF90CEBB4) /*0x8E038014*/, Native::DecVal(0x9189EB42) /*0x3EC*/, NULL, NULL, NULL, NULL, newHash, XECRYPT_SHA_DIGEST_SIZE);
	Mem::Copy((PVOID)Native::DecVal(0x1D0DEBB4) /*0x8E038000*/, newHash, XECRYPT_SHA_DIGEST_SIZE);

	Hypervisor::PeekBytes(HVAddr + Native::DecVal(0x6D8CEB42) /*0xD0*/, &kv.ConsoleObfuscationKey, Native::DecVal(0xDD8CEB42) /*0x40*/); //0x40 on purpose
	Mem::Copy(kv.RoamableObfuscationKey, Teapot::Devkit ? DeveloperKey : retailKey, Native::DecVal(0x2D8DEB42) /*0x10*/);

	Hypervisor::PokeBytes(HVAddr, &kv, Native::DecVal(0x1D4DEB42) /*0x4000*/);

	XECRYPT_HMACSHA_STATE hmacSha;
	XeCryptHmacShaInit(&hmacSha, KVCPU, Native::DecVal(0x2D8DEB42) /*0x10*/);
	XeCryptHmacShaUpdate(&hmacSha, (BYTE*)&kv.OddFeatures, Native::DecVal(0x398CEB42) /*0xD4*/);
	XeCryptHmacShaUpdate(&hmacSha, (BYTE*)&kv.DvdKey, Native::DecVal(0x5570EB42) /*0x1CF8*/);
	XeCryptHmacShaUpdate(&hmacSha, (BYTE*)&kv.CardeaCertificate, Native::DecVal(0x256CEB42) /*0x2108*/);
	XeCryptHmacShaFinal(&hmacSha, KVDigest, XECRYPT_SHA_DIGEST_SIZE);
	Mem::Copy((PBYTE)Native::DecVal(0xDDE2E6B4) /*0x8E03AA40*/, KVDigest, Native::DecVal(0xF98CEB42) /*0x14*/);

	type1KV = true;
	for (DWORD x = 0; x < Native::DecVal(0x1D8CEB42) /*0x100*/; x++) {
		if (kv.KeyVaultSignature[x] != NULL) type1KV = FALSE;
	}

	/*if (type1KV) {
		if (!XeKeysPkcs1Verify(KVDigest, kv.KeyVaultSignature, (XECRYPT_RSA*)masterKey)) {
			debug("[KV] Warning: invalid CPU key!");
		}else debug("[KV] Notice: - valid CPUKey!");
	}*/

	FCRT = (kv.OddFeatures & ODD_POLICY_FLAG_CHECK_FIRMWARE) != 0 ? TRUE : FALSE;
	return ERROR_SUCCESS;
}


bool readFromCPUKeyTxt(byte *CPUKey) {
	ifstream infile(DRIVE_TEAPOT "CPUKey.txt");

	if (infile.good()) {
		string sLine;
		getline(infile, sLine);
		if (sLine.length() != 32 || !isValidHexStr(sLine)) goto breakout;
		std::vector<char> cpuKey = HexToBytes(sLine);
		std::copy(cpuKey.begin(), cpuKey.end(), CPUKey);
		infile.close();
		return true;
	}
breakout:
	infile.close();
	return false;
}

bool readFromCPUKeyBin(byte *CPUKey) {
	MemoryBuffer mbCpu;
	if (cReadFile(DRIVE_TEAPOT "CPUKey.bin", mbCpu)) {
		if (mbCpu.CheckSize(0x10)) {
			Mem::Copy(CPUKey, mbCpu.GetData(), Native::DecVal(0x2D8DEB42) /*0x10*/);
			return true;
		}else debug("[KV] CPUKey.bin invalid length.");
	}
	return false;
}

void KV::processCPUKey(CHAR* FilePath) {
	const DWORD x10 = Native::DecVal(0x2D8DEB42) /*0x10*/;
	if (!readFromCPUKeyBin(KVCPU)) {
		if (!readFromCPUKeyTxt(KVCPU)) {
			debug("[KV] CPU Key Not Found! Generating random.");
			randomBytes(KVCPU, x10);
			cWriteFile(DRIVE_TEAPOT "CPUKey.bin", KVCPU, x10);
		}else debug("[KV] CPU Key read from %s", DRIVE_TEAPOT "CPUKey.txt");
	}else debug("[KV] CPU Key read from %s", DRIVE_TEAPOT "CPUKey.bin");
	Native::XeCryptSha(KVCPU, x10, NULL, NULL, NULL, NULL, CPUKeyDigest, XECRYPT_SHA_DIGEST_SIZE);
}

HRESULT KV::setKeyVault(CHAR* FilePath) {
	MemoryBuffer mbKv;
	if (!cReadFile(FilePath, mbKv)) {
		debug("[KV] - Failed to read KV.bin from device.");
		return E_FAIL;
	}

	return setKeyVaultBytes(mbKv.GetData());
}

HRESULT KV::InitKV() {
	DLaunch.SetLiveBlock(true);
	processCPUKey(PATH_CPUKEY);

	if (fileExists(PATH_KV)) {
		debug("[KV] Setting Keyvault from HDD.");
		if (FAILED(setKeyVault(PATH_KV))) {
			FNotify(L"Crackpot Error - 0xD9C0").error();
			debug("[KV] Set Keyvault from HDD Failed.");
			return E_FAIL;
		}
	}else {
		debug("[KV] Setting Keyvault from Flash.");
		BYTE* kvbuff = (BYTE*)Native::Malloc(Native::DecVal(0x1D4DEB42) /*0x4000*/);
		Hypervisor::PeekBytes(HVAddr, kvbuff, Native::DecVal(0x1D4DEB42) /*0x4000*/);
		setKeyVaultBytes(kvbuff); 
		Native::Free(kvbuff);
	}
	return ERROR_SUCCESS;
}

HRESULT KV::InitMACAddr() {
	BYTE macAddress[6];
	WORD settingSize = 6;
	macAddress[0] = 0x00;
	macAddress[1] = 0x22;
	macAddress[2] = 0x48;
	macAddress[3] = kv.ConsoleCertificate.ConsoleId.asBits.MacIndex3;
	macAddress[4] = kv.ConsoleCertificate.ConsoleId.asBits.MacIndex4;
	macAddress[5] = kv.ConsoleCertificate.ConsoleId.asBits.MacIndex5;



	Native::ExGetXConfigSetting(XCONFIG_SECURED_CATEGORY, XCONFIG_SECURED_MAC_ADDRESS, MACAddress, 6, &settingSize);
	if (memcmp(MACAddress, macAddress, 6) == 0) {
		DWORD temp = 0;
		Native::XeCryptSha(macAddress, 6, NULL, NULL, NULL, NULL, (BYTE*)&temp, 4);
		KV::updateSequence |= (temp & ~Native::DecVal(0x228CEB42) /*0xFF*/);
	}else if (NT_SUCCESS(Native::ExSetXConfigSetting(XCONFIG_SECURED_CATEGORY, XCONFIG_SECURED_MAC_ADDRESS, macAddress, 6))) {
		FNotify(L"Crackpot - Setting KV & MAC address!\nConsole Restarting.").show(true);
		clearCache(true);
		return E_FAIL;
	}
	return ERROR_SUCCESS;
}

HRESULT KV::Init() {
	debug("[KV] Keyvault Initialized!");
	QWORD KVAdd = Teapot::Devkit ? DecQWORD(0x8690FEFFFE000B09) /*0x00000002000162E0*/ : DecQWORD(0xA693FEFFFE000B09) /*0x00000002000163C0*/;
	KV::HVAddr = Hypervisor::PeekQWORD(KVAdd);

	Hypervisor::PeekBytes(KV::HVAddr + Native::DecVal(0x1D8CEB42) /*0x100*/, Teapot::CachedOriginalDVDKey, Native::DecVal(0x2D8DEB42) /*0x10*/);
	Mem::Xor(Teapot::CachedOriginalDVDKey, 0x2, Native::DecVal(0x2D8DEB42) /*0x10*/);

	if (!fileExists(PATH_KV) && !ini::UseNandKV) {
		debug("[KV] No KV found on HDD. Forcing NoKV to true.");
		ini::NoKVMode = true;
	}

	//if (FAILED(InitKV())) return E_FAIL;
	//if (FAILED(InitMACAddr())) return E_FAIL;;
	//debug("WARNING: TESTING THIS! KV::INIT! DON'T FORGET!!!!!!!!!!!!!!!!!!!!");

	if (!ini::NoKVMode) {
		if (FAILED(InitKV())) return E_FAIL;
		if (FAILED(InitMACAddr())) return E_FAIL;;
		clearCache();
	}else {
		InstallThreadProcServiceSystemTasks();
		//Hypervisor::PokeBytes(HVAddr + 0x110, NoKV_TESTBINARY + 0x110, 0x188);
		//setKeyVaultBytes(NoKV_TESTBINARY);
	}
	return ERROR_SUCCESS;
}

CONSOLE_TYPE KV::getConsoleType() {
	byte KVPolicy[2];

	if (ini::NoKVMode) Mem::Copy(KVPolicy, NoKV.data.KVPolicy, 0x2);
	else Hypervisor::PeekBytes(HVAddr + Native::DecVal(0x7483EB42) /*0x9D1*/, KVPolicy, 0x2);

	byte type = (byte)(((KVPolicy[0] << 4) & Native::DecVal(0x8D8CEB42) /*0xF0*/) | (KVPolicy[1] & Native::DecVal(0x328DEB42) /*0x0F*/));
	if (type < Native::DecVal(0x2D8DEB42) /*0x10*/)  return CONSOLE_TYPE_XENON;
	else if (type < 0x14) return CONSOLE_TYPE_ZEPHYR;
	else if (type < 0x18) return CONSOLE_TYPE_FALCON;
	else if (type < 0x52) return CONSOLE_TYPE_JASPER;
	else if (type < 0x58) return CONSOLE_TYPE_TRINITY;
	else return CONSOLE_TYPE_CORONA;
}
