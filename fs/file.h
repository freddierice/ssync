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
		
	class Reader;
	class Writer;

	class File {
	public:
		// opens a file
		File(const std::string& path, int id);

		// creates a file with size
		File(const std::string& path, int id, int size);
		int id() { return m_id; }
		int size() { return m_stat.st_size; }
		uint64_t chunks_done() { return m_chunks; }
		uint64_t chunks_total() { return (m_stat.st_size + 4096 - 1) / 4096; }
		bool done() { return chunks_done() == chunks_total(); }
		const std::string path() { return m_path.string(); }
		const std::string name() { return m_path.filename(); }
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
	protected:
		friend Reader;
		friend Writer;
		uint64_t m_chunks;
	};


	class Reader {
	public:
		Reader(std::shared_ptr<File> file) : m_file(file) {
			if ((m_fd = ::open(m_file->path().c_str(), O_RDONLY)) < 0)
				throw FileException("could not open file");	
			if (lseek(m_fd, file->chunks_done()*4096, SEEK_SET) == -1)
				throw FileException("could not seek file");
		}
		Reader(Reader&& other) : m_file(other.m_file), m_fd(other.m_fd) {
			other.m_fd = -1;
		}

		void read_to(int fd) {
			read_to(fd, m_file->chunks_total() - m_file->chunks_done());
		}

		void read_to(int fd, uint64_t chunks) {
			int ret;
			int total_left = m_file->size() - m_file->chunks_done()*4096;
			int total_requested = chunks*4096;
			int total = total_requested > total_left ? total_left : total_requested;
			while (total) {
				if ((ret = ::sendfile(fd, m_fd, NULL, total)) == -1) {
					if (errno == EAGAIN) 
						continue;
					throw FileException("could not send file");
				}
				total -= ret;
			}

			m_file->m_chunks += chunks;
		}
		~Reader() {
			if (m_fd >= 0)
				::close(m_fd);
		}
	private:
		Reader() = delete;
		Reader(const Reader& other) = delete;
		Reader& operator=(const Reader& other) = delete;

		std::shared_ptr<File> m_file;
		int m_fd;
	};

	class Writer {
	public:
		Writer(std::shared_ptr<File> file) : m_file(file) {
			if ((m_fd = ::open(m_file->path().c_str(), O_RDWR)) < 0)
				throw FileException("could not open file");
		}
		Writer(Writer&& other) : m_fd(other.m_fd), m_file(other.m_file) {
			other.m_fd = -1;
		}

		void write_from(int fd) {
			write_from(fd, m_file->chunks_total() - m_file->chunks_done());
		}

		void write_from(int fd, uint64_t chunks) {

			int pipe_write, pipe_read;
			auto s = session::current();

			pipe_read = s->m_pipefds[0];
			pipe_write = s->m_pipefds[1];

			int ret;
			int total_left = m_file->size() - m_file->chunks_done()*4096;
			int total_requested = chunks*4096;
			int total = total_requested > total_left ? total_left : total_requested;
			while (total) {
				ret = ::splice(fd, NULL, pipe_write, NULL, total, SPLICE_F_MOVE);
				if (ret == -1)
					throw FileException("could not splice from socket");
				::splice(pipe_read, NULL, m_fd, NULL, ret, SPLICE_F_MOVE);
				total -= ret;
			}
			
			m_file->m_chunks += chunks;
		}

		~Writer() {
			if (m_fd >= 0) 
				::close(m_fd);
		}
	protected:
		int m_fd;
		std::shared_ptr<File> m_file;
	private:
		Writer() = delete;
		Writer(const Reader& other) = delete;
		Writer& operator=(const Writer& other) = delete;
	};
}
}

#endif /* __FILE_H__ */
