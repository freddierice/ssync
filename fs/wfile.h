#ifndef __WFILE_H__
#define __WFILE_H__

#include <atomic>
#include <experimental/filesystem>
namespace filesystem = std::experimental::filesystem;
#include <memory>
#include <list>
#include <vector>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <errno.h>

#include "util/exception.h"
#include "net/connection.h"
#include "fs/fd.h"
#include "fs/file.h"
#include "fs/io.h"

// forward definition of stat
struct stat;

namespace ssync {
namespace fs {
	
	class WFile {
	public:
		// Reader operations are atomic, start and end bounds cannot change.
		class FDWriter : public Writer {
		public:
			FDWriter(const FD& fd, int start, int end) : m_fd(fd),
				m_start(start), m_end(end) {}

			virtual void write_from(std::shared_ptr<net::Connection> conn) {
				int fd = conn->rfd();
				
				// another reader could have moved fd, lets seek to start
				if (lseek(m_fd.fd(), m_start, SEEK_SET) == -1)
					throw FileException("could not seek file");

				int total = m_end - m_start;
				while (total) {
					int ret;
					if ((ret = ::splice(fd, NULL, m_fd.fd(), NULL, total,
									SPLICE_F_MOVE)) == -1) {
						if (errno == EAGAIN || errno == EWOULDBLOCK
								|| errno == EINTR) // keep going on fake news
							continue;
						throw FileException("could not send file");
					}
					total -= ret;
				}
			}

			~FDWriter() {
				// if (m_start != m_end) 
				// 	throw FileException("FDReader out of scope before reading.");
			}
		private:
			FDWriter() = delete;
			FDWriter(const FDWriter& other) = delete;
			FDWriter& operator=(const FDWriter& other) = delete;
			FDWriter(const FDWriter&&) = delete;
			FDWriter(FDWriter&&) = delete;

			FD m_fd;
			const int m_start, m_end;
		};
	public:
		// opens a file
		WFile(const std::string& path, int id, int size);
		
		std::unique_ptr<FDWriter> take(unsigned int chunks) {
			int start, end;
			if (chunks > m_chunks)
				throw FileException("requested more chunks than available");
			
			start = (chunks_total() - m_chunks)*4096;
			end = start + chunks*4096;
			end = end > size() ? size() : end;

			m_chunks -= chunks;
			return std::make_unique<FDWriter>(m_fd, start, end);
		}

		int id() { return m_id; }
		int size() { return m_stat.st_size; }
		uint64_t chunks_total() { return (m_stat.st_size + 4096 - 1) / 4096; }
		bool done() { return !m_chunks; }
		const std::string path() { return m_path.string(); }
		const std::string name() { return m_path.filename(); }
	private:
		filesystem::path m_path;
		struct stat m_stat;
		int m_id;
		uint64_t m_chunks;
		FD m_fd;
	};
		
	std::list<std::shared_ptr<WFile>> create_files(
			std::vector<std::string>& file_list, std::vector<int>& size_list);
}
}

#endif /* __WFILE_H__ */
