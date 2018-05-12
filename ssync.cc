#include <regex>
#include <iostream>
#include <string>
#include <vector>

#include "net/SSH.h"
#include "util/parse.h"

#include <cstring>
#include <libgen.h>

int main(int argc, const char *argv[]) {

	if (argc <= 3) {
		std::cerr << "usage: " << argv[0] << 
			" <user@server[:port]> <list of files an dirs> ... <target or target dir>" << std::endl;
		return 1;
	}

	std::string username, servername;
	std::string server_string = std::string(argv[1]);
	try {
		ssync::util::parse_server(server_string, username, servername);
	} catch (ssync::util::ParseException &ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	
	// parse the file list
	std::vector<std::string> filenames;
	for (int i = 1; i < argc-1; i++) {
		filenames.push_back(std::string(argv[i]));
	}
	std::string target(argv[argc-1]);

	try {
		ssync::net::SSH ssh(username, servername);
		char buffer[0x10000];

		auto chan = ssh.run("/bin/dd bs=1M count=10 if=/dev/urandom");
		while (chan->open()) {
			int i = chan->read(static_cast<void *>(buffer), sizeof(buffer));
			if (i > 0) 
				std::cerr << "read " << i << std::endl;
		}
	} catch (ssync::net::SSHException &ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	
	return 0;
}

