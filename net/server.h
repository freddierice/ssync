#ifndef __SERVER_H__
#define __SERVER_H__

#include <chrono>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>

#include "util/exception.h"
#include "util/io.h"
#include "net/connection.h"

#include <openssl/ssl.h>
#include <libssh2.h>

// /usr/include/netinet/tcp.h
namespace ssync {
namespace net {
	
	SSYNC_EXCEPTION(util::SSyncException, ServerException);
	SSYNC_EXCEPTION(ServerException, ServerTimeoutException);
	class Server {
	public:
		class Config {
		public:
			constexpr static const int DEFAULT_PORT = 3130;
			constexpr static auto DEFAULT_HOST = "127.0.0.1";
			constexpr static const int DEFAULT_TIMEOUT_SEC = 1;
			constexpr static const int DEFAULT_TIMEOUT_USEC = 0;
			constexpr static const int DEFAULT_MAX_CONNECTIONS = 5;
			constexpr static auto DEFAULT_CA = "ca.pem";
			constexpr static auto DEFAULT_CERT = "server.pem";
			constexpr static auto DEFAULT_KEY = "server-key.pem";
		public:
			Config() : m_port(DEFAULT_PORT), m_host(DEFAULT_HOST),
				m_timeout(), m_max_connections(DEFAULT_MAX_CONNECTIONS),
		   		m_ca(DEFAULT_CA), m_cert(DEFAULT_CERT), m_key(DEFAULT_KEY) {
				m_timeout.tv_sec = DEFAULT_TIMEOUT_SEC;
				m_timeout.tv_usec = DEFAULT_TIMEOUT_USEC;
			}
			void set_port(int port) { m_port = port; }
			void set_host(const std::string& host)  { m_host = host; }
			void set_max_connections(int m) { m_max_connections = m; }
			void set_ca(const std::string& ca) { m_ca = ca; }
			void set_cert(const std::string& cert) { m_cert = cert; }
			void set_key(const std::string& key) { m_key = key; }

			template <class Rep, class Period = std::ratio<1>>
			void set_timeout(std::chrono::duration<Rep, Period> duration) {
				using namespace std::chrono;
				m_timeout.tv_sec = duration_cast<seconds>(duration).count();
				duration -= seconds(m_timeout.tv_sec);
				m_timeout.tv_usec = duration_cast<microseconds>(duration).count();
			}

		protected:
			friend Server;
			int m_port;
			std::string m_host;
			struct timeval m_timeout;
			int m_max_connections;
			std::string m_ca, m_cert, m_key;
		};

	public:
		Server();
		Server(const Config& config);
		~Server();

		std::shared_ptr<net::Connection> accept();

		void create_context();

	private:

		Config m_config;
		int m_fd;
		SSL_CTX *m_ctx;
	};
}
}

#endif /* __SERVER_H__ */
