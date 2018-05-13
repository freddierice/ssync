#ifndef __FILE_H__
#define __FILE_H__

#include <filesystem>
namespace filesystem = std::filesystem;
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
#include "session/session.h"

// forward definition of stat
struct stat;

namespace ssync {
namespace fs {
	SSYNC_EXCEPTION(util::SSyncException, FileException);

	class File {
	public:
		class Reader {
		public:
			Reader(File& file) : Reader(file.path(), file.size()) {}
			Reader(const std::string& path, int size) : m_fd(-1), m_size(size) {
				if ((m_fd = ::open(path.c_str(), O_RDONLY)) < 0)
					throw FileException("could not open file");	
			}
			Reader(Reader&& other) : m_fd(other.m_fd), m_size(other.m_size) {
				other.m_fd = 0;
			}

			void read_to(int fd) {
				int ret;
				int total = m_size;
				while (total) {
					if ((ret = ::sendfile(fd, m_fd, NULL, m_size)) == -1) {
						if (errno == EAGAIN) 
							continue;
						throw FileException("could not send file");
					}
					total -= ret;
				}
			}
			~Reader() {
				if (m_fd >= 0)
					::close(m_fd);
			}
		protected:
			int m_fd;
			int m_size;
		private:
			Reader() = delete;
			Reader(const Reader& other) = delete;
			Reader& operator=(const Reader& other) = delete;
		};

		class Writer {
		public:
			Writer(File& file) : Writer(file.path(), file.size()) {}
			Writer(const std::string& path, int size) : m_fd(-1), m_size(size) {
				if ((m_fd = ::open(path.c_str(), O_RDWR)) < 0)
					throw FileException("could not open file");
			}
			Writer(Writer&& other) : m_fd(other.m_fd), m_size(other.m_size) {
				other.m_fd = -1;
			}

			void write_from(int fd) {
				int pipe_write, pipe_read;
				auto s = session::current();

				pipe_read = s->m_pipefds[0];
				pipe_write = s->m_pipefds[1];

				int total = m_size;
				int ret;
				while (total) {
					ret = ::splice(fd, NULL, pipe_write, NULL, total, SPLICE_F_MOVE);
					if (ret == -1)
						throw FileException("could not splice from socket");
					::splice(pipe_read, NULL, m_fd, NULL, ret, SPLICE_F_MOVE);
					total -= ret;
				}
			}

			~Writer() {
				if (m_fd >= 0) 
					::close(m_fd);
			}
		protected:
			int m_fd;
			int m_size;
		private:
			Writer() = delete;
			Writer(const Reader& other) = delete;
			Writer& operator=(const Writer& other) = delete;
		};
	public:
		// opens a file
		File(const std::string& path, int id);

		// creates a file with size
		File(const std::string& path, int id, int size);
		int id() { return m_id; }
		int size() { return m_stat.st_size; }
		const std::string path() { return m_path.string(); }
		const std::string name() { return m_path.filename(); }
		Reader reader() { return Reader(*this); }
		Writer writer() { return Writer(*this); }
	public:
		static std::list<std::shared_ptr<File>> 
			create_files(std::vector<std::string>& file_list,
					std::vector<int>& size_list);
		static std::list<std::shared_ptr<File>> 
			get_files(std::vector<std::string>& file_list);
	private:
		filesystem::path m_path;
		struct stat m_stat;
		int m_id;
	};
}
}

#endif /* __FILE_H__ */
