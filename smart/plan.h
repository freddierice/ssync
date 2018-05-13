#ifndef __PLAN_H__
#define __PLAN_H__
#include <queue>

#include "fs/file.h"

namespace ssync {
namespace smart {

	enum PlanId : uint64_t {
		Raw = 0,
		Zlib = 1
	};

	class Plan {
	public:
		Plan(std::vector<std::shared_ptr<fs::File>> files) : m_files(files) {}

		// id gives the plan id so the client knows to use the same plan
		// to write the files.
		virtual uint64_t id() const = 0;

		// send and recv entire plan.
		virtual void send(int fd) = 0;
		virtual void recv(int fd) = 0;
	protected:
		std::vector<std::shared_ptr<fs::File>> m_files;
	};

}
}

#endif /* __PLAN_H__ */
