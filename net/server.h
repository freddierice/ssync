#ifndef __SERVER_H__
#define __SERVER_H__

#include <chrono>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>

#include "util/exception.h"
#include "util/io.h"

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
		public:
			Config() : m_port(DEFAULT_PORT), m_host(DEFAULT_HOST),
				m_timeout(), m_max_connections(DEFAULT_MAX_CONNECTIONS) {
				m_timeout.tv_sec = DEFAULT_TIMEOUT_SEC;
				m_timeout.tv_usec = DEFAULT_TIMEOUT_USEC;
			}
			void set_port(int port) {
				m_port = port;
			}
			void set_host(const std::string& host) {
				m_host = host;
			}
			void set_max_connections(int m) {
				m_max_connections = m;
			}

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
		};

	public:
		Server();
		Server(const Config& config);
		~Server();

		int accept();

	private:

		Config m_config;
		int m_socket;
	};
}
}

#endif /* __SERVER_H__ */
