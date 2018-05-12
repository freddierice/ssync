#include <atomic>
#include <regex>
#include <iostream>
#include <string>
#include <vector>

#include "net/server.h"
#include "net/SSH.h"
#include "util/parse.h"
#include "log/log.h"

#include <cstring>
#include <csignal>
#include <libgen.h>

std::atomic<bool> shutdown(false);

void signal_shutdown(int sig) {
	ssync::log::console->warn("shutting down");
	shutdown = true;
}

int main(int argc, const char *argv[]) try {
	using namespace ssync;
	using namespace ssync::log;

	// shutdown in a safe way
	std::signal(SIGINT, signal_shutdown);

	net::Server server;
	while (!shutdown) {
		// try to accept a client
		try {
			int fd = server.accept();
			console->info("accepted {}", fd);
			close(fd);
		} catch (net::ServerTimeoutException &ex) {}
	}

	return 0;
} catch (ssync::net::ServerException &ex) {
	ssync::log::console->error("server exception: {}", ex.what());
	return 1;
} catch (ssync::util::SSyncException &ex) {
	ssync::log::console->error("uncaught exception: {}", ex.what());
	return 1;
}
