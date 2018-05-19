#ifndef __FILE_H__
#define __FILE_H__

#include "util/exception.h"

namespace ssync {
namespace fs {
	SSYNC_EXCEPTION(util::SSyncException, FileException);
}
}

#endif /* __FILE_H__ */
