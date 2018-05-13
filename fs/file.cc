#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fs/file.h"

namespace ssync {
namespace fs {
	File::File(const std::string& path, int id) : m_path(path), m_stat(),
		m_id(id) {
		if (stat(path.c_str(), &m_stat))
			throw FileException("could not get stats on file");
	}
	
	File::File(const std::string& path, int id, int size) : m_path(path), m_stat(),
		m_id(id) {
		int fd, ret;
		char zero = '\0';

		if ((fd = ::open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC)) == -1)
			throw FileException("could not create file");

		if (!size) {
			::close(fd);
			return;
		}

		if ((ret = ::lseek(fd, size-1, SEEK_SET)) == -1)
			throw FileException("could not truncate file");

		for (;;) {
			ret = ::write(fd, &zero, 1);
			if (ret == 1)
				break;
			if (ret == -1 && errno == EINTR)
				continue;
			throw FileException("could not write to end of file");
		}

		// XXX: this could be racey. 
		if ((ret = fstat(fd, &m_stat)) == -1)
			throw FileException("could not get file stats");

		::close(fd);
	}
	
	std::list<std::shared_ptr<File>> File::create_files(std::vector<std::string>& file_list, std::vector<int>& size_list) {
		std::list<std::shared_ptr<File>> ret;

		int id = 0;
		auto size_iter = size_list.begin();
		for (const auto& str: file_list)
			ret.push_back(std::make_shared<File>(str, id++, *size_iter++));
		
		return ret;
	}

	std::list<std::shared_ptr<File>> File::get_files(
			std::vector<std::string>& file_list) {
		std::list<std::shared_ptr<File>> ret;

		int id = 0;
		for (const auto& str : file_list)
			ret.push_back(std::make_shared<File>(str, id++));

		return ret;
	}
}
}
