#ifndef __CLIENT_H__
#define __CLIENT_H__
#include "util/exception.h"
#include "net/SSH.h"

namespace ssync {
namespace net {
	SSYNC_EXCEPTION(util::SSyncException, ClientException);
	void create_proxy(int& read_fd, int& write_fd);
	void run_proxy(int read_fd, int write_fd, std::shared_ptr<SSH::SSHChannel> chan);
}
}

#endif /* __CLIENT_H__ */
