#pragma once

#define ERROR_UPDATER_REQ_CREATE     0xC4000001
#define ERROR_UPDATER_REQ_CMD		 0xC4000002
#define ERROR_UPDATER_INVALID_SIZE   0xC4000003
#define ERROR_UPDATER_ALLOCATE       0xC4000004
#define ERROR_UPDATER_RECV_IMAGE     0xC4000005
#define ERROR_UPDATER_WRITEIMAGE     0xC4000006
#define ERROR_UPDATER_INTEG_OPENFAIL 0xC4000007
#define ERROR_UPDATER_INTEG_UNKMODE  0xC4000008
#define ERROR_UPDATER_INTEG_FAILED   0xC4000009 //(0x09 - 0x0A is reserved)

namespace Teapot {
	class C_UPDATER
	{
		enum INTEGRITY_MODE {
			MODE_PREWRITE,
			MODE_POSTWRITE
		};

#pragma pack(1)
		struct UPDATE_RESPONSE {
			bool Critical;
			byte Digest[0x14];
			size_t ImageSize;
			byte *Image;
		};
#pragma pack()

	private:
		NET::CLIENT Client;
		UPDATE_RESPONSE Pkg;
		size_t CachedImageSize;
		bool AllocationSuccess;
		bool WasError;
		bool Force;

		HRESULT VerifyIntegrity(INTEGRITY_MODE Mode);
		HRESULT SendCMD();
		HRESULT Allocate();
		HRESULT DownloadImage();
		HRESULT WriteUpdate();
		HRESULT Query();

	public:
		C_UPDATER(bool Force = false);
		~C_UPDATER();
		void InstallLatest();
	};
}
