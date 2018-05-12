#include <unistd.h>

#include "util/const.h"

namespace ssync {
namespace util {
	int page_size = static_cast<int>(sysconf(_SC_PAGESIZE));
}
}
