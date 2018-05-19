#ifndef __UTIL_IO_H__
#define __UTIL_IO_H__

#include <unistd.h>
#include <errno.h>

namespace ssync {
namespace util {
	/*
	class Reader {
	public:
		// reads into the buffer and returns the number of bytes written.
		virtual int read(void *buffer, int num_pages) = 0;
	};
	class Writer {
	public:
		// writes from the buffer and returns the number of bytes written.
		virtual int write(const void *buffer, int num_pages) = 0;
	};
	class ReaderWriter {
		public:
		// writes from the buffer and returns the number of bytes written.
		virtual int write(const void *buffer, int num_pages) = 0;

		// reads into the buffer and returns the number of bytes written.
		virtual int read(void *buffer, int num_pages) = 0;

		// checks to see if the file is still open 
		virtual bool open() = 0;
	};
*/
	// write_full is a utility function that can be inlined to write the
	// entire buffer.
	inline int write_full(int fd, const void* buffer, int len) {
		int ret, total = 0;
		do {
			if ((ret = write(fd, reinterpret_cast<const char *>(buffer)+total, 
							len - total)) < 0) {
				if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
					continue;
				return ret;
			}
			total += ret;
		} while (len != total);

		return total;
	}

	template <typename F>
	inline int write_full(F f, const void* buffer, int len) {
		int ret, total = 0;
		do {
			if ((ret = f(reinterpret_cast<const char *>(buffer)+total,
							len - total)) < 0) {
				if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
					continue;
				return ret;
			}
			total += ret;
		} while (len != total);

		return total;
	}
}
}

#endif /* __UTIL_IO_H__ */
