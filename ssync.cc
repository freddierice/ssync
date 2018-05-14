#include <regex>
#include <iostream>

#include <experimental/filesystem>
namespace filesystem = std::experimental::filesystem;

#include <string>
#include <vector>
#include <thread>

#include "net/SSH.h"
#include "net/client.h"
#include "net/server.h"
#include "util/parse.h"
#include "util/proto.h"
#include "proto/command.pb.h"
#include "log/log.h"

#include <cstring>
#include <libgen.h>
using namespace ssync;
using namespace ssync::log;

int main(int argc, const char *argv[]) {

	if (argc <= 3) {
		std::cerr << "usage: " << argv[0] << 
			" <user@server[:port]> <list of files> ... <target or target dir>" << std::endl;
		return 1;
	}

	std::string username, servername;
	std::string server_string = std::string(argv[1]);
	try {
		util::parse_server(server_string, username, servername);
	} catch (util::ParseException &ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	
	// get the target
	filesystem::path target(argv[argc-1]);
	target = std::experimental::filesystem::current_path() / target;
	std::experimental::filesystem::create_directories(target);

	// parse the file list
	std::vector<std::string> locals;
	proto::FileList file_list;
	for (int i = 1; i < argc-1; i++) {
		auto p = std::experimental::filesystem::path(argv[i]);
		locals.push_back(target / p.filename());
		file_list.add_files(p.string());
	}

	try {
		net::SSH ssh(username, servername);
		// char buffer[0x10000];

		auto chan = ssh.conn(net::Server::Config::DEFAULT_HOST,
				net::Server::Config::DEFAULT_PORT);
		
		int read_fd, write_fd;
		net::create_proxy(read_fd, write_fd);
		std::thread thr(net::run_proxy, read_fd, write_fd, chan);
		thr.detach();

		console->info("sending file list");
		util::Proto proto(read_fd, write_fd);
		proto::Command command;
		command.set_type(proto::Command::FILE_LIST);
		command.set_allocated_file_list(&file_list);
		proto.sendMessage(command);
		
		/*
		while (chan->open()) {
			int i = chan->read(static_cast<void *>(buffer), sizeof(buffer));
			if (i > 0) 
				std::cerr << "read " << i << std::endl;
		}
		*/
	} catch (net::SSHException &ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	
	return 0;
}

