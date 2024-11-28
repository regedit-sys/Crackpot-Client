#pragma once

namespace network {
	class sockiobuf : public std::streambuf {
	protected:
		SOCKET sock;
		static const int pbSize = 4;        // size of putback area
		static const int bufSize = 1024;    // size of the data buffer
		char buffer[bufSize+pbSize];        // data buffer

	public:
		sockiobuf (SOCKET _sock) : sock(_sock) {
			setg (buffer+pbSize,     // beginning of putback area
				buffer+pbSize,     // read position
				buffer+pbSize);    // end position
		}

	protected:
		virtual int_type underflow();
		virtual int_type overflow(int_type c);
		virtual std::streamsize xsputn(const char *s, std::streamsize num);
	};

	class sockiostream : public std::iostream, public sockiobuf {
	public:
		sockiostream(SOCKET sock) : std::iostream(0), sockiobuf(sock) {
			rdbuf(this);
		}
	};
}