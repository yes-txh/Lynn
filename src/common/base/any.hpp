// Copyright (c) 2011, Tencent Inc. All rights reserved.

#ifndef COMMON_BASE_ANY_HPP_INCLUDED
#define COMMON_BASE_ANY_HPP_INCLUDED

#include <algorithm>
#include <exception>
#include <typeinfo>

/// An Any class represents a general type and is capable of storing any type,
/// supporting type-safe extraction of the internally stored data.
///
/// Code taken from the Boost 1.33.1 library.
/// Original copyright by Kevlin Henney.
class Any
{
public:
    /// Creates an empty any type.
    Any():
        m_content(0)
    {
    }

    /// Creates an any which stores the init parameter inside.
    ///
    /// Example:
    ///     Any a(13);
    ///     Any a(string("12345"));
    template <typename ValueType>
    Any(const ValueType& value): // NOLINT(runtime/explicit)
        m_content(new Holder<ValueType>(value))
    {
    }

    /// Copy constructor, works with empty Anys and initialized Any values.
    Any(const Any& other):
        m_content(other.m_content ? other.m_content->Clone() : 0)
    {
    }

    ~Any()
    {
        delete m_content;
    }

    /// Swaps the content of the two Anys.
    Any& Swap(Any& rhs)
    {
        std::swap(m_content, rhs.m_content);
        return *this;
    }

    /// Assignment operator for all types != Any.
    ///
    /// Example:
    ///    Any a = 13;
    ///    Any a = string("12345");
    template <typename ValueType>
    Any& operator=(const ValueType& rhs)
    {
        Any(rhs).Swap(*this);
        return *this;
    }

    /// Assignment operator for Any.
    Any& operator=(const Any& rhs)
    {
        Any(rhs).Swap(*this);
        return *this;
    }

    /// returns true if the Any is empty
    bool IsEmpty() const
    {
        return !m_content;
    }

    /// Returns the type information of the stored content.
    /// If the Any is empty typeid(void) is returned.
    /// It is suggested to always query an Any for its type info before trying
    /// to extract data via an AnyCast/RefAnyCast.
    const std::type_info& Type() const
    {
        return m_content ? m_content->Type() : typeid(void);
    }

private:
    class Placeholder
    {
    public:
        virtual ~Placeholder()
        {
        }

        virtual const std::type_info& Type() const = 0;
        virtual Placeholder* Clone() const = 0;
    };

    template <typename ValueType>
    class Holder: public Placeholder
    {
    public:
        Holder(const ValueType& value):
            m_held(value)
        {
        }

        virtual const std::type_info& Type() const
        {
            return typeid(ValueType);
        }

        virtual Placeholder* Clone() const
        {
            return new Holder(m_held);
        }

        ValueType m_held;
    };

private:
    template <typename ValueType>
    friend ValueType* AnyCast(Any* any);

    template <typename ValueType>
    friend ValueType* UnsafeAnyCast(Any* any);

    Placeholder* m_content;
};

/// AnyCast operator used to extract the ValueType from an Any*.
/// Will return a pointer to the stored value.
///
/// Example Usage:
///     MyType* pTmp = AnyCast<MyType*>(pAny).
/// Will return NULL if the cast fails, i.e. types don't match.
template <typename ValueType>
ValueType* AnyCast(Any* operand)
{
    return operand && operand->Type() == typeid(ValueType)
        ? &static_cast<Any::Holder<ValueType>*>(operand->m_content)->m_held
        : 0;
}

/// AnyCast operator used to extract a const ValueType pointer from an const Any*.
/// Will return a const pointer to the stored value.
///
/// Example Usage:
///     const MyType* pTmp = AnyCast<MyType*>(pAny).
/// Will return NULL if the cast fails, i.e. types don't match.
template <typename ValueType>
const ValueType* AnyCast(const Any* operand)
{
    return AnyCast<ValueType>(const_cast<Any*>(operand));
}


/// AnyCast operator used to extract a copy of the ValueType from an const Any&.
///
/// Example Usage:
///     MyType tmp = AnyCast<MyType>(anAny).
/// Will throw a std::bad_cast if the cast fails.
/// Dont use an AnyCast in combination with references, i.e.
/// MyType& tmp = .m.. or const MyType& = ...
/// Some compilers will accept this code although a copy is returned.
/// Use the RefAnyCast in these cases.
template <typename ValueType>
ValueType AnyCast(const Any& operand)
{
    ValueType* result = AnyCast<ValueType>(const_cast<Any*>(&operand));
    if (!result) throw std::bad_cast();
    return *result;
}

/// AnyCast operator used to extract a copy of the ValueType from an Any&.
///
/// Example Usage:
///     MyType tmp = AnyCast<MyType>(anAny).
/// Will throw a std::bad_cast if the cast fails.
/// Dont use an AnyCast in combination with references,
/// i.e. MyType& tmp = ... or const MyType& tmp = ...
/// Some compilers will accept this code although a copy is returned.
/// Use the RefAnyCast in these cases.
template <typename ValueType>
ValueType AnyCast(Any& operand)
{
    ValueType* result = AnyCast<ValueType>(&operand);
    if (!result) throw std::bad_cast();
    return *result;
}

/// AnyCast operator used to return a const reference to the internal data.
///
/// Example Usage:
///     const MyType& tmp = RefAnyCast<MyType>(anAny);
template <typename ValueType>
const ValueType& RefAnyCast(const Any & operand)
{
    ValueType* result = AnyCast<ValueType>(const_cast<Any*>(&operand));
    if (!result) throw std::bad_cast();
    return *result;
}

/// AnyCast operator used to return a reference to the internal data.
///
/// Example Usage:
///     MyType& tmp = RefAnyCast<MyType>(anAny);
template <typename ValueType>
ValueType& RefAnyCast(Any& operand)
{
    ValueType* result = AnyCast<ValueType>(&operand);
    if (!result) throw std::bad_cast();
    return *result;
}

/// The "unsafe" versions of AnyCast are not part of the
/// public interface and may be removed at any time. They are
/// required where we know what type is stored in the any and can't
/// use typeid() comparison, e.g., when our types may travel across
/// different shared libraries.
template <typename ValueType>
ValueType* UnsafeAnyCast(Any* operand)
{
    return &static_cast<Any::Holder<ValueType>*>(operand->m_content)->m_held;
}

/// The "unsafe" versions of AnyCast are not part of the
/// public interface and may be removed at any time. They are
/// required where we know what type is stored in the any and can't
/// use typeid() comparison, e.g., when our types may travel across
/// different shared libraries.
template <typename ValueType>
const ValueType* UnsafeAnyCast(const Any* operand)
{
    return AnyCast<ValueType>(const_cast<Any*>(operand));
}

namespace std
{
inline void swap(Any& lhs, Any& rhs)
{
    lhs.Swap(rhs);
}
}

#endif // COMMON_BASE_ANY_HPP_INCLUDED
