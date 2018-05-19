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
#include "fs/rfile.h"
#include "fs/wfile.h"
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

	console->info("starting server");
	net::Server server;
	console->info("accepting");
	while (!shutdown) {
		// try to accept a client
		try {
			auto conn = server.accept();
			std::thread thr(handle_client, conn);
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
	if (!command.files_size())
		throw std::runtime_error("client should have sent a file list");
	
	// get the file list
	console->info("getting file list");
	const auto const_files = command.files();
	const auto files_size = command.files_size();
	std::vector<std::string> filenames;
	for (auto i = 0; i < files_size; i++) {
		console->info("got: {}", const_files[i].name());
		filenames.push_back(const_files[i].name());
	}

	// get file info
	std::list<std::shared_ptr<fs::RFile>> info = fs::get_files(filenames);

	// send back file info
	google::protobuf::RepeatedPtrField<proto::File>* files = command.mutable_files();
	int i = 0;
	for (auto iter = info.begin(); iter != info.end(); ++iter)
		files->Mutable(i++)->set_size((*iter)->size());

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
