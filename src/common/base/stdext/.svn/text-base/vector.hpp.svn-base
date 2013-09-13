// Copyright (c) 2011, Tencent.com
// All rights reserved.

/// @file vector.hpp
/// @brief vector extensions
/// @date  03/31/2011 11:16:46 PM
/// @author CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASE_STDEXT_VECTOR_HPP
#define COMMON_BASE_STDEXT_VECTOR_HPP

#include <vector>

// To treat a possibly-empty vector as an array, use these functions.
// If you know the array will never be empty, you can use &*v.begin()
// directly, but that is allowed to dump core if v is empty.  This
// function is the most efficient code that will work, taking into
// account how our STL is actually implemented.  THIS IS NON-PORTABLE
// CODE, so call us instead of repeating the nonportable code
// everywhere.  If our STL implementation changes, we will need to
// change this as well.

template<typename T>
inline T* vector_as_array(std::vector<T>* v) {
# ifdef NDEBUG
    return &*v->begin();
# else
    return v->empty() ? NULL : &*v->begin();
# endif
}

template<typename T>
inline const T* vector_as_array(const std::vector<T>* v) {
# ifdef NDEBUG
    return &*v->begin();
# else
    return v->empty() ? NULL : &*v->begin();
# endif
}

#endif // COMMON_BASE_STDEXT_VECTOR_HPP
