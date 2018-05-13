#ifndef __STATS_H__
#define __STATS_H__

namespace ssync {
namespace smart {
	class Stats {
	public:
		Stats();

	protected:
		friend smart::Planner;
		int m_sent;
		int m_size;
		int 
	};
}
}

#endif /* __STATS_H__ */
