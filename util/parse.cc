#include <algorithm>
#include <string>

#include "util/parse.h"

namespace ssync {
namespace util {
	void parse_server(const std::string& s, std::string& username, std::string& server) {

		// split at the colon to get port part
		auto split = std::find(s.begin(), s.end(), ':');
		std::string port_str(split, s.end());
		if (port_str.length())
			port_str = std::string(port_str.begin()+1,port_str.end());

		// construct server portion
		std::string login_str(s.begin(), split);
		if (!login_str.length())
			throw ParseException("server string needs username");

		auto at_split = std::find(login_str.begin(), login_str.end(), '@');
		if (at_split == login_str.end())
			throw ParseException("string needs username portion");

		username = std::string(login_str.begin(), at_split);
		server = std::string(at_split+1, login_str.end());
	}
}
}
