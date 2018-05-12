#ifndef __EXCEPTION_H__ 
#define __EXCEPTION_H__

#include <stdexcept>

// a macro to make an exception subclass. 
#define SSYNC_EXCEPTION(parent, child) class child: public parent { \
public: \
	child(std::string const& msg) : parent(msg) {} \
}

namespace ssync {
namespace util {
	SSYNC_EXCEPTION(std::runtime_error, SSyncException);
}
}

#endif /* __EXCEPTION_H__ */
