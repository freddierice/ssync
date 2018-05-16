#include <algorithm>
#include <string>

#include "util/parse.h"

namespace ssync {
namespace util {
	void parse_server(const std::string& s, std::string& server, int& port) {

		// default to no port found
		port = -1;

		// split at the colon to get port part
		auto split = std::find(s.begin(), s.end(), ':');
		server = std::string(s.begin(), split);
		if (split == s.end())
			return;

		// check for something after the split
		std::string port_str(split, s.end());
		if (!port_str.length())
			throw ParseException("server cannot end in a colon");
		
		port = std::stoi(std::string(port_str.begin()+1,port_str.end()));
	}
}
}
