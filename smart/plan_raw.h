#ifndef __PLAN_RAW_H__
#define __PLAN_RAW_H__

#include "smart/plan.h"

namespace ssync {
namespace smart {
	class PlanRaw : public Plan {
	public:
		PlanRaw(std::vector<std::shared_ptr<fs::File>> files);
		virtual uint64_t id() const;
		virtual void send(int fd);
		virtual void recv(int fd);
	private:
		PlanRaw() = delete;
		std::vector<std::shared_ptr<fs::File>>::iterator m_current;
	};
}
}

#endif /* __PLAN_RAW_H__ */
