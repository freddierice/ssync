#ifndef __PLAN_RAW_H__
#define __PLAN_RAW_H__

#include "smart/plan.h"
#include "fs/rfile.h"
#include "fs/wfile.h"
#include "fs/io.h"

namespace ssync {
namespace smart {
	class PlanRaw : public Plan {
	public:
		PlanRaw(std::vector<std::shared_ptr<fs::RFile>> files, int chunks);
		PlanRaw(std::vector<std::shared_ptr<fs::WFile>> files);

		virtual uint64_t id() const;
		virtual void send(std::shared_ptr<net::Connection> conn);
		virtual void recv(std::shared_ptr<net::Connection> conn);
	private:
		PlanRaw() = delete;

		std::vector<std::unique_ptr<fs::Reader>> m_readers;
		std::vector<std::unique_ptr<fs::Writer>> m_writers;
	};
}
}

#endif /* __PLAN_RAW_H__ */
