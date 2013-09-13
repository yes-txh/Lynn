#ifndef COMMON_BASE_STDEXT_ITERATOR_HPP
#define COMMON_BASE_STDEXT_ITERATOR_HPP

#include <iterator>

namespace stdext {
namespace iterator {

// Helper functions for typenamees like bidirectional iterators not supporting
// operator+ and operator-
//
// Usage:
//   const std::list<T>::iterator p = get_some_iterator();
//   const std::list<T>::iterator prev = stdext::iterator::prior(p);
//   const std::list<T>::iterator next = stdext::iterator::next(prev, 2);

// Contributed by Dave Abrahams

template <typename T>
inline T next(T x) { return ++x; }

template <typename T, typename Distance>
inline T next(T x, Distance n)
{
    std::advance(x, n);
    return x;
}

template <typename T>
inline T prior(T x) { return --x; }

template <typename T, typename Distance>
inline T prior(T x, Distance n)
{
    std::advance(x, -n);
    return x;
}

} // end namespace iterator
} // end namespace stdext

#endif // COMMON_BASE_STDEXT_ITERATOR_HPP
