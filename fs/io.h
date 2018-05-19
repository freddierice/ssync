#ifndef __IO_H__
#define __IO_H__
#include <memory>

#include "net/connection.h"

namespace ssync {
namespace fs {
	class Writer {
	public:
		virtual void write_from(std::shared_ptr<net::Connection> conn) = 0;
	};

	class Reader {
	public:
		virtual void read_to(std::shared_ptr<net::Connection> conn) = 0;
	};
}
}

#endif /* __IO_H__ */
