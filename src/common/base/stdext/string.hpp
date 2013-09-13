// Copyright (c) 2011, Tencent.com
// All rights reserved.

/// @file string.hpp
/// @brief
/// @date  03/31/2011 11:11:48 PM
/// @author CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASE_STDEXT_STRING_HPP
#define COMMON_BASE_STDEXT_STRING_HPP

#include <string>

// Return a mutable char* pointing to a string's internal buffer,
// which may not be null-terminated. Writing through this pointer will
// modify the string.
//
// string_as_array(&str)[i] is valid for 0 <= i < str.size() until the
// next call to a string method that invalidates iterators.
//
// As of 2006-04, there is no standard-blessed way of getting a
// mutable reference to a string's internal buffer. However, issue 530
// (http://www.open-std.org/JTC1/SC22/WG21/docs/lwg-active.html#530)
// proposes this as the method. According to Matt Austern, this should
// already work on all current implementations.
inline char* string_as_array(std::string* str)
{
    // DO NOT USE const_cast<char*>(str->data())! See the unittest for why.
    return str->empty() ? NULL : &*str->begin();
}

#endif // COMMON_BASE_STDEXT_STRING_HPP
