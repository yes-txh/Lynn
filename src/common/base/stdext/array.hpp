#ifndef COMMON_BASE_STDEXT_ARRAY_HPP
#define COMMON_BASE_STDEXT_ARRAY_HPP

/*
 * from Boost.Array
 * See
 *      http://www.boost.org/libs/array/
 * for documentation.
 *
 * The original author site is at: http://www.josuttis.com/
 */

#include <assert.h>
#include <cstddef>
#include <stdexcept>

// Handles broken standard libraries better than <iterator>
#include <algorithm>

namespace stdext {

template<class T, std::size_t N>
class array {
public:
    T elements[N];    // fixed-size array of elements of type T

public:
    // type definitions
    typedef T              value_type;
    typedef T*             iterator;
    typedef const T*       const_iterator;
    typedef T&             reference;
    typedef const T&       const_reference;
    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;

    // iterator support
    iterator begin() { return elements; }
    const_iterator begin() const { return elements; }
    iterator end() { return elements+N; }
    const_iterator end() const { return elements+N; }

    // reverse iterator support
#if defined(_MSC_VER) && (_MSC_VER == 1300)
    // workaround for broken reverse_iterator in VC7
    typedef std::reverse_iterator<std::_Ptrit<value_type, difference_type, iterator,
                                  reference, iterator, reference> > reverse_iterator;
    typedef std::reverse_iterator<
        std::_Ptrit<value_type, difference_type, const_iterator,
        const_reference, iterator, reference> > const_reverse_iterator;
#else
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
#endif

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    // operator[]
    reference operator[](size_type i)
    {
        assert(i < N && "out of range");
        return elements[i];
    }

    const_reference operator[](size_type i) const
    {
        assert(i < N && "out of range");
        return elements[i];
    }

    // at() with range check
    reference at(size_type i) { rangecheck(i); return elements[i]; }
    const_reference at(size_type i) const { rangecheck(i); return elements[i]; }

    // front() and back()
    reference front()
    {
        return elements[0];
    }

    const_reference front() const
    {
        return elements[0];
    }

    reference back()
    {
        return elements[N-1];
    }

    const_reference back() const
    {
        return elements[N-1];
    }

    // size is constant
    static size_type size() { return N; }
    static bool empty() { return false; }
    static size_type max_size() { return N; }
    enum { static_size = N };

    // swap (note: linear complexity)
    void swap(array<T, N>& y) {
        std::swap_ranges(begin(), end(), y.begin());
    }

    // direct access to data (read-only)
    const T* data() const { return elements; }
    T* data() { return elements; }

    // use array as C array (direct read/write access to data)
    T* c_array() { return elements; }

    // assignment with type conversion
    template <typename T2>
    array<T, N>& operator=(const array<T2, N>& rhs) {
        std::copy(rhs.begin(), rhs.end(), begin());
        return *this;
    }

    // assign one value to all elements
    void assign(const T& value)
    {
        std::fill_n(begin(), size(), value);
    }

    // check range (may be private because it is static)
    static void rangecheck(size_type i) {
        if (i >= size()) {
            throw std::out_of_range("array<>: index out of range");
        }
    }
};

template<class T>
class array<T, 0>
{
public:
    // type definitions
    typedef T              value_type;
    typedef T*             iterator;
    typedef const T*       const_iterator;
    typedef T&             reference;
    typedef const T&       const_reference;
    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;

    // iterator support
    iterator begin() { return iterator( reinterpret_cast< T * >( this ) ); }
    const_iterator begin() const {
        return const_iterator(  reinterpret_cast< const T * >( this ) );
    }
    iterator end() { return begin(); }
    const_iterator end() const { return begin(); }

    // reverse iterator support
#if defined(_MSC_VER) && (_MSC_VER == 1300)
    // workaround for broken reverse_iterator in VC7
    typedef std::reverse_iterator<std::_Ptrit<value_type, difference_type, iterator,
                                  reference, iterator, reference> > reverse_iterator;
    typedef std::reverse_iterator<
        std::_Ptrit<value_type, difference_type, const_iterator,
        const_reference, iterator, reference> > const_reverse_iterator;
#else
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
#endif

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    // operator[]
    reference operator[](size_type i)
    {
        return failed_rangecheck();
    }

    const_reference operator[](size_type i) const
    {
        return failed_rangecheck();
    }

    // at() with range check
    reference at(size_type i)               {   return failed_rangecheck(); }
    const_reference at(size_type i) const   {   return failed_rangecheck(); }

    // front() and back()
    reference front()
    {
        return failed_rangecheck();
    }

    const_reference front() const
    {
        return failed_rangecheck();
    }

    reference back()
    {
        return failed_rangecheck();
    }

    const_reference back() const
    {
        return failed_rangecheck();
    }

    // size is constant
    static size_type size() { return 0; }
    static bool empty() { return true; }
    static size_type max_size() { return 0; }
    enum { static_size = 0 };

    void swap(array<T, 0>& y) {
    }

    // direct access to data (read-only)
    const T* data() const { return 0; }
    T* data() { return 0; }

    // use array as C array (direct read/write access to data)
    T* c_array() { return 0; }

    // assignment with type conversion
    template <typename T2>
    array<T, 0>& operator= (const array<T2, 0>& ) {
        return *this;
    }

    // assign one value to all elements
    void assign(const T& ) {   }

    // check range (may be private because it is static)
    static reference failed_rangecheck() {
            std::out_of_range e("attempt to access element of an empty array");
            throw e;
            //
            // We need to return something here to keep
            // some compilers happy: however we will never
            // actually get here....
            //
            static T placeholder;
            return placeholder;
        }
};

// comparisons
template<class T, std::size_t N>
bool operator==(const array<T, N>& x, const array<T, N>& y) {
    return std::equal(x.begin(), x.end(), y.begin());
}
template<class T, std::size_t N>
bool operator<(const array<T, N>& x, const array<T, N>& y) {
    return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
}
template<class T, std::size_t N>
bool operator!=(const array<T, N>& x, const array<T, N>& y) {
    return !(x == y);
}
template<class T, std::size_t N>
bool operator>(const array<T, N>& x, const array<T, N>& y) {
    return y < x;
}
template<class T, std::size_t N>
bool operator<=(const array<T, N>& x, const array<T, N>& y) {
    return !(y < x);
}
template<class T, std::size_t N>
bool operator>=(const array<T, N>& x, const array<T, N>& y) {
    return !(x < y);
}

// global swap()
template<class T, std::size_t N>
inline void swap(array<T, N>& x, array<T, N>& y) {
    x.swap(y);
}

} // namespace stdext

#endif // COMMON_BASE_STDEXT_ARRAY_HPP

