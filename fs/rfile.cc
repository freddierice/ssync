#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fs/rfile.h"

namespace ssync {
namespace fs {
	RFile::RFile(const std::string& path, int id) : m_path(path), m_stat(),
		m_id(id), m_chunks(0), m_fd(
				// open file
				[path](){
					int fd;
					if ((fd = ::open(path.c_str(), O_RDONLY)) == -1) {
						throw std::runtime_error("could not open file");
					}
					return fd;
				}, 
				// close a file
				[](int fd){
					::close(fd);
				}) {
		if (stat(path.c_str(), &m_stat))
			throw FileException("could not get stats on file");
		m_chunks = chunks_total();
	}

	int RFile::size() {
		return m_stat.st_size;
	}
	
	std::list<std::shared_ptr<RFile>> get_files(
			std::vector<std::string>& file_list) {
		std::list<std::shared_ptr<RFile>> ret;

		int id = 0;
		for (const auto& str : file_list)
			ret.push_back(std::make_shared<RFile>(str, id++));

		return ret;
	}
}
}
