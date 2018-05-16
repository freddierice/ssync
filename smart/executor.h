#ifndef __EXECUTOR_H__
#define __EXECUTOR_H__

#include "util/exception.h"
#include "smart/stats.h"
#include "smart/plan.h"
#include "net/connection.h"

namespace ssync {
namespace smart {

	SSYNC_EXCEPTION(util::SSyncException, ExecutorException);

	class Executor {
	public:
		Executor(std::shared_ptr<net::Connection> conn);
		
		Stats execute(std::shared_ptr<Plan> plan);
	private:
		Executor() = delete;

		std::shared_ptr<net::Connection> m_conn;
	};
}
}

#endif /* __EXECUTOR_H__ */
