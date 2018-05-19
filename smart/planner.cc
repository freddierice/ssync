#include "smart/planner.h"

namespace ssync {
namespace smart {
	Planner::Planner(std::list<std::shared_ptr<fs::RFile>> files) : m_files(files),
		m_config(), m_cv(), m_mutex(), m_plan_taken(0), m_next_plan(nullptr) {
		// put the smallest files in the front
		m_files.sort([](const std::shared_ptr<fs::RFile>& a, 
					const std::shared_ptr<fs::RFile>& b) {
				return a->size() < b->size();
		});

		// create a plan where we send the least number of small files that fits the
		// min plan quota.
		std::vector<std::shared_ptr<fs::RFile>> small_files;
		auto file_iter = m_files.begin();
		int total;
		for (; file_iter != m_files.end() && total < m_config.m_min_plan_size; 
				file_iter++) {
			small_files.push_back(*file_iter);
			total += (*file_iter)->size();
		}
		m_files.erase(m_files.begin(), file_iter);
		// auto plan = new PlanRaw(small_files);
		m_next_plan = std::make_unique<PlanRaw>(small_files, 50);
	}

	bool Planner::empty() {
		return m_files.size() == 0;
	}

	void Planner::next(std::unique_ptr<Plan>& plan) {
		if (empty())
			throw PlannerException("plan is empty");
		plan = std::move(m_next_plan);
	}
}
}
