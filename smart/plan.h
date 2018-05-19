#ifndef __PLAN_H__
#define __PLAN_H__
#include <queue>

#include "net/connection.h"
#include "fs/rfile.h"
#include "fs/wfile.h"

namespace ssync {
namespace smart {

	enum PlanId : uint64_t {
		Raw = 0,
		Zlib = 1
	};

	class Plan {
	public:
		// Plan(std::vector<std::shared_ptr<fs::RFile>> files) {}
		// Plan(std::vector<std::shared_ptr<fs::WFile>> files) {}

		// id gives the plan id so the client knows to use the same plan
		// to write the files.
		virtual uint64_t id() const = 0;

		// send and recv entire plan.
		virtual void send(std::shared_ptr<net::Connection> conn) = 0;
		virtual void recv(std::shared_ptr<net::Connection> conn) = 0;
	protected:
	private:
		// Plan() = delete;
	};

}
}

#endif /* __PLAN_H__ */
