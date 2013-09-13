////////////////////////////////////////////////////////////////////////////////
// 版权信息：(C) 2003, Jerry.CS.HUST.China
// 文件名：  MD5.h
// 作者：    Jerry
// 功能描述：对数据进行MD5加密
// 版本：    V1.1.2003.02.25
// 修改历史：
////////////////////////////////////////////////////////////////////////////////

#ifndef _MD5_H_
#define _MD5_H_

#include <stddef.h>
#include <string.h>
#include <string>
#include <common/base/platform_features.hpp>
#include <common/base/static_assert.hpp>

#ifdef _WIN32
#include <wtypes.h>
typedef struct {
    ULONG i[2];
    ULONG buf[4];
    unsigned char in[64];
    unsigned char digest[16];
} MD5_CTX;
#else
#include "common/crypto/hash/openssl/md5.h"
#endif

class MD5
{
public:
    static const size_t DigestLength = 16;

public:
    MD5()
    {
        Init();
    }

    /// 初始化或者复位
    void Init();

    /// 累积缓冲区
    void Update(const void* data, size_t size);

    /// 得到最终结果
    void Final(void* digest);

    //////////////////////////////////////////////////////////////////////////
    // overloading for capture buffer size error at compile time

    template <typename ResultType, size_t Size>
    void Final(ResultType (*buffer)[Size])
    {
        STATIC_ASSERT(sizeof(*buffer) >= DigestLength);
        Final(static_cast<void*>(buffer));
    }

    // need non-const reference here to catpure the following bad case
    // unsigned char digest[19];
    // sha1.Final(digest); // implicit array to pointer cast
    template <typename ResultType, size_t Size>
    void Final(ResultType (&buffer)[Size])
    {
        STATIC_ASSERT(sizeof(buffer) >= DigestLength);
        Final(static_cast<void*>(&buffer[0]));
    }
public:
    static void Digest(const void *data, size_t size, void *digest);
    static void Digest(const char* str, void *digest)
    {
        Digest(str, strlen(str), digest);
    }

    /// 供外部使用的哈希函数：生成 16 bytes 的 hash 值
    DEPRECATED_BY(Digest) static void Hash(const char* pcSrc, void* pcDest);

    /// 供外部使用的64位哈希函数：生成 16 bytes 的 hash 值
    DEPRECATED_BY(Digest) static void Hash(const void* pcSrc, size_t dwSrcLen, void* pcDest);

    /// 供外部使用的 64 位哈希函数：生成 long long 的字符串形式 hash 值，缓冲区长度至少 21
    /// @return 字符串的长度，不包含末尾的 \0
    static size_t Hash64String(const char* pcSrc, char* pcHashValue);

    /// 供外部使用的64位哈希函数：生成 unsigned long long 的字符串形式 hash 值，缓冲区长度至少 21
    static std::string GetHash64String(const char* pcSrc);

    /// 供外部使用的64位哈希函数：生成 unsigned long long 的十六进制字符串形式 hash 值，缓冲区长度至少 17
    /// @return 字符串的长度，不包含末尾的 \0
    static size_t Hash64HexString(const char* pcSrc, char* pcHashValue);

    /// 供外部使用的64位哈希函数：生成 unsigned long long 的十六进制字符串形式 hash 值
    static std::string GetHash64HexString(const char* pcSrc);

    /// 供外部使用的64位哈希函数：生成long long的hash值。
    /// 已废弃：不要用有符号数存 hash
    DEPRECATED static void Hash64(const char* pcSrc, long long& ddwHashValue);

    /// 供外部使用的64位哈希函数：生成long long的hash值。
    /// 已废弃：不要用有符号数存 hash
    DEPRECATED static void Hash64(const void* pcsrc, size_t dwsrclen, long long& ddwhashvalue);

    /// 供外部使用的64位哈希函数：生成unsigned long long的hash值
    static void Hash64(const char* pcSrc, unsigned long long& ddwHashValue);

    /// 供外部使用的64位哈希函数：生成 unsigned long long 的 hash 值
    static void Hash64(const void* pcSrc, size_t dwsrclen, unsigned long long& ddwHashValue);
#ifdef _LP64 // Unix 64 位下，long 是 64 位的
    /// 供外部使用的64位哈希函数：生成 long 的 hash 值
    static void Hash64(const char* pcSrc, unsigned long & ddwHashValue);
    /// 供外部使用的64位哈希函数：生成 unsigned long 的 hash 值
    static void Hash64(const void* pcSrc, size_t dwsrclen, unsigned long& ddwHashValue);
#endif
    /// 供外部使用的 64 位哈希函数：生成 long long 的 hash 值
    static unsigned long long GetHash64(const char* pcSrc);
    static unsigned long long GetHash64(const void* pcSrc, size_t dwsrclen);

    static unsigned long long GetHash64(const std::string& str)
    {
        return GetHash64(str.data(), str.length());
    }
private:
    MD5_CTX m_ctx;
};

#endif//_MD5_H_

