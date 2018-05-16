#ifndef __CLIENT_H__
#define __CLIENT_H__
#include "util/exception.h"
#include "net/SSH.h"
#include <openssl/ssl.h>

#include "util/proto.h"

namespace ssync {
namespace net {
	SSYNC_EXCEPTION(util::SSyncException, ClientException);
	class Client {
	public:
		class Config {
		public:
			constexpr static const int DEFAULT_PORT = 3130;
			constexpr static auto DEFAULT_HOST = "127.0.0.1";
			constexpr static auto DEFAULT_CA   = "ca.pem";
			constexpr static auto DEFAULT_CERT = "client.pem";
			constexpr static auto DEFAULT_KEY = "client-key.pem";

		public:
			Config() : m_port(DEFAULT_PORT), m_ca(DEFAULT_CA),
			m_cert(DEFAULT_CERT), m_host(DEFAULT_HOST),
		   		 m_key(DEFAULT_KEY) {}
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

		std::shared_ptr<util::Proto> get_proto();

	private:
		Config m_config;
		int m_fd, m_rfd, m_wfd;
		SSL_CTX *m_ctx;
		SSL *m_ssl;
		BIO *m_bio;
		std::shared_ptr<util::Proto> m_proto;
		
		void create_context();
		
		static void init_openssl();
		static void cleanup_openssl();
	};
}
}

#endif /* __CLIENT_H__ */
