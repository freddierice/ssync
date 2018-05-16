#include <atomic>
#include <stdexcept>
#include <exception>
#include <regex>
#include <future>
#include <thread>
#include <iostream>
#include <string>
#include <vector>

#include "net/server.h"
#include "net/SSH.h"
#include "util/parse.h"
#include "log/log.h"
#include "proto/command.pb.h"
#include "fs/file.h"
#include "smart/planner.h"
#include "smart/executor.h"
#include "session/session.h"

#include <cstring>
#include <csignal>
#include <libgen.h>

using namespace ssync;
	
std::atomic<bool> shutdown(false);

void signal_shutdown(int sig);
void handle_client(std::shared_ptr<net::Connection>);

int main(int argc, const char *argv[]) try {
	using namespace ssync::log;

	/*
	int port = 0;
	if (argc > 1) {
		try {
			port = std::stoi(std::string(argv[1]));
		} catch (std::invalid_argument& ex) {
			console->error("invalid port number");
			return 1;
		}
	}
	*/

	// shutdown in a safe way
	std::signal(SIGINT, signal_shutdown);

	console->info("starting");
	net::Server server;
	while (!shutdown) {
		// try to accept a client
		try {
			std::thread thr(handle_client, server.accept());
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

void handle_client(std::shared_ptr<net::Connection> conn) try {
	using namespace ssync::log;
	session::create();

	console->info("accepted client");

	// get a FileList
	proto::Command command;
	while (!conn->recvMessage(command)) {}
	if (command.type() != proto::Command::FILE_LIST)
		throw std::runtime_error("client should have sent a file list");
	
	// get the file list
	console->info("getting file list");
	auto file_list = command.file_list().files();
	const auto file_list_size = file_list.size();
	std::vector<std::string> files;
	for (auto i = 0; i < file_list_size; i++) {
		console->info("got: {}", file_list[i]);
		files.push_back(file_list[i]);
	}

	// get file info
	auto info = fs::File::get_files(files);

	// create the planner
	smart::Planner planner(info);
	smart::Executor executor(conn);
	while (!planner.empty()) {
		// smart::Stats stats = executor.execute(planner.next());
	}

} catch (net::ConnectionException &ex) {
	log::console->error("proto exception: {}", ex.what());
} catch (std::runtime_error &ex) {
	log::console->error("runtime exception: {}", ex.what());
}
