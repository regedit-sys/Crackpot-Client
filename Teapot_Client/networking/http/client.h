#pragma once
#include "../../stdafx.h"
#include "sockiostream.h"
#include "uri.h"




namespace http {
	/*
	 * Encapsulates a single http connection
	 * does not support keep-alive
	 * one http_connection == one htt_request
	 */
	class connection {
	protected:
		SOCKET sock;
		std::streamsize line_buf_size;

		virtual int init_sockiostream() {
			sockstream = new network::sockiostream(sock);

			return sockstream == 0;
		}

	public:
		bool connected;
		network::sockiostream *sockstream;

		connection(std::streamsize _line_buf_size = 1024) :
			line_buf_size(_line_buf_size),
			connected(false),
			sock(INVALID_SOCKET) { }

		~connection() {
			disconnect();
		}

		HRESULT init_connection(SOCKADDR_IN remote);

		HRESULT disconnect();
	};

	/*
	 * Information needed to send a HTTP request
	 */
	class request {
	public:
		std::string url;
		std::map<std::string, std::string> headers;
		std::vector<unsigned char> data;

#ifdef _PROFILE_HTTP_CLIENT
		LARGE_INTEGER frequency;
		LARGE_INTEGER dnsStart;
		LARGE_INTEGER connectStart;
		LARGE_INTEGER writeStart;
		LARGE_INTEGER readStart;
		LARGE_INTEGER complete;
#endif
	};

	/*
	 * A response to a HTTP request
	 */
	class response {
	public:
		response(request &req) : request(req), success(false) {}

		bool success;
		int http_code;
		request request;
		std::map<std::string, std::string> headers;
		std::vector<unsigned char> data;
	};

	/*
	 * An object which can send HTTP requests and receive responses to them
	 */
	class client {
	public:
		std::map<std::string, std::string> headers;

		client() {
			headers["User-Agent"] = "Xbox/TPAPI";
		}

		response get(request &request);
		response post(request &request);

	protected:
		std::string build_headers(std::map<std::string, std::string> &additional_headers, std::string &host, size_t content_length);
		bool read_headers(network::sockiostream &stream, std::map<std::string, std::string> &dest);
		bool read_data(connection &connection, size_t content_length, std::vector<unsigned char> &data);
		std::string build_query(std::string &method, http::uri &uri);
		bool build_sockaddr(http::uri &uri, SOCKADDR_IN &saddr);
		virtual connection *build_connection(SOCKADDR_IN &saddr);
		response do_query(std::string &query_type, request &request);
	};
}