// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

/// @author phongchen
/// @date Nov 10, 2010
/// @brief base 128 variant integer encoding

#ifndef COMMON_ENCODING_VARIANT_INTEGER_HPP
#define COMMON_ENCODING_VARIANT_INTEGER_HPP

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include "common/base/platform_features.hpp"
#include "common/base/type_cast.hpp"
#include "common/base/type_traits.hpp"

/// base128 variant integer encoding implementation base128
/// encode each 7 bit as a group in an encoding byte from the lowest group
namespace base128
{

/// bits used in each encoding byte
const unsigned int ENCODING_WIDTH = CHAR_BIT - 1;

/// encoding mask of each byte
const unsigned int ENCODING_MASK = (1U << ENCODING_WIDTH) - 1;

/// whether encoding sequence is end
const unsigned int ENCODING_CONTINUE_MASK = 1U << ENCODING_WIDTH;

inline int DecodedSize(const void* buffer, size_t size)
{
    const unsigned char* p = static_cast<const unsigned char*>(buffer);
    size_t pos = 0;
    while (pos < size)
    {
        unsigned char byte = p[pos];
        ++pos;
        if ((byte & ENCODING_CONTINUE_MASK) == 0)
            return pos;
    }
    return -1;
}

inline int UncheckedDecodedSize(const void* buffer)
{
    const unsigned char* p = static_cast<const unsigned char*>(buffer);
    size_t pos = 0;
    for (;;)
    {
        unsigned char byte = p[pos];
        ++pos;
        if ((byte & ENCODING_CONTINUE_MASK) == 0)
            break;
    }
    return pos;
}


#ifdef __x86_64__
inline unsigned int log2(unsigned int x)
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

inline unsigned int log2(unsigned long long x) // NOLINT(runtime/int)
{
    if (__builtin_expect(x == 0, 0))
        return 0;
    unsigned long long y; // NOLINT(runtime/int)
    __asm__ __volatile__(
        "\tbsrq %1, %0\n"
        : "=r"(y)
        : "r" (x)
    );
    return y;
}

inline unsigned int log2(unsigned char x)
{
    return log2(static_cast<unsigned int>(x));
}

inline unsigned int log2(unsigned short x) // NOLINT(runtime/int)
{
    return log2(static_cast<unsigned int>(x));
}

inline unsigned int log2(unsigned long x) // NOLINT(runtime/int)
{
    if (sizeof(x) == sizeof(unsigned long long)) // NOLINT(runtime/int)
        return log2(static_cast<unsigned long long>(x)); // NOLINT(runtime/int)
    else
        return log2(static_cast<unsigned int>(x));
}

template <typename T>
unsigned int SignificantBits(T value)
{
    return log2(value) + 1;
}
#endif

/// handle unsigned type
template <bool is_signed> // false
struct HandleSign
{
private:
    // lookup table is faster than div by 7,
    // even if div by 7 can be optimized to mutiple and shift by compiler
    static unsigned int Div7(unsigned int n)
    {
        // we know n will not greater than 64
        static const unsigned char table[] =
        {
            // 2  3  4  5  6  7
            0, 0, 0, 0, 0, 0, 0,
            1, 1, 1, 1, 1, 1, 1,
            2, 2, 2, 2, 2, 2, 2,
            3, 3, 3, 3, 3, 3, 3,
            4, 4, 4, 4, 4, 4, 4,
            5, 5, 5, 5, 5, 5, 5,
            6, 6, 6, 6, 6, 6, 6,
            7, 7, 7, 7, 7, 7, 7,
            8, 8, 8, 8, 8, 8, 8,
            9, 9, 9, 9, 9, 9, 9,
            10, 10, 10, 10, 10, 10, 10,
        };
        return table[n];
    }
public:
    template <typename Type>
    static size_t EncodedSize(Type value)
    {
        STATIC_ASSERT(TypeTraits::IsUnsignedInteger<Type>::Value);
#ifdef __x86_64__
        return Div7(SignificantBits(value) + ENCODING_WIDTH - 1);
#else
        size_t result = 1;
        while (value > ENCODING_MASK)
        {
            value >>= ENCODING_WIDTH;
            ++result;
        }
        return result;
#endif
    }

private:
    template <typename Type>
    static int UncheckedEncodeKnownSize(
        Type value,
        size_t encoded_size,
        void* buffer
        )
    {
        unsigned char* p = static_cast<unsigned char*>(buffer);
#define BASE128_ENCODE_ONE_BYTE() \
        *p++ = static_cast<unsigned char>( \
            (value & ENCODING_MASK) | ENCODING_CONTINUE_MASK); \
        value >>= ENCODING_WIDTH

        // encoded size is already known, expand the loop manually
        switch (encoded_size)
        {
        case 10:
            BASE128_ENCODE_ONE_BYTE(); // fallthrough
        case 9:
            BASE128_ENCODE_ONE_BYTE(); // fallthrough
        case 8:
            BASE128_ENCODE_ONE_BYTE(); // fallthrough
        case 7:
            BASE128_ENCODE_ONE_BYTE(); // fallthrough
        case 6:
            BASE128_ENCODE_ONE_BYTE(); // fallthrough
        case 5:
            BASE128_ENCODE_ONE_BYTE(); // fallthrough
        case 4:
            BASE128_ENCODE_ONE_BYTE(); // fallthrough
        case 3:
            BASE128_ENCODE_ONE_BYTE(); // fallthrough
        case 2:
            BASE128_ENCODE_ONE_BYTE(); // fallthrough
        case 1:
            // last byte has no highest bit set
            *p++ = static_cast<unsigned char>(value);
            break;
        default:
            assert(!"Invalid encoded_size");
        }
#undef BASE128_ENCODE_ONE_BYTE
        return p - static_cast<unsigned char*>(buffer);
    }

public:
    template <typename Type>
    static int UncheckedEncode(Type value, void* buffer)
    {
        STATIC_ASSERT(TypeTraits::IsUnsignedInteger<Type>::Value);
#if 1
        unsigned int result_size = EncodedSize(value);
        return UncheckedEncodeKnownSize(value, result_size, buffer);
#else
        unsigned char* p = static_cast<unsigned char*>(buffer);
        while (value > ENCODING_MASK)
        {
            *p++ = static_cast<unsigned char>(
                (value & ENCODING_MASK) | ENCODING_CONTINUE_MASK);
            value >>= ENCODING_WIDTH;
        }
        // last byte has no highest bit set
        *p++ = static_cast<unsigned char>(value);
        return p - static_cast<unsigned char*>(buffer);
#endif
    }

