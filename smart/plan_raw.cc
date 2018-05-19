
#include "smart/plan_raw.h"

namespace ssync {
namespace smart {
	PlanRaw::PlanRaw(std::vector<std::shared_ptr<fs::RFile>> files, int chunks) : 
		m_readers(), m_writers() {
		// take the first allotted.
		int total = 0;
		for (auto f : files) {
			int to_take = f->chunks_total();
			to_take = to_take > total ? total : to_take;
			m_readers.push_back(std::move(f->take(to_take)));
		}
	}
	PlanRaw::PlanRaw(std::vector<std::shared_ptr<fs::WFile>> files) {}
	
	uint64_t PlanRaw::id() const {
		return static_cast<uint64_t>(PlanId::Raw);
	}
	
	void PlanRaw::send(std::shared_ptr<net::Connection> conn) {
		for (auto& reader : m_readers) {
			reader->read_to(conn);
		}
	}

	void PlanRaw::recv(std::shared_ptr<net::Connection> conn) {
		for (auto& writer : m_writers) {
			writer->write_from(conn);
		}
	}
}
}
