#include <atomic>
#include <mutex>

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <cstring>

#include "net/client.h"
#include "log/log.h"

namespace ssync {
namespace net {

	std::atomic<bool> openssl_initialized(false);
	std::mutex openssl_initialized_mutex;

	Client::Client() : Client(Config()) {}
	Client::Client(const Config& config) : m_config(config), m_fd(-1),
	m_rfd(-1), m_wfd(-1), m_ctx(NULL), m_ssl(NULL), m_bio(NULL),
	m_conn(nullptr){
	
    	struct sockaddr_in addr;

		// initialize the openssl library if we haven't
		if (!openssl_initialized) {
			std::unique_lock<std::mutex> lock(openssl_initialized_mutex);
			if (!openssl_initialized) {
				init_openssl();
				openssl_initialized = true;
			}
		}

		addr.sin_family = AF_INET;
		addr.sin_port = htons(m_config.m_port);
		inet_pton(AF_INET, m_config.m_host.c_str(), &addr.sin_addr);

		log::console->info("client: connecting to {}:{}", m_config.m_host,
				m_config.m_port);

		if ((m_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			throw ClientException("could not create socket");

		if (connect(m_fd, (const struct sockaddr *)&addr,
					sizeof(struct sockaddr_in)) == -1)
			throw ClientException("could not connect");
		
		create_context();
		if ((m_ssl = SSL_new(m_ctx)) == NULL)
			throw ClientException("could not setup SSL");
		m_bio = BIO_new_socket(m_fd, BIO_NOCLOSE);
		SSL_set_bio(m_ssl, m_bio, m_bio);
    
		if (SSL_connect(m_ssl) <= 0)
			throw ClientException("could not complete handshake");

		// now that the handshake is complete, we will have new file
		// descriptors pointing toward the SOL_ULP sockets.
		BIO_get_fd(SSL_get_rbio(m_ssl), &m_rfd);
		BIO_get_fd(SSL_get_wbio(m_ssl), &m_wfd);
		m_conn = std::make_shared<net::Connection>(m_rfd, m_wfd);
	}

	Client::~Client() {
		if (m_ctx)
			SSL_CTX_free(m_ctx);
		if (m_ssl)
        	SSL_free(m_ssl);
		if (m_bio)
			BIO_free(m_bio);
        ::close(m_fd);
	}

	std::shared_ptr<net::Connection> Client::conn() {
		return m_conn;
	}

	void Client::init_openssl() { 
		SSL_load_error_strings();	
		OpenSSL_add_ssl_algorithms();
	}

	void Client::cleanup_openssl() {
		// EVP_cleanup();
	}

	void Client::create_context() {
	    const SSL_METHOD *method;

		method = TLS_client_method();
		m_ctx = SSL_CTX_new(method);
		if (!m_ctx)
			throw ClientException("unable to create SSL context");

		SSL_CTX_set_min_proto_version(m_ctx, TLS1_2_VERSION);
		SSL_CTX_set_ecdh_auto(m_ctx, 1);

		STACK_OF(X509_NAME) *cert_names;

		// make sure we verify the server
		SSL_CTX_set_verify(m_ctx, SSL_VERIFY_PEER, NULL);

		// set the ca
		SSL_CTX_load_verify_locations(m_ctx, m_config.m_ca.c_str(), NULL);
		cert_names = SSL_load_client_CA_file(m_config.m_ca.c_str());
		if (cert_names == NULL)
			throw ClientException("unable to read CA");
		SSL_CTX_set_client_CA_list(m_ctx, cert_names);
		if (SSL_CTX_use_certificate_file(m_ctx, m_config.m_cert.c_str(),
					SSL_FILETYPE_PEM) <= 0)
			throw ClientException("unable to read client cert");
		if (SSL_CTX_use_PrivateKey_file(m_ctx, m_config.m_key.c_str(),
					SSL_FILETYPE_PEM) <= 0 )
			throw ClientException("unable to read client key");
	}
		// ERR_print_errors_fp(stderr);
}
}