    template <typename Type>
    static int Encode(Type value, void* buffer, size_t size)
    {
        STATIC_ASSERT(TypeTraits::IsUnsignedInteger<Type>::Value);

        unsigned int result_size = EncodedSize(value);
        if (result_size > size)
            return -1;

        return UncheckedEncodeKnownSize(value, result_size, buffer);
    }

    template <typename Type>
    static int Decode(const void* buffer, size_t size, Type* value)
    {
        STATIC_ASSERT(TypeTraits::IsUnsignedInteger<Type>::Value);

        *value = 0;

        size_t pos = 0;
        unsigned int shift = 0;
        const unsigned char* p = static_cast<const unsigned char*>(buffer);
        while (pos < size)
        {
            unsigned char byte = p[pos];
            if (shift < sizeof(Type) * CHAR_BIT)
                *value |= (static_cast<Type>(byte) & ENCODING_MASK) << shift;
            shift += ENCODING_WIDTH;
            ++pos;
            if ((byte & ENCODING_CONTINUE_MASK) == 0)
                return pos;
        }
        return -1;
    }

    template <typename Type>
    static int UncheckedDecode(const void* buffer, Type* value)
    {
        STATIC_ASSERT(TypeTraits::IsUnsignedInteger<Type>::Value);

        *value = 0;
        size_t pos = 0;
        unsigned int shift = 0;
        const unsigned char* p = static_cast<const unsigned char*>(buffer);
        for (;;)
        {
            unsigned char byte = p[pos];
            if (shift < sizeof(Type) * CHAR_BIT)
                *value |= (static_cast<Type>(byte) & ENCODING_MASK) << shift;
            shift += ENCODING_WIDTH;
            ++pos;
            if ((byte & ENCODING_CONTINUE_MASK) == 0)
                return pos;
        }
        return -1;
    }
};

/// handle signed type
/// using ZigZag algorithm to map signed integer into unsigned
template <>
struct HandleSign<true>
{
private:
    /// map signed integer to unsigned
    template <typename Type>
    static typename TypeTraits::RemoveSign<Type>::Type MapToUnsigned(Type value)
    {
        STATIC_ASSERT(TypeTraits::IsSignedInteger<Type>::Value);
        // ZigZag encoding, euqals to: n = (abs(n) << 1) - (n < 0)
        value = (value << 1U) ^ (value >> (sizeof(Type) * CHAR_BIT - 1));
        typedef typename TypeTraits::RemoveSign<Type>::Type UnsignedType;
        return static_cast<UnsignedType>(value);
    }

public:
    template <typename Type>
    static size_t EncodedSize(Type value)
    {
        STATIC_ASSERT(TypeTraits::IsSignedInteger<Type>::Value);
        return HandleSign<false>::EncodedSize(MapToUnsigned(value));
    }

    template <typename Type>
    static int Encode(Type value, void* buffer, size_t size)
    {
        STATIC_ASSERT(TypeTraits::IsSignedInteger<Type>::Value);
        return HandleSign<false>::Encode(MapToUnsigned(value), buffer, size);
    }

