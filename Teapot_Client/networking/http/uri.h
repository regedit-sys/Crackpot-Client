#pragma once

namespace http {
	struct uri {
	public:
		std::string query_string, path, protocol, host;
		unsigned int port;

		std::string build() {
			std::string ret(protocol);

			bool https = protocol.compare("https") == 0;

			if (!ret.length()) ret.assign("http");
			ret.append("://");
			ret.append(host);
			
			int defaultPort = https ? 443 : 80;

			if (port != defaultPort) {
				ret.append(":");
				ret.append(std::to_string((unsigned long long)port));
			}

			ret.append(path);
			ret.append(query_string);

			return ret;
		}

		static uri parse(const std::string &uri) {
			http::uri _result;

			typedef std::string::const_iterator iterator_t;

			if (uri.length() == 0)
				return _result;

			iterator_t uriEnd = uri.end();

			// get query start
			iterator_t queryStart = std::find(uri.begin(), uriEnd, '?');

			// protocol
			iterator_t protocolStart = uri.begin();
			iterator_t protocolEnd = std::find(protocolStart, uriEnd, ':');

			if (protocolEnd != uriEnd) {
				std::string prot = &*(protocolEnd);
				if ((prot.length() > 3) && (prot.substr(0, 3) == "://")) {
					_result.protocol = std::string(protocolStart, protocolEnd);
					protocolEnd += 3;   //      ://
				} else
					protocolEnd = uri.begin();  // no protocol
			} else
				protocolEnd = uri.begin();  // no protocol

			// protocol to lower
			std::transform(_result.protocol.begin(), _result.protocol.end(), _result.protocol.begin(), ::tolower);

			bool https = _result.protocol.compare("https") == 0;

			// host
			iterator_t hostStart = protocolEnd;
			iterator_t pathStart = std::find(hostStart, uriEnd, '/');  // get pathStart

			iterator_t hostEnd = std::find(protocolEnd, 
				(pathStart != uriEnd) ? pathStart : queryStart,
				L':');  // check for port

			_result.host = std::string(hostStart, hostEnd);

			// port
			if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == ':')) {  // we have a port
				hostEnd++;
				iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
				//if (!(std::istringstream(std::string(hostEnd, portEnd)) >> _result.port)) _result.port = https ? 443 : 80;
				_result.port = 80;
			} else {
				_result.port = https ? 443 : 80;
			}

			// path
			if (pathStart != uriEnd)
				_result.path = std::string(pathStart, queryStart);

			// query
			if (queryStart != uriEnd)
				_result.query_string = std::string(queryStart, uri.end());

			return _result;
		}
	};
}