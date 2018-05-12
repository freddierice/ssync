#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <spdlog/spdlog.h>
#include <memory>

namespace ssync {
namespace log {
	extern std::shared_ptr<spdlog::logger> console;
}
}

#endif
