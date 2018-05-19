#include <regex>
#include <iostream>

#include <experimental/filesystem>
namespace filesystem = std::experimental::filesystem;

#include <string>
#include <vector>
#include <thread>

#include "net/SSH.h"
#include "net/client.h"
#include "net/connection.h"
#include "util/parse.h"
#include "proto/command.pb.h"
#include "log/log.h"
#include "fs/wfile.h"

#include <cstring>
#include <libgen.h>
using namespace ssync;
using namespace ssync::log;

int main(int argc, const char *argv[]) {

	if (argc <= 3) {
		std::cerr << "usage: " << argv[0] << 
			" <server[:port]> <list of files> ... <target or target dir>" << std::endl;
		return 1;
	}

	int port;
	std::string server_name;
	std::string server_string = std::string(argv[1]);
	try {
		util::parse_server(server_string, server_name, port);
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
	proto::Command command;
	auto files = command.mutable_files();
	for (int i = 1; i < argc-1; i++) {
		auto p = std::experimental::filesystem::path(argv[i]);
		locals.push_back(target / p.filename());
		auto file = files->Add();
		file->set_name(p.string());
	}

	net::Client::Config client_config;
	client_config.set_host(server_name.c_str());
	if (port != -1)
		client_config.set_port(port);

	try {
		console->info("connecting to server");
		net::Client client(client_config);
		auto conn = client.conn();
		
		console->info("sending files");
		conn->sendMessage(command);
		
		console->info("clearing");
		command.Clear();

		console->info("getting the files");
		conn->recvMessage(command);

		if (!command.files_size()) {
			console->error("server did not send files");
			return 1;
		}

		std::vector<std::string> file_list;
		std::vector<int> size_list;
		for (const auto& file : command.files()) {
			file_list.push_back(file.name());
			size_list.push_back(file.size());
		}

		auto files = fs::create_files(file_list, size_list);
	
		while (1) {
			console->info("waiting for plan");
			conn->recvMessage(command);
			if (command.error_msg().length()) {
				console->error("error from server: {}", command.error_msg());
				break;
			}
			if (!command.has_plan()) {
				console->error("did not recieve a plan");
				break;
			}

			auto plan = command.plan();
			for (const auto& iter : plan.files()) {
				console->info("files: {}", iter);
			}
			break;
		}

	} catch (net::ClientException &ex) {
		console->error("caught client exception: {}", ex.what());
		return 1;
	}

	/*
	try {
		console->info("logging into {}@{}", username, servername);
		net::SSH ssh(username, servername);
		// char buffer[0x10000];

		auto chan = ssh.conn(net::Server::Config::DEFAULT_HOST,
				net::Server::Config::DEFAULT_PORT);
		
		int client_fd, proxy_fd;
		net::create_proxy(client_fd, proxy_fd);
		std::thread thr(net::run_proxy, proxy_fd, chan);

		console->info("sending file list");
		util::Proto proto(client_fd);
		proto.sendMessage(command);
		command.Clear();
		
		console->info("joining with proxy");
		thr.join();
		while (chan->open()) {
			int i = chan->read(static_cast<void *>(buffer), sizeof(buffer));
			if (i > 0) 
				std::cerr << "read " << i << std::endl;
		}
	} catch (net::SSHException &ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	*/
	
	return 0;
}

