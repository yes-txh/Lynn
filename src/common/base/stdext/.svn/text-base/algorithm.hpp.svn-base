#ifndef COMMON_BASE_STDEXT_ALGORITHM_HPP
#define COMMON_BASE_STDEXT_ALGORITHM_HPP

/// @file
/// @brief some STL algorithms extension
/// @author Chen Feng <phongchen@tencent.com>
/// @date 7 Jan, 2011

#include <algorithm>

namespace stdext {

/// @brief similarly to std::binary_search but return iterator
/// @param first start of range
/// @param last one past the last element of the range
/// @param value value to be found
/// @return position found
/// @retval last not found
template <typename ForwardIterator, typename Type>
ForwardIterator binary_find(
    ForwardIterator first,
    ForwardIterator last,
    const Type& value
)
{
    ForwardIterator i = ::std::lower_bound(first, last, value);
    return i != last && !(value < *i) ? i : last;
}

/// @brief similarly to std::binary_search but return iterator
/// @param first start of range
/// @param last one past the last element of the range
/// @param value value to be found
/// @param compare User-defined predicate function object that defines sense
///        in which one element is less than another. A binary predicate takes
///        two arguments and returns true when satisfied and false when not
///        satisfied.
/// @return position found
/// @retval last not found
template <typename ForwardIterator, typename Type, typename BinaryPredicate>
ForwardIterator binary_find(
    ForwardIterator first,
    ForwardIterator last,
    const Type& value,
    BinaryPredicate compare
)
{
    ForwardIterator i = ::std::lower_bound(first, last, value, compare);
    return i != last && !compare(value, *i) ? i : last;
}

/// @brief Copies all of the elements referred to by the iterator i in the range
///        [first,last) for which pred(*i) is true.
/// @param first An input iterator that indicates the start of a range to check for a condition.
/// @param last An input iterator that indicates the end of a range.
/// @param pred The condition to test for. This is provided by a user-defined
///        predicate function object that defines the condition to be satisfied
///        by the element being searched for. A predicate takes a single argument
///        and returns true or false.
/// @note has been introduced in C++0x
template<typename InputIterator, typename OutputIterator, typename Predicate>
OutputIterator copy_if(
    InputIterator first,
    InputIterator last,
    OutputIterator result,
    Predicate pred
)
{
    while (first != last)
    {
        if (pred(*first))
            *result++ = *first;
        ++first;
    }
    return result;
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
using ::std::is_sorted_until;
using ::srd::is_sorted;
#else
/**
 *  @brief  Determines the end of a sorted sequence.
 *  @ingroup sorting_algorithms
 *  @param  first   An iterator.
 *  @param  last    Another iterator.
 *  @return  An iterator pointing to the last iterator i in [first, last)
 *           for which the range [first, i) is sorted.
 */
template<typename _ForwardIterator>
_ForwardIterator
is_sorted_until(_ForwardIterator __first, _ForwardIterator __last)
{
    if (__first == __last)
        return __last;

    _ForwardIterator __next = __first;
    for (++__next; __next != __last; __first = __next, ++__next)
        if (*__next < *__first)
            return __next;
    return __next;
}

/**
 *  @brief  Determines the end of a sorted sequence using comparison functor.
 *  @ingroup sorting_algorithms
 *  @param  first   An iterator.
 *  @param  last    Another iterator.
 *  @param  comp    A comparison functor.
 *  @return  An iterator pointing to the last iterator i in [first, last)
 *           for which the range [first, i) is sorted.
 */
template<typename _ForwardIterator, typename _Compare>
_ForwardIterator
is_sorted_until(_ForwardIterator __first, _ForwardIterator __last,
                _Compare __comp)
{
    if (__first == __last)
        return __last;

    _ForwardIterator __next = __first;
    for (++__next; __next != __last; __first = __next, ++__next)
        if (__comp(*__next, *__first))
            return __next;
    return __next;
}

/**
 *  @brief  Determines whether the elements of a sequence are sorted.
 *  @ingroup sorting_algorithms
 *  @param  first   An iterator.
 *  @param  last    Another iterator.
 *  @return  True if the elements are sorted, false otherwise.
 */
template<typename _ForwardIterator>
inline bool
is_sorted(_ForwardIterator __first, _ForwardIterator __last)
{ return stdext::is_sorted_until(__first, __last) == __last; }

/**
 *  @brief  Determines whether the elements of a sequence are sorted
 *          according to a comparison functor.
 *  @ingroup sorting_algorithms
 *  @param  first   An iterator.
 *  @param  last    Another iterator.
 *  @param  comp    A comparison functor.
 *  @return  True if the elements are sorted, false otherwise.
 */
template<typename _ForwardIterator, typename _Compare>
inline bool
is_sorted(_ForwardIterator __first, _ForwardIterator __last,
          _Compare __comp)
{ return stdext::is_sorted_until(__first, __last, __comp) == __last; }

#endif // __GXX_EXPERIMENTAL_CXX0X__

} // end namespace stdext

#endif // COMMON_BASE_STDEXT_ALGORITHM_HPP

