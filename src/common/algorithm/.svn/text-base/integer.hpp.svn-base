#ifndef COMMON_BASE_INTEGER_ALGORITHM_HPP
#define COMMON_BASE_INTEGER_ALGORITHM_HPP

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include "common/base/stdint.h"

namespace integer_algorithm
{

inline unsigned int ConstLog2(unsigned int x)
{
    unsigned int l = 0;

    if (x >= 1 << 16)
    {
        x >>= 16;
        l |= 16;
    }

    if (x >= 1 << 8)
    {
        x >>= 8;
        l |= 8;
    }

    if (x >= 1 << 4)
    {
        x >>= 4;
        l |= 4;
    }

    if (x >= 1 << 2)
    {
        x >>= 2;
        l |= 2;
    }

    if (x >= 1 << 1) l |= 1;

    return l;
}

#ifdef __GNUC__
inline unsigned int AsmLog2(unsigned int x)
{
    if (__builtin_expect(x == 0, 0))
        return 0;
    unsigned int y;
    __asm__ __volatile__(
        "\tbsr %1, %0\n"
        : "=r"(y)
        : "r" (x)
    );
    return y;
}
#endif

inline unsigned int Log2(unsigned int x)
{
#ifdef __GNUC__
    return __builtin_constant_p(x) ? ConstLog2(x) : AsmLog2(x);
#else
    return ConstLog2(x);
#endif
}

template <typename T>
inline bool IsPowerOf2(T n)
{
    return n != 0 && (n & (n-1)) == 0;
}

inline unsigned UpperPowerOf2(unsigned v)
{
    if (v == 0)
        return 1;
    if (IsPowerOf2(v))
        return v;
    else
        return 1U << (Log2(v - 1) + 1);
}

/// @brief 256之内的数包含为1的bit位
inline const int* PopCountTable()
{
    static const int table[256] =
    {
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
    };
    return table;
}

/// @brief number of 1 bits in integer
inline int PopCount(unsigned int n)
{
#ifdef __GNUC__
    return __builtin_popcount(n);
#elif defined _MSC_VER
    return __popcnt(n);
#else
    for (i = 0; i < 32; i += 8)
        ret += PopCountTable()[(x >> i) & 0xff];
    return ret;
#endif
}

}

using namespace integer_algorithm;

#endif // COMMON_BASE_INTEGER_ALGORITHM_HPP

