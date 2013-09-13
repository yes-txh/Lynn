// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/09/11

#include "common/base/ref_counted.hpp"

#include <assert.h>
#include "common/system/concurrency/atomic/atomic.h"

namespace base
{

bool IRefCounted::IsUnique() const
{
    return AtomicGet(m_ref_count) == 1;
}

int IRefCounted::GetRefCount() const
{
    return AtomicGet(m_ref_count);
}

IRefCounted::IRefCounted() : m_ref_count(0)
{
}

IRefCounted::~IRefCounted()
{
}

int IRefCounted::AddRef() const
{
    return AtomicIncrement(m_ref_count);
}

bool IRefCounted::Release() const
{
    int count = AtomicDecrement(m_ref_count);
    assert(count >= 0);
    return count == 0;
}

}  // namespace base
