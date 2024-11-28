#include "stdafx.h"
#include "common.h"
#include "sockiostream.h"

namespace network {
	sockiobuf::int_type sockiobuf::underflow() {
		if (gptr() < egptr()) {
			return traits_type::to_int_type(*gptr());
		}

		int numPutback;
		numPutback = gptr() - eback();
		if (numPutback > pbSize) {
			numPutback = pbSize;
		}

		memmove(buffer+(pbSize-numPutback), gptr()-numPutback, numPutback);

		int num;
		num = 
			NetDll_recv(XNCALLER_SYSAPP, sock, buffer+pbSize, bufSize, NULL);
		if (num <= 0) {
			return EOF;
		}

		setg (buffer+(pbSize-numPutback),  
			buffer+pbSize,              
			buffer+pbSize+num);         

		return traits_type::to_int_type(*gptr());
	}

	sockiobuf::int_type sockiobuf::overflow(int_type c) {
		if (c != EOF) {
			char z = c;
			if (NetDll_send(XNCALLER_SYSAPP, sock, &z, 1, NULL) != 1) {
				return c = EOF;
			}
		}

		return traits_type::to_int_type(c);
	}

	std::streamsize sockiobuf::xsputn(const char *s, std::streamsize num) {
		return NetDll_send(XNCALLER_SYSAPP, sock, s, (int)num, NULL);
	}
}