// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 2011年04月19日 06时56分33秒
// Description: UUID class

#ifndef COMMON_SYSTEM_UUID_UUID_HPP
#define COMMON_SYSTEM_UUID_UUID_HPP

#include <string>
#include <stdexcept>
#include "common/system/uuid/uuid.h"
#include "common/system/uuid/uuidP.h"

class Uuid
{
public:
    Uuid()
    {
        Clear();
    }

    explicit Uuid(const char* str)
    {
        if (!Parse(str))
            throw std::runtime_error("Invalid UUID string");
    }

    explicit Uuid(const std::string& str)
    {
        if (!Parse(str))
            throw std::runtime_error("Invalid UUID string");
    }

    std::string ToString() const
    {
        char buf[37];
        uuid_unparse(m_buf, buf);
        return std::string(buf, 36);
    }

    bool Parse(const char* str)
    {
        return uuid_parse(str, m_buf) == 0;
    }

    bool Parse(const std::string& str)
    {
        return Parse(str.c_str());
    }

    void Generate()
    {
        return uuid_generate(m_buf);
    }

    bool IsNull() const
    {
        return uuid_is_null(m_buf) != 0;
    }

    void Clear()
    {
        uuid_clear(m_buf);
    }

    int Compare(const Uuid& that) const
    {
        return uuid_compare(this->m_buf, that.m_buf);
    }

    const uuid_t& Bytes() const
    {
        return m_buf;
    }
private:
    uuid_t m_buf;
};

inline bool operator<(const Uuid& lhs, const Uuid& rhs)
{
    return lhs.Compare(rhs) < 0;
}
inline bool operator<=(const Uuid& lhs, const Uuid& rhs)
{
    return lhs.Compare(rhs) <= 0;
}
inline bool operator==(const Uuid& lhs, const Uuid& rhs)
{
    return lhs.Compare(rhs) == 0;
}
inline bool operator!=(const Uuid& lhs, const Uuid& rhs)
{
    return lhs.Compare(rhs) != 0;
}
inline bool operator>(const Uuid& lhs, const Uuid& rhs)
{
    return lhs.Compare(rhs) > 0;
}
inline bool operator>=(const Uuid& lhs, const Uuid& rhs)
{
    return lhs.Compare(rhs) >= 0;
}

#endif // COMMON_SYSTEM_UUID_UUID_HPP
