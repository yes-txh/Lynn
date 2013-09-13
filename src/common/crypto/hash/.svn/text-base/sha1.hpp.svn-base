#ifndef COMMON_CRYPTO_HASH_SHA1_HPP
#define COMMON_CRYPTO_HASH_SHA1_HPP

/// @file
/// @author phongchen <phongchen@tencent.com>
/// @date Nov 28, 2010

#include <stddef.h>
#include <string.h>
#include <string>
#include "common/base/uncopyable.hpp"
#include "common/base/static_assert.hpp"
#include "common/crypto/hash/openssl/sha.h"

/// SHA1 hash alogrithm
class SHA1
{
    // copy is safe
public:
    static const size_t kDigestLength = SHA_DIGEST_LENGTH;

public:
    SHA1()
    {
        Init();
    }

    /// init or reset
    void Init()
    {
        SHA1_Init(&m_info);
    }

    /// add more buffer to calculate
    void Update(const void* buffer, size_t length)
    {
        SHA1_Update(&m_info, buffer, length);
    }

    /// final and retrieve result
    /// input is buffer
    void Final(void* buffer)
    {
        SHA1_Final(static_cast<unsigned char*>(buffer), &m_info);
    }

    //////////////////////////////////////////////////////////////////////////
    // overloading for capture buffer size error at compile time

    template <typename ResultType, size_t Size>
    void Final(ResultType (*buffer)[Size])
    {
        STATIC_ASSERT(sizeof(*buffer) >= kDigestLength);
        Final(static_cast<void*>(buffer));
    }

    // need non-const reference here to catpure the following bad case
    // unsigned char digest[19];
    // sha1.Final(digest); // implicit array to pointer cast
    template <typename ResultType, size_t Size>
    void Final(ResultType (&buffer)[Size])
    {
        STATIC_ASSERT(sizeof(buffer) >= kDigestLength);
        Final(static_cast<void*>(&buffer[0]));
    }

    /// get final result as string
    std::string FinalString()
    {
        unsigned char digest[kDigestLength];
        Final(&digest);

        std::string result;
        result.reserve(2 * kDigestLength);
        for (size_t i = 0; i < kDigestLength; ++i)
        {
            static const char hex[] = "0123456789abcdef";
            result.push_back(hex[digest[i] >> 4]);
            result.push_back(hex[digest[i] & 0x0F]);
        }
        return result;
    }

public: // static utility functions
    /// @brief hash buffer and retrieve hash result
    /// @param buffer buffer to be hashed
    /// @param size buffer size
    /// @param result buffer to contain hash result,
    ///               should be at lease kDigestLength bytes
    static void Digest(const void* buffer, size_t size, void* result)
    {
        SHA1 sha1;
        sha1.Update(buffer, size);
        sha1.Final(result);
    }

    static void Digest(const char* str, void* result)
    {
        Digest(str, strlen(str), result);
    }

    // overloading for compile time buffer size check
    template <typename ResultType, size_t ResultSize>
    static void Digest(
        const void* buffer,
        size_t size,
        ResultType (*result)[ResultSize]
        )
    {
        STATIC_ASSERT(sizeof(*result) >= kDigestLength);
        Digest(buffer, size, static_cast<void*>(result));
    }

    // overloading for compile time buffer size check
    template <typename ResultType, size_t ResultSize>
    static void Digest(
        const void* buffer,
        size_t size,
        ResultType (&result)[ResultSize]
        )
    {
        STATIC_ASSERT(sizeof(result) >= kDigestLength);
        Digest(buffer, size, static_cast<void*>(result));
    }

    /// get hash result as hex string
    /// input is buffer
    static std::string HashToHexString(const void* buffer, size_t size)
    {
        SHA1 sha1;
        sha1.Update(buffer, size);
        return sha1.FinalString();
    }

    /// get hash result as hex string
    /// input is string
    static std::string HashToHexString(const char* str)
    {
        return HashToHexString(str, strlen(str));
    }
private:
    SHA_CTX m_info;
};

#endif // COMMON_CRYPTO_HASH_SHA1_HPP
