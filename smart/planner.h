#ifndef __PLANNER_H__
#define __PLANNER_H__

#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>

#include "fs/rfile.h"
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
		Planner(std::list<std::shared_ptr<fs::RFile>> files);

		bool empty();
		void next(std::unique_ptr<Plan>&);
	private:
		Planner() = delete;

		std::list<std::shared_ptr<fs::RFile>> m_files;
		Config m_config;

		std::condition_variable m_cv;
		std::mutex m_mutex;
		int m_plan_taken;
		std::unique_ptr<Plan> m_next_plan;
	};
}
}

#endif /*__PLANNER_H__ */
