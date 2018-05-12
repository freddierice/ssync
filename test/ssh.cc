#include <regex>
#include <iostream>
#include <string>
#include <vector>

#include "net/SSH.h"
#include "test/test.h"

namespace ssync {
namespace test {

	// test
	void ssh(const std::string& username, const std::string& servername ) {
		try {
			net::SSH ssh(username, servername);
			auto buffer = new char[0x10000]();

			auto chan = ssh.run("/bin/dd bs=1M count=10 if=/dev/urandom");
			while (1) {
				chan->read(buffer, sizeof(buffer));
			}
		} catch (net::SSHException &ex) {
			throw Exception(ex.what());
		}
	}
}
}
