#ifndef __CLIENT_H__
#define __CLIENT_H__
#include "util/exception.h"
#include <openssl/ssl.h>

#include "net/connection.h"

namespace ssync {
namespace net {
	SSYNC_EXCEPTION(util::SSyncException, ClientException);
	class Client {
	public:
		class Config {
		public:
			constexpr static const int DEFAULT_PORT = 3130;
			constexpr static auto DEFAULT_HOST = "127.0.0.1";
			constexpr static auto DEFAULT_CA   = "/data/ca.pem";
			constexpr static auto DEFAULT_CERT = "/data/client.pem";
			constexpr static auto DEFAULT_KEY = "/data/client-key.pem";

		public:
			Config() : m_port(DEFAULT_PORT), m_ca(DEFAULT_CA),
			m_cert(DEFAULT_CERT), m_host(DEFAULT_HOST),
		   		 m_key(DEFAULT_KEY) {}

			void set_port(int port) { m_port = port; }
			void set_host(const std::string& host) { m_host = host; }
			void set_ca(const std::string& ca) { m_ca = ca; }
			void set_cert(const std::string& cert) { m_cert = cert; }
			void set_key(const std::string& key) { m_key = key; }
		protected:
			friend Client;
			int m_port;
			std::string m_ca;
			std::string m_cert;
			std::string m_host;
			std::string m_key;
		};
	public:
		Client();
		Client(const Config& config);
		~Client();

		std::shared_ptr<net::Connection> conn();

	private:
		Config m_config;
		int m_fd, m_rfd, m_wfd;
		SSL_CTX *m_ctx;
		SSL *m_ssl;
		std::shared_ptr<net::Connection> m_conn;
		
		void create_context();
		
		static void init_openssl();
		static void cleanup_openssl();
	};
}
}

#endif /* __CLIENT_H__ */
