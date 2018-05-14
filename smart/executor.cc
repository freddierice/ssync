#include "smart/executor.h"

namespace ssync {
namespace smart {
	Executor::Executor(int fd) : m_fd(fd) {}

	Stats Executor::execute(std::shared_ptr<Plan> plan) {
		return Stats();
	}
}
}
