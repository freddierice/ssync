#ifndef __PARSE_H__
#define __PARSE_H__

#include <stdexcept>

namespace ssync {
namespace util {
	class ParseException: std::runtime_error {
		public:
			ParseException(const std::string& msg) : 
				std::runtime_error(msg), m_msg(msg) {}
			virtual const char *what() {
				return m_msg.c_str();
			}
		private:
			const std::string m_msg;
	};

	void parse_server(const std::string& s, std::string& server, int& port);
}
}

#endif /* __PARSE_H__ */
