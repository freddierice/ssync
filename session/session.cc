#include "session/session.h"

#include <thread>
#include <unordered_map>
#include <shared_mutex>

namespace ssync {
namespace session {

	std::shared_mutex mutex;
	// std::unordered_map<std::string, std::thread::id> ip_map;
	std::unordered_map<std::thread::id, std::shared_ptr<Session>> thread_map;

	void create() {
		std::unique_lock<std::shared_mutex> lock(mutex);
		thread_map.insert_or_assign(std::this_thread::get_id(),
				std::make_shared<Session>());
	}

	std::shared_ptr<Session> current() {
		std::shared_lock<std::shared_mutex> lock(mutex);
		return thread_map[std::this_thread::get_id()];
	}
	
}
}
