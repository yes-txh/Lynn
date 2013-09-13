// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/26/11
// Description: strict bool type

#ifndef COMMON_BASE_STRICT_BOOL_HPP
#define COMMON_BASE_STRICT_BOOL_HPP
#pragma once

#include "common/base/static_assert.hpp"

namespace base {

class strict_bool
{
    typedef bool (strict_bool::*SafeBool)() const;
public:
    strict_bool() : m_value(false) {}
    strict_bool(bool value) : m_value(value) {} // NOLINT(runtime/explicit)

    template <typename T>
    strict_bool(const T&) // NOLINT(runtime/explicit)
    {
        STATIC_ASSERT(sizeof(T) != sizeof(T), "strict_bool can only accept real bool as ctor param");
    }

    strict_bool& operator=(bool value)
    {
        m_value = value;
        return *this;
    }

    template <typename T>
    void operator=(const T& value)
    {
        STATIC_ASSERT(sizeof(T) != sizeof(T), "strict_bool can only be assigned to real bool");
    }

    operator SafeBool() const
    {
        return &strict_bool::operator!;
    }
    bool operator!() const
    {
        return !m_value;
    }
    template <typename T>
    operator T() const
    {
        STATIC_ASSERT(sizeof(T) != sizeof(T), "strict_bool can only cast to bool");
        return T();
    }

    bool value() const
    {
        return m_value;
    }

    bool operator==(bool rhs) const
    {
        return value() == rhs;
    }

    bool operator==(strict_bool rhs) const
    {
        return value() == rhs.value();
    }

    template <typename T>
    bool operator==(const T& rhs) const
    {
        STATIC_ASSERT(sizeof(T) != sizeof(T), "strict_bool can only compare with real bool");
        return false;
    }

    bool operator!=(bool rhs) const
    {
        return value() != rhs;
    }

    bool operator!=(strict_bool rhs) const
    {
        return value() != rhs.value();
    }

    template <typename T>
    bool operator!=(const T& rhs) const
    {
        STATIC_ASSERT(sizeof(T) != sizeof(T), "strict_bool can only compare with real bool");
        return false;
    }

private:
    bool m_value;
};

template <typename T>
inline bool operator==(const T& lhs, const strict_bool& rhs)
{
    return rhs == lhs;
}

template <typename T>
inline bool operator!=(const T& lhs, const strict_bool& rhs)
{
    return rhs != lhs;
}

} // end namespace base

using ::base::strict_bool;

#endif // COMMON_BASE_STRICT_BOOL_HPP
