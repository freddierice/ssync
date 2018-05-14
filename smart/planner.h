#ifndef __PLANNER_H__
#define __PLANNER_H__

#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>

#include "fs/file.h"
#include "util/exception.h"
#include "smart/plan.h"
#include "smart/plan_raw.h"

namespace ssync {
namespace smart {

	SSYNC_EXCEPTION(util::SSyncException, PlannerException);
	class Planner {
	public:
		class Config {
		public:
			constexpr static int DEFAULT_MIN_PLAN_SIZE = 1024*1024*50;
		public:
			Config() : m_min_plan_size(DEFAULT_MIN_PLAN_SIZE) {};
			int m_min_plan_size;
		};
	public:
		Planner(std::list<std::shared_ptr<fs::File>> files);

		bool empty();
		std::shared_ptr<Plan> next();
	private:
		Planner() = delete;

		std::list<std::shared_ptr<fs::File>> m_files;
		Config m_config;

		std::condition_variable m_cv;
		std::mutex m_mutex;
		int m_plan_taken;
		std::shared_ptr<Plan> m_next_plan;
	};
}
}

#endif /*__PLANNER_H__ */
