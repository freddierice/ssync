#ifndef __EXECUTOR_H__
#define __EXECUTOR_H__

#include "util/exception.h"

namespace ssync {
namespace smart {

	SSYNC_EXCEPTION(SSyncException, ExecutorException);

	class Executor {
	public:
		Executor(int fd);
		
		Stats execute(Plan &plan);
	private:
		Executor() = delete;
	};
}
}

#endif /* __EXECUTOR_H__ */
