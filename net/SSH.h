#ifndef __SSH_H__
#define __SSH_H__

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
	SSYNC_EXCEPTION(util::SSyncException, SSHException);

	class SSH {
	public:
		SSH(const std::string& username, const std::string& ip, int port = 22);
		~SSH();
		
		class SSHChannel : public util::ReaderWriter {
			public:
				SSHChannel(int socket, LIBSSH2_SESSION *session, const std::string& cmd);
				SSHChannel(int socket, LIBSSH2_SESSION *session, const std::string& host,
						int port);
				SSHChannel(SSHChannel &&);
				~SSHChannel();
				virtual int read(void *buffer, int len);
				virtual int write(const void *buffer, int len);
				virtual bool open();
				int socket();
			protected:
				LIBSSH2_CHANNEL *m_channel;
			private:
				SSHChannel() = delete;
				SSHChannel(const SSHChannel&) = delete;
				SSHChannel& operator=(const SSHChannel&) = delete;
				int m_socket;

		};
		friend SSHChannel;

		std::shared_ptr<SSHChannel> run(const std::string& cmd);
		std::shared_ptr<SSHChannel> conn(const std::string& host, int port);

	private:
		SSH() = delete;

		const std::string m_username;
		const std::string m_ip;
		int m_port;

		int m_socket;
		LIBSSH2_SESSION *m_session;
	};
}
}

#endif /* __SSH_H__ */
