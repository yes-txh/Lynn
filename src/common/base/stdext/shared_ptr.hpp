// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 04/15/2011 12:40:47 PM
// Description: shared ptr

#ifndef COMMON_BASE_STDEXT_SHARED_PTR_HPP
#define COMMON_BASE_STDEXT_SHARED_PTR_HPP

#include <memory>

#ifdef __GNUC__
#include <tr1/memory>
#define SHARED_PTR_NAMESPACE std::tr1
#else
#define SHARED_PTR_NAMESPACE std
#endif

namespace stdext
{
using SHARED_PTR_NAMESPACE::shared_ptr;
using SHARED_PTR_NAMESPACE::weak_ptr;
using SHARED_PTR_NAMESPACE::enable_shared_from_this;
using SHARED_PTR_NAMESPACE::bad_weak_ptr;
}

#undef SHARED_PTR_NAMESPACE

#endif // COMMON_BASE_STDEXT_SHARED_PTR_HPP
