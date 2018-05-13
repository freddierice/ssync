#ifndef __SESSION_H__
#define __SESSION_H__

#include <memory>

#include <unistd.h>
#include <stdlib.h>

#include "util/exception.h"

namespace ssync {
namespace session {

	SSYNC_EXCEPTION(util::SSyncException, SessionException);
	class Session {
	public:
		Session() {
			if (pipe(m_pipefds) == -1)
				throw SessionException("could not create pipes");
		}
		~Session() {
			::close(m_pipefds[0]);
			::close(m_pipefds[1]);
		}
		// Session(std::thread thread) : m_thread(thread) {}
		// ~Session() {
		// 	m_thread.join();
		//}

		// std::chrono::system_clock::time_point m_last_interaction;
		int m_pipefds[2];
	private:
		// Session() = delete;
		
		// std::thread m_thread;
	};
	
	void create();
	std::shared_ptr<Session> current();
}
}

#endif /* __SESSION_H__ */
