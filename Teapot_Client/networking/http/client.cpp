#include "stdafx.h"
#include "client.h"
#include "globals.h"

#define CFG_SOCKET_TIMEOUT 3500

static inline std::string &ltrim(std::string &str) {
	str.erase(str.begin(), std::find_if_not(str.begin(), str.end(), std::isspace));
	return str;
}

static inline std::string &rtrim(std::string &str) {
	str.erase(std::find_if_not(str.rbegin(), str.rend(), std::isspace).base(), str.end());
	return str;
}

static inline std::string &trim(std::string &s) {
	return ltrim(rtrim(s));
}

HRESULT http::connection::init_connection(SOCKADDR_IN remote) {
	// only run if not currently connected
	if (connected) {
		return WSAEISCONN;
	}

	// create socket
	sock = NetDll_socket(XNCALLER_SYSAPP, AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == SOCKET_ERROR || sock == INVALID_SOCKET) {
		return NetDll_WSAGetLastError();
	}

	// ghetto rig socket to disable encryption
	DWORD val = 1;
	if (NetDll_setsockopt(XNCALLER_SYSAPP, sock, SOL_SOCKET, SO_MARKINSECURE, (PCSTR)&val, sizeof(DWORD)) != 0) {
		NetDll_closesocket(XNCALLER_SYSAPP, sock);
		return NetDll_WSAGetLastError();
	}

	DWORD timeout = 3000;
	if (NetDll_setsockopt(XNCALLER_SYSAPP, sock, SOL_SOCKET, SO_RCVTIMEO, (PCSTR)&timeout, sizeof(DWORD)) != 0) {
		NetDll_closesocket(XNCALLER_SYSAPP, sock);
		return NetDll_WSAGetLastError();
	}

	if (NetDll_setsockopt(XNCALLER_SYSAPP, sock, SOL_SOCKET, SO_SNDTIMEO, (PCSTR)&timeout, sizeof(DWORD)) != 0) {
		NetDll_closesocket(XNCALLER_SYSAPP, sock);
		return NetDll_WSAGetLastError();
	}

	// connect socket
	if (NetDll_connect(XNCALLER_SYSAPP, sock, (struct sockaddr*)&remote, sizeof(remote)) != 0) {
		NetDll_closesocket(XNCALLER_SYSAPP, sock);
		return NetDll_WSAGetLastError();
	}

	// create a socket stream
	if (int ret = init_sockiostream() != 0) {
		NetDll_closesocket(XNCALLER_SYSAPP, sock);
		return ret;
	}

	connected = true;
	return S_OK;
}

HRESULT http::connection::disconnect() {
	// only run if currently connected
	if (!connected) {
		return WSAENOTCONN;
	}

	// free sockstream if allocated
	if (sockstream != NULL)
		delete sockstream;

	sockstream = NULL;

	// signal connection shutdown and close socket
	NetDll_shutdown(XNCALLER_SYSAPP, sock, SD_BOTH);
	NetDll_closesocket(XNCALLER_SYSAPP, sock);

	connected = false;
	return S_OK;
}

std::string http::client::build_query(std::string &method, http::uri &uri) {
	std::ostringstream output;

	// convert method to uppercase
	std::transform(method.begin(), method.end(), method.begin(), ::toupper);

	// build the query
	output << method << " " << uri.path << uri.query_string << " HTTP/1.0" << "\r\n";

	return output.str();
}

std::string http::client::build_headers(std::map<std::string, std::string> &additional_headers, std::string &host, size_t content_length) {
	std::map<std::string, std::string> final;
	std::map<std::string, std::string>::iterator it;
	std::ostringstream output;

	// insert additional headers first
	final.insert(additional_headers.begin(), additional_headers.end());

	// add any values from the default headers which are not overridden by additional_headers
	final.insert(headers.begin(), headers.end());

	// add host header
	if (!host.empty()) final["Host"] = host;

	// add content length header
	if (content_length > 0) final["Content-Length"] = std::to_string((long double)content_length);

	// iterate the final headers and add them to the output
	for (it = final.begin(); it != final.end(); ++it) {
		output << it->first << ": " << it->second << "\r\n";
	}

	return output.str();
}

bool http::client::build_sockaddr(http::uri &uri, SOCKADDR_IN &saddr) {
	// set family and port
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(uri.port);

	// try parse IP
	DWORD ip = inet_addr(uri.host.c_str());

	// check if IP parse succeeded
	if (ip != INADDR_NONE && ip != NULL) {
		saddr.sin_addr.s_addr = ip;
		return true;
	}

	// need to lookup DNS
	//if (network::dnsLookup(uri.host.c_str(), &saddr.sin_addr, true) == S_OK) {
	//	return true;
	//}

	// DNS lookup failed
	return false;
}

http::connection *http::client::build_connection(SOCKADDR_IN &saddr) {
	http::connection *connection = new http::connection();

	// open connection
	connection->init_connection(saddr);

	// return connection regardless of success
	return connection;
}

