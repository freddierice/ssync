#include "log/log.h"

namespace ssync {
namespace log {
	std::shared_ptr<spdlog::logger> console(spdlog::stdout_color_mt("console"));
}
}
