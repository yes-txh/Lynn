#ifndef COMMON_BASE_STDEXT_POINTEE_COMPARE_HPP
#define COMMON_BASE_STDEXT_POINTEE_COMPARE_HPP

#include <functional>

/// @file
/// @brief compare pointee instead of pointer
/// @author Chen Feng <phongchen@tencent.com>
/// @date Jan 12, 2011

namespace stdext {

template <
    typename ValueCompare,
    typename FirstArgumentType = const typename ValueCompare::first_argument_type*,
    typename SecondArgumentType = const typename ValueCompare::second_argument_type*
>
struct pointee_compare :
    public std::binary_function<FirstArgumentType, SecondArgumentType, bool>
{
    bool operator()(
        FirstArgumentType p1,
        SecondArgumentType p2) const
    {
        return ValueCompare()(*p1, *p2);
    }
};

/// pointer version of the 'less' functor
template <typename Type>
struct pointee_less : public pointee_compare<std::less<Type> >
{
};

/// pointer version of the 'less_equal' functor
template <typename Type>
struct pointee_less_equal : public pointee_compare<std::less_equal<Type> >
{
};

/// pointer version of the 'greater' functor
template <typename Type>
struct pointee_greater : public pointee_compare<std::greater<Type> >
{
};

/// pointer version of the 'greater_equal' functor
template <typename Type>
struct pointee_greater_equal : public pointee_compare<std::greater_equal<Type> >
{
};

/// pointer version of the 'equal_to' functor
template <typename Type>
struct pointee_equal_to : public pointee_compare<std::equal_to<Type> >
{
};

/// pointer version of the 'not_equal_to' functor
template <typename Type>
struct pointee_not_equal_to : public pointee_compare<std::not_equal_to<Type> >
{
};

} // end namespace stdext

#endif // COMMON_BASE_STDEXT_POINTEE_COMPARE_HPP

