#include <atomic>
#include <stdexcept>
#include <exception>
#include <regex>
#include <iostream>
#include <string>
#include <vector>

#include "net/server.h"
#include "net/SSH.h"
#include "util/parse.h"
#include "util/proto.h"
#include "log/log.h"
#include "proto/command.pb.h"
#include "fs/file.h"
#include "smart/planner.h"
#include "session/session.h"

#include <cstring>
#include <csignal>
#include <libgen.h>

using namespace ssync;
	
std::atomic<bool> shutdown(false);

void signal_shutdown(int sig);
void handle_client(int fd);

int main(int argc, const char *argv[]) try {
	using namespace ssync::log;

	// shutdown in a safe way
	std::signal(SIGINT, signal_shutdown);

	net::Server server;
	while (!shutdown) {
		// try to accept a client
		try {
			int fd = server.accept();
			std::thread thr(handle_client, fd);
			thr.detach(); // XXX: resource leak
		} catch (net::ServerTimeoutException &ex) {}
	}

	return 0;
} catch (net::ServerException &ex) {
	log::console->error("server exception: {}", ex.what());
	return 1;
} catch (util::SSyncException &ex) {
	log::console->error("uncaught exception: {}", ex.what());
	return 1;
}

void signal_shutdown(int sig) {
	ssync::log::console->warn("shutting down");
	shutdown = true;
}

void handle_client(int fd) try {
	using namespace ssync::log;
	session::create();

	console->info("accepted {}", fd);
	util::Proto proto(fd);

	// get a FileList
	proto::Command command;
	while (!proto.recvMessage(command)) {}
	if (command.type() != proto::Command::FILE_LIST)
		throw std::runtime_error("client should have sent a file list");
	
	// get the file list
	auto file_list = command.file_list().files();
	const auto file_list_size = file_list.size();
	std::vector<std::string> files;
	for (auto i = 0; i < file_list_size; i++)
		files.push_back(file_list[i]);

	// get file info
	auto info = fs::File::get_files(files);

	// generate a plan
	smart::Planner planner(info);
	
	// execute the plan

	close(fd);
} catch (util::ProtoException &ex) {
	log::console->error("proto exception: {}", ex.what());
} catch (std::runtime_error &ex) {
	log::console->error("runtime exception: {}", ex.what());
}
