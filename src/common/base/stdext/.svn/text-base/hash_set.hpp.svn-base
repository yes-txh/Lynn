#ifndef EXT_HASH_SET_HPP_INCLUDED
#define EXT_HASH_SET_HPP_INCLUDED

#ifdef __GNUC__

#ifdef __DEPRECATED
#undef __DEPRECATED
#define __DEPRECATED_DEFINED
#endif

#include <ext/hash_set>

#ifdef __DEPRECATED_DEFINED
#define __DEPRECATED
#undef __DEPRECATED_DEFINED
#endif

#include "common/base/stdext/hash_function.hpp"

namespace stdext
{
	using __gnu_cxx::hash_set;
	using __gnu_cxx::hash_multiset;
}

#elif defined _MSC_VER
#include <hash_set>
#endif

/// for compatiable
namespace ext = stdext;

#endif//EXT_HASH_SET_HPP_INCLUDED