bool http::client::read_headers(network::sockiostream &stream, std::map<std::string, std::string> &dest) {
	size_t i;

	for (std::string line; std::getline(stream, line); ) {
		// trim the line first
		trim(line);

		// empty line signals end of headers
		if (line.empty()) return true;

		// find delimiter
		i = line.find(':');

		// malformed line
		if (i == std::string::npos) return false;

		// get the header key and value from the line and trim
		std::string key(trim(line.substr(0, i)));
		std::string val(trim(line.substr(i + 1, line.length())));

		// convert key to lowercase
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);

		// insert the values to the header map
		dest[key] = val;
	}

	// falling out of for loop indicates empty line never received
	return false;
}

bool http::client::read_data(http::connection &connection, size_t content_length, std::vector<unsigned char> &data) {
	// allocate buffer for data
	char *pdata = (char*)malloc(content_length);

	// check the pointer
	if (!pdata) return false;

	// read data off line
	if (!connection.sockstream->read(pdata, content_length)) {
		// read failed, free buffer
		free(pdata);
		return false;
	}

	// copy data into output vector and free buffer
	data.assign(pdata, pdata + content_length);
	free(pdata);

	return true;
}

http::response http::client::do_query(std::string &query_type, http::request &request) {
	http::response response(request);
	http::connection *connection;
	std::ostringstream request_head;
	std::string status_line, request_head_data;
	size_t content_length;
	SOCKADDR_IN saddr;
	http::uri uri;

	// parse uri
	uri = http::uri::parse(request.url);

#ifdef _PROFILE_HTTP_CLIENT
	QueryPerformanceFrequency(&request.frequency);
	QueryPerformanceCounter(&request.dnsStart);
#endif

	// build sockaddr
	if (!build_sockaddr(uri, saddr)) return response;

	// build query
	std::string query(build_query(query_type, uri));

	// build request length
	content_length = request.data.size();

	// build headers
	std::string header_data(build_headers(request.headers, uri.host, content_length));

#ifdef _PROFILE_HTTP_CLIENT
	QueryPerformanceCounter(&request.connectStart);
#endif

	// build connection
	connection = build_connection(saddr);

	// bail out if connection failed
	if (!connection->connected) {
		goto fail;
	}

#ifdef _PROFILE_HTTP_CLIENT
	QueryPerformanceCounter(&request.writeStart);
#endif

	// build and send request head
	request_head << query << header_data << "\r\n";
	request_head_data.assign(request_head.str());
	connection->sockstream->write(request_head_data.c_str(), request_head_data.length());

	// send request body
	if (content_length > 0) {
		connection->sockstream->write((const char *)(&request.data.front()), request.data.size());
		if (!(*connection->sockstream)) goto fail;
	}

	// force flush
	connection->sockstream->flush();

#ifdef _PROFILE_HTTP_CLIENT
	QueryPerformanceCounter(&request.readStart);
#endif

	// read response status
	if (!std::getline(*connection->sockstream, status_line)) {
		goto fail;
	}

	// check response line http version
	/*if (status_line.compare(0, 9, "HTTP/1.0 ") != 0 && !status_line.compare(0, 9, "HTTP/1.1 ")) {
		debug("http: failed to validate http version");
		goto fail;
	}*/

	// parse response code
	if ((response.http_code = strtol(status_line.substr(9).c_str(), NULL, 10)) == 0) {
		goto fail;
	}

	// read response headers
	if (!read_headers(*connection->sockstream, response.headers)) {
		goto fail;
	}

	// be sure there is a content-length header
	response.headers.insert(std::pair<std::string, std::string>("content-length", "0"));

	// parse content length header
	if (!(std::istringstream(response.headers["content-length"]) >> content_length)) content_length = 0;

	// read data if there is any
	if (content_length > 0 && !read_data(*connection, content_length, response.data)) {
		// read failed
		goto fail;
	}

#ifdef _PROFILE_HTTP_CLIENT
	QueryPerformanceCounter(&request.complete);
#endif

	// close connection
	connection->disconnect();

	// it actually worked? no fucking way
	response.success = true;

#ifdef _PROFILE_HTTP_CLIENT
	double totalTime = (double)(request.complete.QuadPart - request.dnsStart.QuadPart) / (double)request.frequency.QuadPart;
	double dnsTime = (double)(request.connectStart.QuadPart - request.dnsStart.QuadPart) / (double)request.frequency.QuadPart;
	double connectTime = (double)(request.writeStart.QuadPart - request.connectStart.QuadPart) / (double)request.frequency.QuadPart;
	double writeTime = (double)(request.readStart.QuadPart - request.writeStart.QuadPart) / (double)request.frequency.QuadPart;
	double readTime = (double)(request.complete.QuadPart - request.readStart.QuadPart) / (double)request.frequency.QuadPart;

	debug("%s request statistics:", request.url.c_str());
	debug("dns lookup: %.3f seconds", dnsTime);
	debug("connect: %.3f seconds", connectTime);
	debug("write: %.3f seconds", writeTime);
	debug("read: %.3f seconds", readTime);
	debug("total: %.3f seconds", totalTime);
#endif

fail:
	// free connection if allocated
	if (connection != NULL)
		delete connection;

	return response;
}

http::response http::client::get(http::request &request) {
	return do_query(std::string("GET"), request);
}

http::response http::client::post(http::request &request) {
	return do_query(std::string("POST"), request);
}