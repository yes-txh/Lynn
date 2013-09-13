// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/09/11
// Description:

#ifndef COMMON_BASE_FUNCTION_INVOKER_STORAGE_BASE_HPP
#define COMMON_BASE_FUNCTION_INVOKER_STORAGE_BASE_HPP
#pragma once

#include "common/base/ref_counted.hpp"
#include "common/base/scoped_refptr.hpp"

namespace base
{
namespace internal
{

// InvokerStorageBase is used to provide an opaque handle that the Function
// class can use to represent a function object with bound arguments.  It
// behaves as an existential type that is used by a corresponding
// DoInvoke function to perform the function execution.  This allows
// us to shield the Function class from the types of the bound argument via
// "type erasure."
class InvokerStorageBase : public RefCountedBase<InvokerStorageBase>
{
protected:
    friend class RefCountedBase<InvokerStorageBase>;
    virtual ~InvokerStorageBase() {}
};

// This structure exists purely to pass the returned |m_invoker_storage| from
// Bind() to Function while avoiding an extra AddRef/Release() pair.
//
// To do this, the constructor of Function<> must take a const-ref.  The
// reference must be to a const object otherwise the compiler will emit a
// warning about taking a reference to a temporary.
//
// So, the internal m_invoker_storage must be mutable.
template <typename T>
struct InvokerStorageHolder
{
    explicit InvokerStorageHolder(T* invoker_storage)
        : m_invoker_storage(invoker_storage)
    {
    }
    mutable scoped_refptr<InvokerStorageBase> m_invoker_storage;
};

template <typename T>
InvokerStorageHolder<T> MakeInvokerStorageHolder(T* o)
{
    return InvokerStorageHolder<T>(o);
}

}  // namespace internal
}  // namespace base

#endif // COMMON_BASE_FUNCTION_INVOKER_STORAGE_BASE_HPP
