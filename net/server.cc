#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <openssl/ssl.h>

#include "net/server.h"

namespace ssync {
namespace net {
	Server::Server() : Server(Config()) {}
	Server::Server(const Config& config) : m_config(config), m_fd(-1),
   		m_ctx(NULL) {
		struct sockaddr_in addr;
		int one = 1;

		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(config.m_port);
		if (inet_pton(AF_INET, config.m_host.c_str(), &(addr.sin_addr)) != 1)
			throw ServerException("bad ip address");

		if ((m_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			throw ServerException("could not create socket");
		if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&one,
					sizeof(one)) < 0)
			throw ServerException("could not set SO_REUSEADDR");
		if (bind(m_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
			throw ServerException("could not bind to socket");
		if (listen(m_fd, config.m_max_connections) < 0)
			throw ServerException("could not start listening on socket");
		
		// TODO: play around with non-blocking
		// set socket non blocking
		/*
		flags = ::fcntl(m_socket, F_GETFL);
		flags |= O_NONBLOCK;
		::fcntl(m_socket, F_SETFL, flags);
		*/
	}

	std::shared_ptr<net::Connection> Server::accept() {
		struct sockaddr_in addr;
		socklen_t size = sizeof(struct sockaddr_in);
		int fd;

		if ((fd = ::accept(m_fd, (struct sockaddr *)&addr, &size)) < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
				throw ServerTimeoutException("server timed out");
			throw ServerException("server connection is dead");
		}
		
		SSL *ssl = SSL_new(m_ctx);
		if (!ssl)
			throw ServerException("could not create SSL");
		if (SSL_accept(ssl) <= 0)
			throw ServerException("could not accept client");
		
		int rfd, wfd;
		BIO_get_fd(SSL_get_rbio(ssl), &rfd);
		BIO_get_fd(SSL_get_wbio(ssl), &wfd);

		return std::make_shared<net::Connection>(rfd, wfd);
	}

	void Server::create_context() {
		STACK_OF(X509_NAME) *cert_names;

		// make sure we have at least TLS 1.2
		SSL_CTX_set_min_proto_version(m_ctx, TLS1_2_VERSION);
		// use a secure curve for ecdh
		SSL_CTX_set_ecdh_auto(m_ctx, 1);
		// make sure that we verify the client
		SSL_CTX_set_verify(m_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);

		// use only ca.pem
		SSL_CTX_load_verify_locations(m_ctx, m_config.m_ca.c_str(), NULL);
		cert_names = SSL_load_client_CA_file(m_config.m_ca.c_str());
		if (cert_names == NULL)
			throw ServerException("could not read from CA");

		SSL_CTX_set_client_CA_list(m_ctx, cert_names);
		SSL_CTX_set_verify_depth(m_ctx, 0);
		if (SSL_CTX_use_certificate_file(m_ctx, m_config.m_cert.c_str(),
					SSL_FILETYPE_PEM) <= 0)
			throw ServerException("could not read cert");
		if (SSL_CTX_use_PrivateKey_file(m_ctx, m_config.m_key.c_str(),
					SSL_FILETYPE_PEM) <= 0 )
			throw ServerException("could not read key");

		// use KTLS available cipher
		if(!SSL_CTX_set_cipher_list(m_ctx, "ECDHE-ECDSA-AES128-GCM-SHA256"))
			throw ServerException("could not set the cipher list");
	}

	Server::~Server() {
		if (m_fd >= 0)
			::close(m_fd);
	}
}
}
