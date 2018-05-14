#ifndef __STATS_H__
#define __STATS_H__

#include "smart/planner.h"

namespace ssync {
namespace smart {
	class Stats {
	public:
		Stats();

	protected:
		friend smart::Planner;
		int m_sent;
		int m_size;
	};
}
}

#endif /* __STATS_H__ */
