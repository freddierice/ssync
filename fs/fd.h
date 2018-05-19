#ifndef __FD_H__
#define __FD_H__
#include <atomic>
#include <memory>
#include <functional>

#include <unistd.h>

namespace ssync {
namespace fs {

	// TODO: templatize it for inlining optimization
	// template <typename FOpen, typename FClose>
	
	typedef std::function<int(void)> FOpen;
	typedef std::function<void(int)> FClose;

	// This class has weird semantics, so bear with me.
	// Your job as a Copier or Destructor depends on the refcount before you
	// changed it. If you are the first, you have no job, just initialize
	// yourself to have a reference count of 0 and an fd of -1 (invalid).
	class FD {
	public:
		// the file is only open when it is copied. initialize the refcount
		// to 0 and the file descriptor to invalid. 
		FD(FOpen fopen, FClose fclose) : m_fopen(fopen), m_fclose(fclose),
			m_fd(std::make_shared<std::atomic<int32_t>>(-1)),
			m_rc(std::make_shared<std::atomic<int32_t>>(0)) {}

		// COPY CONSTRUCTORS //
		// during a copy, we need to increase some reference counts and
		// make sure that the fd is always correct.
		FD(const FD& other) : m_fopen(other.m_fopen), m_fclose(other.m_fclose),
			m_fd(other.m_fd), m_rc(other.m_rc) {

			auto previous = m_rc->fetch_add(1);
	
			// 1. the file is getting closed or is closed. open a file
			// and replace the invalid fd.
			if (previous == 0) {
				auto new_fd = m_fopen();
				int32_t fd_invalid = -1;
				while (!m_fd->compare_exchange_strong(fd_invalid,
							new_fd)) {}
				return;
			}

			// 2. I got confirmation that the previous is at least 1, so
			// any destructor running now got confirmation of not being 
			// the last destructor.
		}
		
		// MOVE CONSTRUCTORS //
		// during a move, we need to do what a copy would do.
		FD(const FD&& other) : FD(other) {}
		
		// DESTRUCTOR //
		~FD() {
			auto previous = m_rc->fetch_sub(1);

			// 1. we are the last line of defense! close up shop. Given the
			// structure of our program, it is impossible to copy an FD object
			// while it is getting destroyed because it is only destroyed after
			// it has left scope.
			if (previous == 0) {
				return;
			}

			// 2. we are the penultimate reference. Close the file
			// and let the last object deal with freeing resources.
			if (previous == 1) {
				// if another object has increased the reference count, it is
				// waiting for us to invalidate its value so it can store the
				// new opened file descriptor.
				// XXX: this is probably not the most efficient version, try to
				// use the correct load/store gates.
				auto close_fd = m_rc->load();
				m_fd->store(-1);
				m_fclose(close_fd);
				return;
			}

			// 3. plenty of other FDs around to take care of things for me.
		}


		// by class definition, the file descriptor cannot change underneath me.
		int fd() { return static_cast<int>(*m_fd); }

	private:
		FD() = delete;
		FD(FD&) = delete; // compiler doesn't need suboptimal version.
		FD(FD&&) = delete; // compiler doesn't need suboptimal version.

		FOpen m_fopen;
		FClose m_fclose;
		const std::shared_ptr<std::atomic<int32_t>> m_fd;
		const std::shared_ptr<std::atomic<int32_t>> m_rc;
	};
}
}

#endif /* __FD_H__ */
