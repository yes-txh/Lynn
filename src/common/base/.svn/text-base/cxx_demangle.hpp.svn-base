// Copyright (c) 2009, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASE_CXX_DEMANGLE_HPP
#define COMMON_BASE_CXX_DEMANGLE_HPP

/// @file CxxDemangle.hpp
/// @author phongchen
/// brief do C++ demangling
/// convert mangled name to human readable form
/// support gcc/MSVC C++ ABI
///
/// example:
/// _ZNKSt13basic_fstreamIcSt11char_traitsIcEE7is_openEv
/// ->
/// std::basic_fstream<char, std::char_traits<char> >::is_open() const

#include <string>
#include <typeinfo>

#ifdef _MSC_VER

#include "common/base/common_windows.h"
#include <dbghelp.h>

inline std::string CxxDemangle(const char* name)
{
    char buffer[4096];
    DWORD length = UnDecorateSymbolName(name, buffer, sizeof(buffer), 0);
    if (length > 0)
        return std::string(buffer, length);
    else
        return name;
}

#pragma comment(lib, "DbgHelp")

#elif defined __GNUC__

#include <cxxabi.h>

inline std::string CxxDemangle(const char* name)
{
    char buffer[4096];
    size_t size = sizeof(buffer);
    int status;
    if (abi::__cxa_demangle(name, buffer, &size, &status))
        return std::string(buffer);
    else
        return name;
}
#endif

#endif // COMMON_BASE_CXX_DEMANGLE_HPP

