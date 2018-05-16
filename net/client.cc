#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <cstring>

#include "net/client.h"

namespace ssync {
namespace net {

	Client::Client() : Client(Config()) {}
	Client::Client(const Config& config) : m_config(config), m_fd(-1),
   		m_ctx(NULL) {
    	struct sockaddr_in addr;

		addr.sin_family = AF_INET;
		addr.sin_port = htons(m_config.m_port);
		inet_pton(AF_INET, m_config.m_host.c_str(), &addr.sin_addr);

		if ((m_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			throw ClientException("could not create socket");

		if (connect(m_fd, (const struct sockaddr *)&addr,
					sizeof(struct sockaddr_in)) == -1)
			throw ClientException("could not connect");
		
		m_ctx = create_context();
		if ((m_ssl = SSL_new(m_ctx)) == NULL)
			throw ClientException("could not setup SSL");

		SSL_set_fd(m_ssl, m_fd);
    
		if (SSL_connect(m_ssl) <= 0)
			throw ClientException("could not complete handshake");

	}

	Client::~Client() {
		if (m_ctx)
			SSL_CTX_free(m_ctx);
		if (m_ssl)
        	SSL_free(m_ssl);
        ::close(m_fd);
	}

	void Client::init_openssl() { 
		SSL_load_error_strings();	
		OpenSSL_add_ssl_algorithms();
	}

	void Client::cleanup_openssl() {
		// EVP_cleanup();
	}

	SSL_CTX *Client::create_context() {
	    const SSL_METHOD *method;
	    SSL_CTX *ctx;

		method = TLS_client_method();
		ctx = SSL_CTX_new(method);
		if (!ctx)
			throw ClientException("unable to create SSL context");
		// ERR_print_errors_fp(stderr);

		SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
		SSL_CTX_set_ecdh_auto(ctx, 1);

		STACK_OF(X509_NAME) *cert_names;

		// make sure we verify the server
		SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

		// set the ca
		SSL_CTX_load_verify_locations(ctx, "ca.pem", NULL);
		cert_names = SSL_load_client_CA_file("ca.pem");
		if (cert_names == NULL) {
			ERR_print_errors_fp(stderr);
			exit(EXIT_FAILURE);
		}
		SSL_CTX_set_client_CA_list(ctx, cert_names);

		if (SSL_CTX_use_certificate_file(ctx, "client.pem", SSL_FILETYPE_PEM) <= 0) {
			ERR_print_errors_fp(stderr);
			exit(EXIT_FAILURE);
		}

		if (SSL_CTX_use_PrivateKey_file(ctx, "client-key.pem", SSL_FILETYPE_PEM) <= 0 ) {
			ERR_print_errors_fp(stderr);
			exit(EXIT_FAILURE);
		}

		return ctx;
	}

}
}
