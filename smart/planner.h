#ifndef __PLANNER_H__
#define __PLANNER_H__

#include <list>

#include "fs/file.h"

namespace ssync {
namespace smart {
	class Planner {
	public:
		Planner(std::list<std::shared_ptr<fs::File>> files);
	private:
		Planner() = delete;

		std::list<std::shared_ptr<fs::File>> m_files;
	};
}
}

#endif /*__PLANNER_H__ */
