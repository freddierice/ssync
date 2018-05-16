#ifndef __CLIENT_H__
#define __CLIENT_H__
#include "util/exception.h"
#include "net/SSH.h"
#include <openssl/ssl.h>

namespace ssync {
namespace net {
	SSYNC_EXCEPTION(util::SSyncException, ClientException);
	class Client {
	public:
		class Config {
		public:
			constexpr static const int DEFAULT_PORT = 3130;
			constexpr static auto DEFAULT_HOST = "127.0.0.1";
			constexpr static auto DEFAULT_CERT = "client.pem";
			constexpr static auto DEFAULT_KEY = "client-key.pem";
		public:
			Config() : m_port(DEFAULT_PORT), m_host(DEFAULT_HOST),
		   		m_cert(DEFAULT_CERT), m_key(DEFAULT_KEY) {}
		protected:
			friend Client;
			int m_port;
			std::string m_host;
			std::string m_cert;
			std::string m_key;
		};
	public:
		Client();
		Client(const Config& config);
		~Client();

		static void init_openssl();
		static void cleanup_openssl();
		static SSL_CTX* create_context();
	private:
		Config m_config;
		int m_fd;
		SSL_CTX *m_ctx;
		SSL *m_ssl;
	};
}
}

#endif /* __CLIENT_H__ */
