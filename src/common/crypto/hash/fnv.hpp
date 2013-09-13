/*
 * =====================================================================================
 *
 *       Filename:  FnvHash.hpp
 *
 *    Description:  FNV hash alogrithm
 *
 *        Version:  1.0
 *        Created:  2010年06月12日 15时53分11秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  phongchen
 *        Company:  Tencent
 *
 * =====================================================================================
 */

#ifndef CRYPTO_FNV_HASH_HPP_INCLUDED
#define CRYPTO_FNV_HASH_HPP_INCLUDED

#include <stddef.h>

namespace std
{
template <typename Char, typename Traits, typename Allocator> class basic_string;
};

struct FnvHasher32
{
	typedef unsigned int ResultType;
	ResultType operator()(const void* data, size_t size, ResultType seed) const;
	ResultType operator()(const char* str, ResultType seed) const;

	template <typename Char, typename Traits, typename Allocator>
	ResultType operator()(const std::basic_string<Char, Traits, Allocator>& str, ResultType seed) const
	{
		return this->operator()(str.data(), str.length(), seed);
	}
};

struct FnvHasher64
{
	typedef unsigned long long ResultType;
	ResultType operator()(const void* data, size_t size, ResultType seed) const;
	ResultType operator()(const char* str, ResultType seed) const;
	template <typename Char, typename Traits, typename Allocator>
	ResultType operator()(const std::basic_string<Char, Traits, Allocator>& str, ResultType seed) const
	{
		return this->operator()(str.data(), str.length(), seed);
	}
};

struct FnvaHasher32
{
	typedef unsigned int ResultType;
	ResultType operator()(const void* data, size_t size, ResultType seed) const;
	ResultType operator()(const char* str, ResultType seed) const;

	template <typename Char, typename Traits, typename Allocator>
	ResultType operator()(const std::basic_string<Char, Traits, Allocator>& str, ResultType seed) const
	{
		return this->operator()(str.data(), str.length(), seed);
	}
};

struct FnvaHasher64
{
	typedef unsigned long long ResultType;
	ResultType operator()(const void* data, size_t size, ResultType seed) const;
	ResultType operator()(const char* str, ResultType seed) const;
	template <typename Char, typename Traits, typename Allocator>
	ResultType operator()(const std::basic_string<Char, Traits, Allocator>& str, ResultType seed) const
	{
		return this->operator()(str.data(), str.length(), seed);
	}
};

#ifndef linux
#include <limits.h>
#elif _WIN64
#define __WORDSIZE 64
#endif

#if __WORDSIZE == 64
typedef FnvHasher32 FnvHasher;
typedef FnvaHasher32 FnvaHasher;
#else
typedef FnvHasher64 FnvHasher;
typedef FnvaHasher64 FnvaHasher;
#endif

#endif//CRYPTO_FNV_HASH_HPP_INCLUDED

