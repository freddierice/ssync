#include <linux/tcp.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
// #include <linux/tcp.h>

#include "net/client.h"
#include "util/io.h"
#include "log/log.h"


namespace ssync {
namespace net {
	void create_proxy(int& read, int& write) {
		int pipefds[2];
		if (::pipe(pipefds) == -1)
			throw ClientException("could not create pipe");
		read = pipefds[0];
		write = pipefds[1];
	}

	void run_proxy(int read_fd, int write_fd, std::shared_ptr<SSH::SSHChannel> chan) {
		char *buffer_pipe, *buffer_channel;
		struct tcp_info ti;
		int buffer_len;
		int ret;
		socklen_t len = sizeof(struct tcp_info);
		if (getsockopt(chan->socket(), IPPROTO_TCP, TCP_INFO, &ti, &len) == -1)
			throw ClientException("could not get tcpinfo");

		// proxy the ssh connection
		buffer_len = ti.tcpi_snd_mss > ti.tcpi_rcv_mss ? ti.tcpi_snd_mss : ti.tcpi_rcv_mss;
		buffer_pipe = new char(buffer_len);
		buffer_channel = new char(buffer_len);

		struct pollfd fds[2];
		fds[0].fd = read_fd;
		fds[0].events = POLLIN;
		fds[1].fd = chan->socket();
		fds[1].events = POLLIN;

		for (;;) {
			ret = poll(fds, 2, -1);
			if (ret == -1) {
				if (errno == EINTR)
					continue;
				throw ClientException("could not poll for file descriptors");
			}
			
			if (fds[0].revents & POLLIN) {
				log::console->info("read_fd");
				ret = ::read(read_fd, buffer_pipe, buffer_len);
				if (ret > 0)
					util::write_full(
							[&](const void *buffer, int size) {
									return chan->write(buffer, size);
								}, buffer_pipe, ret);
			}

			if (fds[1].revents & POLLIN) {
				log::console->info("chan_fd");
				ret = chan->read(buffer_channel, buffer_len);
				if (ret > 0)
					util::write_full(write_fd, buffer_channel, ret);
			}

			if (fds[0].revents & POLLHUP || fds[1].revents & POLLHUP)
				break;
		}

		delete buffer_pipe;
		delete buffer_channel;
	}
}
}
