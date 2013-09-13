// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/10/11

// This file contains utility functions and classes that help the
// implementation, and management of the Function objects.

#ifndef COMMON_BASE_FUNCTION_FUNCTION_BASE_HPP
#define COMMON_BASE_FUNCTION_FUNCTION_BASE_HPP
#pragma once

#include "common/base/ref_counted.hpp"
#include "common/base/scoped_refptr.hpp"
#include "common/base/function/invoker_storage_base.hpp"

namespace base
{
namespace internal
{

// Holds the Function methods that don't require specialization to reduce
// template bloat.
class FunctionBase
{
    // See:
    // http://www.artima.com/cppsource/safebool.html
    // The Safe Bool Idiom
    typedef bool (FunctionBase::*SafeBool)() const;
public:
    operator SafeBool() const
    {
        return IsNull() ? NULL : &FunctionBase::IsNull;
    }

    SafeBool operator!() const
    {
        return IsNull() ? &FunctionBase::IsNull : NULL;
    }

    // The integral constant expression 0 can be converted into a pointer of
    // this type. It is used by the function template to accept NULL pointers.
    struct ClearType;

    // Clear the Function into an uninitalized state.
    void Clear();

protected:
    // In C++, it is safe to cast function pointers to function pointers of
    // another type. It is not okay to use void*. We create a InvokeFuncStorage
    // that that can store our function pointer, and then cast it back to
    // the original type on usage.
    typedef void(*InvokeFuncStorage)(void);

    FunctionBase(InvokeFuncStorage polymorphic_invoke,
                 scoped_refptr<InvokerStorageBase>* invoker_storage);

    // Force the destructor to be instaniated inside this translation unit so
    // that our subclasses will not get inlined versions.  Avoids more template
    // bloat.
    ~FunctionBase();

    bool IsEqualTo(const FunctionBase& other) const;

    // Returns true if Function is null (doesn't refer to anything).
    bool IsNull() const;

    // swap with other object
    void DoSwap(FunctionBase* other);

    scoped_refptr<InvokerStorageBase> m_invoker_storage;
    InvokeFuncStorage m_polymorphic_invoke;
};

}  // namespace internal
}  // namespace base

#endif // COMMON_BASE_FUNCTION_FUNCTION_BASE_HPP
