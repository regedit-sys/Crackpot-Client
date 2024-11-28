#pragma once
#include "stdafx.h"
#include "globals.h"
#include "cheats/CheatEngine.h"

extern "C" {
	struct GeoIpInfo {
		char country[40];
		char state[40];
		char city[40];
		char zip[40];
		char longitude[40];
		char latitude[40];
		char isp[40];
	};

	__declspec(dllexport) bool Extern_ResolveGeoIP(string IP, byte *out, byte *CE_SecurityDigestIn) {
		if (memcmp(CE_SecurityDigestIn, CE_BaseNameDigest, 0x14) != 0) {
			debug("INVALID SECURITY KEY!");
			return false;
		}

		byte buffer[0x33];
		char result[0x100];
		DWORD status;

		if (IP.length() > 0x17)
			return false;

		Mem::Null(buffer, 0x33);
		memcpy(buffer, Teapot::Session, 0x10);
		strncpy((char*)buffer + 0x10, IP.c_str(), 0x17);

		NET::CLIENT nclient;

		if (FAILED(nclient.create(true))) return false;
		if (SUCCEEDED(nclient.SendCommand(NET_CMD_CE_GEOIP, buffer, 0x33, &status, 0x4))) {
			if (status) nclient.receive(result, 0x100);
			else return false;
			std::string s(result);

			std::string delimiter = ";";
			std::string token;
			int index = 0;
			size_t pos = 0;

			while ((pos = s.find(delimiter)) != std::string::npos) {
				token = s.substr(0, pos);
				strcpy((PCHAR)out + (0x20 * index), token.c_str());
				s.erase(0, pos + delimiter.length());
				index++;
			}
			return true;
		}
		return false;
	}
}