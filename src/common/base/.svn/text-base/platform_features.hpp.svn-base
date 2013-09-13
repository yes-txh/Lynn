// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASE_PLATFORM_FEATURES_HPP
#define COMMON_BASE_PLATFORM_FEATURES_HPP

#ifdef __GNUC__
/// support thread safe static variable initialization
#define HAS_THREAD_SAFE_STATICS 1

/// static const integral members need definition out of class
#define STATIC_CONST_MEMBER_NEED_DEFINATION 1

/// convert gcc version to a number. eg, gcc 4.5.1 -> 40501
#define GCC_VERSION_NUMBER() (__GNUC__*10000 + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__)
#endif

#if defined _MSC_VER
# define THREAD_LOCAL __declapsec(thread)
# define WEAK_SYMBOL __declapsec(selectany)
# define ALWAYS_INLINE __forceinline
#elif defined __GNUC__
# define THREAD_LOCAL __thread
# define WEAK_SYMBOL __attribute__((weak))
# define ALWAYS_INLINE inline __attribute__((always_inline))
#else
# error Unknown compiler
#endif

/// define DEPRECATED and DEPRECATED_BY
#ifdef __DEPRECATED
# if defined __GNUC__
#  define DEPRECATED __attribute__((deprecated))
#  if GCC_VERSION_NUMBER() >= 40500
#   define DEPRECATED_BY(new_symbol) __attribute__((deprecated("please use '" #new_symbol "' to instead")))
#   define DEPRECATED_MESSAGE(msg) __attribute__((deprecated(msg)))
#  else
#   define DEPRECATED_BY(new_symbol) DEPRECATED
#   define DEPRECATED_MESSAGE(msg) DEPRECATED
#  endif
# elif defined _MSC_VER
#  define DEPRECATED __declspec(deprecated)
#  define DEPRECATED_BY(new_symbol) __declspec(deprecated("deprecated, please use " #new_symbol " to instead"))
#  define DEPRECATED_MESSAGE(msg) __declspec(deprecated(msg))
# else
#  define DEPRECATED
#  define DEPRECATED_BY(new_symbol)
#   define DEPRECATED_MESSAGE(msg)
# endif
#else
# define DEPRECATED
# define DEPRECATED_BY(x)
# define DEPRECATED_MESSAGE(x)
#endif // __DEPRECATED

/// known alignment insensitive platforms
#if defined(__i386__) || \
    defined(__x86_64__) || \
    defined(_M_IX86) || \
    defined(_M_X64)
#define ALIGNMENT_INSENSITIVE_PLATFORM 1
#endif

/// define __attribute__ of gcc to null under non gcc
#ifndef __GNUC__
#define __attribute__(x)
#endif

// 用以标注一个参数不能为空
// void swap(int *p, int* q) __attribute__((nonnull(1, 2)));

// 变参格式的说明
// int log_printf(const char* format, ...) __attribute__((format(1, 2)));

#endif // COMMON_BASE_PLATFORM_FEATURES_HPP
