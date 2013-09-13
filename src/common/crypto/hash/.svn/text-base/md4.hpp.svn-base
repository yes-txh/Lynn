#ifndef MD4_HPP_INCLUDED
#define MD4_HPP_INCLUDED

#include <stddef.h>
#include <string.h>


#ifdef _WIN32
///////////////////////////////////////////////////////////////////////////////
// Windows implementation, using advapi

typedef struct {
    unsigned long i[2];
    unsigned long buf[4];
    unsigned char in[64];
    unsigned char digest[16];
} MD4_CTX;

class MD4Implement
{
public:
    static const size_t DigestLength = 16;
protected:
    MD4Implement(){}
    ~MD4Implement(){}
public:
    void Init();
    void Update(const void* data, size_t size);
    void Final(void* digest);
public:
    static void Digest(const void* data, size_t size, void* digest);
private:
    MD4_CTX m_ctx;
};

#else

///////////////////////////////////////////////////////////////////////////////
// other platforms, using openssl

#include "common/crypto/hash/openssl/md4.h"

class MD4Implement
{
public:
    static const size_t DigestLength = MD4_DIGEST_LENGTH;
protected:
    MD4Implement()
    {
    }
    ~MD4Implement(){}
public:
    void Init()
    {
        MD4_Init(&m_ctx);
    }
    void Update(const void* data, size_t size)
    {
        MD4_Update(&m_ctx, data, size);
    }
    void Final(void* digest)
    {
        MD4_Final(reinterpret_cast<unsigned char*>(digest), &m_ctx);
    }
public:
    static void Digest(const void* data, size_t size, void* digest)
    {
        MD4Implement md4;
        md4.Init();
        md4.Update(data, size);
        md4.Final(digest);
    }
private:
    MD4_CTX m_ctx;
};

#endif

class MD4 : public MD4Implement
{
public:
    MD4()
    {
        Init();
    }
public:
    using MD4Implement::Digest;
    ///////////////////////////////////////////////////////////////////////////
    // calculate data digest
    static void Digest(const void* data, size_t size, unsigned int& result)
    {
        unsigned int digest[DigestLength/sizeof(unsigned int)];
        Digest(data, size, digest);
        result = digest[0];
    }
    static void Digest(const void* data, size_t size, unsigned long& result)
    {
        unsigned long digest[DigestLength/sizeof(unsigned long)];
        Digest(data, size, digest);
        result = digest[0];
    }
    static void Digest(const void* data, size_t size, unsigned long long& result)
    {
        unsigned long long digest[DigestLength/sizeof(unsigned long long)];
        Digest(data, size, digest);
        result = digest[0];
    }

    ///////////////////////////////////////////////////////////////////////////
    // calculate string's digest
    static void Digest(const char* str, unsigned int& result)
    {
        Digest(str, strlen(str), result);
    }
    static void Digest(const char* str, unsigned long& result)
    {
        Digest(str, strlen(str), result);
    }
    static void Digest(const char* str, unsigned long long& result)
    {
        Digest(str, strlen(str), result);
    }
    static void Digest(const char* str, unsigned char result[16])
    {
        Digest(str, strlen(str), result);
    }

    ///////////////////////////////////////////////////////////////////////////
    // calculate data digest and return its value
    static unsigned int Digest32(const void* data, size_t size)
    {
        unsigned int result;
        Digest(data, size, result);
        return result;
    }
    static unsigned long long Digest64(const void* data, size_t size)
    {
        unsigned long long result;
        Digest(data, size, result);
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    // calculate string digest and return its value
    static unsigned int Digest32(const char* str)
    {
        return Digest32(str, strlen(str));
    }
    static unsigned long long Digest64(const char* str)
    {
        return Digest64(str, strlen(str));
    }
};

#endif//MD4_HPP_INCLUDED
