#pragma once

class Alloc {
public:
	byte *Data;
	size_t BlockSize;
	bool Encrypted;

	Alloc(size_t Size, bool Encrypted = false) {
		this->BlockSize = Size;
		this->Encrypted = Encrypted;
		this->Data = (byte*)Native::Malloc(Size);
	}
};

namespace MemoryEngine {
	namespace Native {
		void Register();
	}

	static void Set(void *dest, int val, size_t size) {
		pInvoker.Invoke<DWORD>(0x9C000000, dest, val, size);
	}

	static void Copy(void *dest, const void *src, size_t size) {
		pInvoker.Invoke<DWORD>(0x9C000100, dest, src, size);
	}

	static int Compare(void *buff1, const void *buff2, size_t size) {
		return pInvoker.Invoke<DWORD>(0x9C000200, buff1, buff2, size);
	}

	static void Null(void *dest, size_t size) {
		pInvoker.Invoke<DWORD>(0x9C000300, dest, size);
	}

	static void Xor(void *dest, int val, size_t size) {
		pInvoker.Invoke<DWORD>(0x9C000400, dest, val, size);
	}

	static void WriteShort(short * ptr, short val) {
		pInvoker.Invoke<DWORD>(0x9C000500, ptr, val);
	}


	static void WriteInt32(INT32 * ptr, int val) {
		pInvoker.Invoke<DWORD>(0x9C000600, ptr, val);
	}

	static void WriteInt64(INT64 * ptr, INT64 val) {
		pInvoker.Invoke<DWORD>(0x9C000700, ptr, val);
	}

	void Init();
}

namespace Mem = MemoryEngine;
