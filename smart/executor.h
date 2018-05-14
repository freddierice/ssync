#ifndef __EXECUTOR_H__
#define __EXECUTOR_H__

#include "util/exception.h"
#include "smart/stats.h"
#include "smart/plan.h"

namespace ssync {
namespace smart {

	SSYNC_EXCEPTION(util::SSyncException, ExecutorException);

	class Executor {
	public:
		Executor(int fd);
		
		Stats execute(std::shared_ptr<Plan> plan);
	private:
		Executor() = delete;

		int m_fd;
	};
}
}

#endif /* __EXECUTOR_H__ */
