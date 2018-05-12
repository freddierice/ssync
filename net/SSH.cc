
#include <cstring>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#include "SSH.h"

namespace ssync {
namespace net {

	// TODO: docs
	SSH::SSH(const std::string& username, const std::string& ip, int port) : m_username(username), m_ip(ip), m_port(port), m_socket(0), m_session(0) {
		struct addrinfo *res;
		int ret;
		struct addrinfo hints;
		// struct sockaddr_in sin;

		// m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_socket == -1)
		// 	throw SSHException("could not create socket");
		std::memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = 0;
		hints.ai_protocol = IPPROTO_TCP;
		res = 0;
		if ((ret = ::getaddrinfo(ip.c_str(), (const char *)0, static_cast<const struct addrinfo *>(&hints), &res)) == -1)
			throw SSHException("could not getaddrinfo");

		m_socket = -1;
		for (auto rp = res; rp != NULL; rp = rp->ai_next) {
			m_socket = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
			if (m_socket == -1)
				continue;
			
			((struct sockaddr_in *)rp->ai_addr)->sin_port = htons(m_port);
			if (!::connect(m_socket, rp->ai_addr, rp->ai_addrlen))
				break;
		}
		freeaddrinfo(res);

		if (m_socket == -1)
			throw SSHException("could not create socket");

		LIBSSH2_AGENT *agent = NULL;
		struct libssh2_agent_publickey *identity, *prev_identity = NULL;

		char *userauthlist;
		// const char *fingerprint;

		if (libssh2_init (0))
			throw SSHException("could not initialize libssh2");

		if (!(m_session = libssh2_session_init()))
			throw SSHException("libssh2 could not initialize session");

		// all non-blocking
		libssh2_session_set_blocking(m_session, 0);

		if (libssh2_session_handshake(m_session, m_socket))
			throw SSHException("libssh2 handshake failed");

		// XXX: add fingerprint checking
		// fingerprint = ..
		libssh2_hostkey_hash(m_session, LIBSSH2_HOSTKEY_HASH_SHA1);
		userauthlist = libssh2_userauth_list(m_session, username.c_str(), username.length());
		if (strstr(userauthlist, "publickey") == NULL)
			throw SSHException("publickey authorization not allowed on server");

		agent = libssh2_agent_init(m_session);
		if (!agent)
			throw SSHException("could not initialize the ssh agent");

		if (libssh2_agent_connect(agent))
			throw SSHException("could not connect to the ssh-agent");

		if (libssh2_agent_list_identities(agent))
			throw SSHException("could not get a list of identities from the ssh agent");

		for (;;) {
			ret = libssh2_agent_get_identity(agent, &identity, prev_identity);
			if (ret == 1)
				throw SSHException("no functioning ssh keys");
			if (ret < 0)
				throw SSHException("could not obtain identity from ssh-agent");
			if (!libssh2_agent_userauth(agent, username.c_str(), identity))
				break;
			prev_identity = identity;
		}
	}
	
	std::shared_ptr<SSH::SSHChannel> SSH::run(const std::string &cmd) {
		return std::make_shared<SSHChannel>(m_session, cmd);
	}

	std::shared_ptr<SSH::SSHChannel> SSH::conn(const std::string &host, int port) {
		return std::make_shared<SSHChannel>(m_session, host, port);
	}

	SSH::~SSH() {
		if (m_session) {
			libssh2_session_disconnect(m_session, "Disconnect Reason");
			libssh2_session_free(m_session);
		}
		::close(m_socket);
	}

	SSH::SSHChannel::SSHChannel(LIBSSH2_SESSION *session, const std::string& cmd) {
		int ret;

		while ((m_channel = libssh2_channel_open_session(session)) == NULL &&
				libssh2_session_last_error(session, NULL, NULL, 0) == LIBSSH2_ERROR_EAGAIN ){}
		if (!m_channel)
			throw SSHException("could not open channel");

		while ((ret = libssh2_channel_exec(m_channel, cmd.c_str())) == LIBSSH2_ERROR_EAGAIN) {}
		if (ret)
			throw SSHException("could not execute command");

		while((ret = libssh2_channel_send_eof(m_channel)) == LIBSSH2_ERROR_EAGAIN){}
		if (ret)
			throw SSHException("could not send eof");
	}

	SSH::SSHChannel::SSHChannel(LIBSSH2_SESSION *session, const std::string& host, int port) {
		if (!(m_channel = libssh2_channel_direct_tcpip(session, host.c_str(), port)))
			throw SSHException("could not open tcpip connection");
	}

	int SSH::SSHChannel::read(void *buffer, int len) {
		int ret;

		do {
			ret = ::libssh2_channel_read(m_channel, static_cast<char *>(buffer), len);
		} while (ret == LIBSSH2_ERROR_EAGAIN);

		if (ret < 0)
			throw SSHException("connection dead");

		return ret;
	}

	int SSH::SSHChannel::write(const void *buffer, int len) {
		int ret;

		do {
			ret = ::libssh2_channel_write(m_channel, static_cast<const char *>(buffer), len);
		} while (ret == LIBSSH2_ERROR_EAGAIN);

		if (ret < 0)
			throw SSHException("connection dead");

		return ret;
	}

	bool SSH::SSHChannel::open() {
		int ret = libssh2_channel_eof(m_channel);
		if (ret < 0)
			throw SSHException("libssh2_channel_eof returned a negative number");
		return !ret;
	}

	SSH::SSHChannel::~SSHChannel() {
		if (m_channel) {
			int ret;
			while( (ret = libssh2_channel_close(m_channel)) == LIBSSH2_ERROR_EAGAIN ){}
			libssh2_channel_free(m_channel);
			m_channel = nullptr;
		}
	}
}
}