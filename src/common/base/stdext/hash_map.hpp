#ifdef __GNUC__

#ifdef __DEPRECATED
#undef __DEPRECATED
#define __DEPRECATED_DEFINED
#endif

#include <ext/hash_map>

#ifdef __DEPRECATED_DEFINED
#define __DEPRECATED
#undef __DEPRECATED_DEFINED
#endif

#include "common/base/stdext/hash_function.hpp"

namespace stdext
{
	using __gnu_cxx::hash_map;
	using __gnu_cxx::hash_multimap;
}

#elif defined _MSC_VER

#include <hash_map>

#endif

namespace ext = stdext;

