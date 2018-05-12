#ifndef __TEST_H__
#define __TEST_H__

namespace ssync {
namespace test {
	class Exception : public std::runtime_error {
		public:
			Exception(std::string const &msg) : std::runtime_error(msg) {};
	};

	void ssh(const std::string& username, const std::string& servername );
}
}

#endif
