#include "smart/executor.h"

namespace ssync {
namespace smart {
	Executor::Executor(std::shared_ptr<net::Connection> conn) : m_conn(conn) {}

	Stats Executor::execute(std::shared_ptr<Plan> plan) {
		return Stats();
	}
}
}
