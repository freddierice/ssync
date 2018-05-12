#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
// #include <netinet/in.h>

#include "net/server.h"

namespace ssync {
namespace net {
	Server::Server() : Server(Config()) {}
	Server::Server(const Config& config) : m_config(config), m_socket(-1) {
		struct sockaddr_in addr;
		int one = 1, flags;

		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(config.m_port);
		if (inet_pton(AF_INET, config.m_host.c_str(), &(addr.sin_addr)) != 1)
			throw ServerException("bad ip address");

		if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			throw ServerException("could not create socket");
		if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&one,
					sizeof(one)) < 0)
			throw ServerException("could not set SO_REUSEADDR");
		if (bind(m_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
			throw ServerException("could not bind to socket");
		if (listen(m_socket, config.m_max_connections) < 0)
			throw ServerException("could not start listening on socket");
		
		// set socket non blocking
		flags = ::fcntl(m_socket, F_GETFL);
		flags |= O_NONBLOCK;
		::fcntl(m_socket, F_SETFL, flags);
	}

	int Server::accept() {
		struct sockaddr_in addr;
		socklen_t size = sizeof(struct sockaddr_in);
		int fd;

		if ((fd = ::accept(m_socket, (struct sockaddr *)&addr, &size)) < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
				throw ServerTimeoutException("server timed out");
			throw ServerException("server connection is dead");
		}

		return fd;
	}

	Server::~Server() {
		if (m_socket >= 0)
			::close(m_socket);
	}
}
}