    template <typename Type>
    static int UncheckedEncode(Type value, void* buffer)
    {
        STATIC_ASSERT(TypeTraits::IsSignedInteger<Type>::Value);
        return HandleSign<false>::UncheckedEncode(MapToUnsigned(value), buffer);
    }

private:
    template <typename Type>
    static typename TypeTraits::AddSign<Type>::Type MapToSigned(Type value)
    {
        STATIC_ASSERT(TypeTraits::IsUnsignedInteger<Type>::Value);

        typedef typename TypeTraits::AddSign<Type>::Type SignedType;
        // lowest bit is sign bit
        if (value & 1) // minus
            return -static_cast<SignedType>((value + 1) >> 1);
        else
            return static_cast<SignedType>(value >> 1);
    }

public:
    template <typename Type>
    static int Decode(const void* buffer, size_t size, Type* value)
    {
        STATIC_ASSERT(TypeTraits::IsSignedInteger<Type>::Value);

        // we need large enough type to hold encoding bit
        unsigned long long n; // NOLINT(runtime/int)
        int result = HandleSign<false>::Decode(buffer, size, &n);
        if (result > 0)
        {
            *value = MapToSigned(n);
            return result;
        }
        return -1;
    }

    template <typename Type>
    static int UncheckedDecode(const void* buffer, Type* value)
    {
        STATIC_ASSERT(TypeTraits::IsSignedInteger<Type>::Value);

        // we need large enough type to hold encoding bit
        unsigned long long n; // NOLINT(runtime/int)
        int result = HandleSign<false>::UncheckedDecode(buffer, &n);
        *value = MapToSigned(n);
        return result;
    }
};

} // end namespace base128

/// using struct as strong namespace(can't be 'using'ed)
struct VariantInteger
{
private:
    VariantInteger();
    ~VariantInteger();
public:
    /// @param value value to be encoded
    /// @param buffer output buffer
    /// @param size output buffer size
    /// @return number of bytes output
    /// @retval <0 no enough buffer
    template <typename Type, typename ArgType>
    static int Encode(ArgType value, void* buffer, size_t size)
    {
        const bool sign = TypeTraits::IsSignedInteger<Type>::Value;
        return base128::HandleSign<sign>::Encode(
            implicit_cast<Type>(value), buffer, size);
    }

    /// encode without checking buffer overflow
    /// @param value value to be encoded
    /// @param buffer output buffer
    /// @param size output buffer size
    /// @return number of bytes output
    template <typename Type, typename ArgType>
    static int UncheckedEncode(ArgType value, void* buffer)
    {
        const bool sign = TypeTraits::IsSignedInteger<Type>::Value;
        return base128::HandleSign<sign>::UncheckedEncode(
            implicit_cast<Type>(value), buffer);
    }

    /// @param buffer input buffer
    /// @param size input buffer size
    /// @param value *value to be decoded
    /// @return number of bytes consumed
    /// @retval <0 unexpected encoding stream terminate
    template <typename Type>
    static int Decode(const void* buffer, size_t size, Type* value)
    {
        const bool sign = TypeTraits::IsSignedInteger<Type>::Value;
        return base128::HandleSign<sign>::Decode(buffer, size, value);
    }

    /// decode without checking buffer overflow
    /// @param buffer input buffer
    /// @param value value to be encoded
    /// @return number of bytes output
    template <typename Type>
    static int UncheckedDecode(const void* buffer, Type* value)
    {
        const bool sign = TypeTraits::IsSignedInteger<Type>::Value;
        return base128::HandleSign<sign>::UncheckedDecode(buffer, value);
    }

    /// get integer encoded byte size
    /// @param value value to be encoded
    template <typename Type, typename ArgType>
    static size_t EncodedSize(ArgType value)
    {
        const bool sign = TypeTraits::IsSignedInteger<Type>::Value;
        return base128::HandleSign<sign>::EncodedSize(
            implicit_cast<Type>(value));
    }

    /// count next decoded size in given buffer
    /// @param buffer buffer to be decoded
    /// @param size buffer size
    /// @return number of bytes consumed
    /// @retval <0 unexpected encoding stream terminate
    static inline int DecodedSize(const void* buffer, size_t size)
    {
        return base128::DecodedSize(buffer, size);
    }

    /// count decoded size without checking buffer overflow
    /// @param buffer buffer to be decoded
    /// @param size buffer size
    /// @return number of bytes consumed
    static inline int UncheckedDecodedSize(const void* buffer)
    {
        return base128::UncheckedDecodedSize(buffer);
    }

    /// max encoded byte size of type
    /// usage:
    /// usnigned char buffer[base128::MaxEncodedSizeOf<int>::Value]
    template <typename Type>
    struct MaxEncodedSizeOf
    {
        // div by 7 up round
        static const size_t Value = ((sizeof(Type) + 1) * CHAR_BIT - 1) / (CHAR_BIT - 1);
    };

    /// max possible encoded size, we know in current time long long is the
    /// largest integer type
    static const size_t MAX_ENCODED_SIZE =
        MaxEncodedSizeOf<long long>::Value; // NOLINT(runtime/int)
};

#if STATIC_CONST_MEMBER_NEED_DEFINATION
template <typename Type> const size_t VariantInteger::MaxEncodedSizeOf<Type>::Value;
WEAK_SYMBOL const size_t VariantInteger::MAX_ENCODED_SIZE;
#endif

#endif // COMMON_ENCODING_VARIANT_INTEGER_HPP

