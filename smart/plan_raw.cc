
#include "smart/plan_raw.h"

namespace ssync {
namespace smart {
	PlanRaw::PlanRaw(std::vector<std::shared_ptr<fs::File>> files) : Plan(files), 
		m_current(m_files.begin()) {
	}
	
	uint64_t PlanRaw::id() const {
		return static_cast<uint64_t>(PlanId::Raw);
	}
	
	void PlanRaw::send(int fd) {
		for (auto& file : m_files) {
			auto reader = fs::Reader(file);
			reader.read_to(fd);
		}
	}

	void PlanRaw::recv(int fd) {
		for (auto& file : m_files) {
			auto writer = fs::Writer(file);
			writer.write_from(fd);
		}
	}
}
}
