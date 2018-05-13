#include "smart/planner.h"

namespace ssync {
namespace smart {
	Planner::Planner(std::list<std::shared_ptr<fs::File>> files) : m_files(files) {
		// put the smallest files in the front
		m_files.sort([](const std::shared_ptr<fs::File>& a, 
					const std::shared_ptr<fs::File>& b) {
				return a->size() < b->size();
		});

		// create a plan where we send the least number of small files that fits the
		// min plan quota.
	}
}
}
