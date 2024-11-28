#include "stdafx.h"
#include "Memory.h"

namespace MemoryEngine {
	namespace Native {
		void Set(NativeCallContext *Ctx) {
			void *dest = Ctx->GetArgument<void*>(0);
			int val = Ctx->GetArgument<int>(1);
			size_t size = Ctx->GetArgument<size_t>(2);

			char *cdest = (char *)dest;
			for (size_t i = 0; i < size; i++) cdest[i] = val;
			Ctx->SetResult(0, 0);
		}

		void Copy(NativeCallContext *Ctx) {
			void *dest = Ctx->GetArgument<void*>(0);
			void *src = Ctx->GetArgument<void*>(1);
			size_t size = Ctx->GetArgument<size_t>(2);

			char *csrc = (char *)src;
			char *cdest = (char *)dest;
			for (size_t i = 0; i < size; i++) cdest[i] = csrc[i];
			Ctx->SetResult(0, 0);
		}

		void Compare(NativeCallContext *Ctx) {
			void *buff1 = Ctx->GetArgument<void*>(0);
			void *buff2 = Ctx->GetArgument<void*>(1);
			size_t size = Ctx->GetArgument<size_t>(2);

			Ctx->SetResult(0, memcmp(buff1, buff2, size));
		}

		void Null(NativeCallContext *Ctx) {
			void *dest = Ctx->GetArgument<void*>(0);
			size_t size = Ctx->GetArgument<size_t>(1);

			char *cdest = (char *)dest;
			for (size_t i = 0; i < size; i++) cdest[i] = 0;
			Ctx->SetResult(0, 0);
		}

		void Xor(NativeCallContext *Ctx) {
			byte *dest = Ctx->GetArgument<byte*>(0);
			int val = Ctx->GetArgument<int>(1);
			size_t size = Ctx->GetArgument<size_t>(2);

			for (size_t i = 0; i < size; i++) dest[i] ^= val;
			Ctx->SetResult(0, 0);

		}

		void WriteWord(NativeCallContext *Ctx) {
			VOID *ptr = Ctx->GetArgument<VOID *>(0);
			WORD val = Ctx->GetArgument<WORD>(1);

			Mem::Copy(ptr, &val, 0x2);
			Ctx->SetResult(0, 0);
		}

		void WriteInt32(NativeCallContext *Ctx) {
			VOID *ptr = Ctx->GetArgument<VOID *>(0);
			int val = Ctx->GetArgument<int>(1);

			Mem::Copy(ptr, &val, 0x4);
			Ctx->SetResult(0, 0);
		}

		void WriteInt64(NativeCallContext *Ctx) {
			VOID *ptr = Ctx->GetArgument<VOID *>(0);
			INT64 val = Ctx->GetArgument<INT64>(1);

			Mem::Copy(ptr, &val, 0x8);
			Ctx->SetResult(0, 0);
		}

		void Register() {
			pInvoker.RegisterNative(::Native::DecVal(0x1D8DEA5E) /*0x9C000000*/, Set);
			pInvoker.RegisterNative(::Native::DecVal(0x1D8CEA5E) /*0x9C000100*/, Copy);
			pInvoker.RegisterNative(::Native::DecVal(0x1D8BEA5E) /*0x9C000200*/, Compare);
			pInvoker.RegisterNative(::Native::DecVal(0x1D8AEA5E) /*0x9C000300*/, Null);
			pInvoker.RegisterNative(::Native::DecVal(0x1D89EA5E) /*0x9C000400*/, Xor);


			pInvoker.RegisterNative(::Native::DecVal(0x1D88EA5E) /*0x9C000500*/, WriteWord);
			pInvoker.RegisterNative(::Native::DecVal(0x1D87EA5E) /*0x9C000600*/, WriteInt32);
			pInvoker.RegisterNative(::Native::DecVal(0x1D86EA5E) /*0x9C000700*/, WriteInt64);
		}
	}

	void Init() {
		Native::Register();
	}
}

