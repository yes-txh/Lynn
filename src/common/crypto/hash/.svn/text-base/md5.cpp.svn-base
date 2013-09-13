/////////////////////////////////////////////////////////////////////////////////
// 文件名：  MD5.cpp
// 功能描述：实现了md5加密函数
//          张立明封装
// 算法说明：
//          报文摘要(MD，MessageDigest）。它是将可变长度的报文M作为单向散列函数输入，然后得
//          出一个固定长度的标志H(M)。H(M)通常称为报文摘要(MD)，它主要用于下面情况：
//          通信双方共享一个常规的密钥。发送端先将报文M输入给散列函数H，计算出H（M）即MD，
//          再用常规的密钥对MD进行加密，将加密的MD追加在报文M的后面，发送到接受端。接收端先
//          除去追加在报文M后面加密的MD，用已知的散列函数计算H(M)，再用自己拥有的密钥K对加密
//          的MD解密而得出真实的MD；比较计算出得H(M)和MD，若一致，则收到的报文M是真的。
// 注释时间：7/27/2001
/////////////////////////////////////////////////////////////////////////////////

#include "common/crypto/hash/md5.hpp"
#include <string.h>
#include <stdio.h>

#ifdef _WIN32

#include <common/base/common_windows.h>

namespace {

static HMODULE hDLL = LoadLibraryA("advapi32.dll");
static void (WINAPI* MD5Init)(MD5_CTX* context) = (void (WINAPI* )(MD5_CTX* context))GetProcAddress(hDLL,"MD5Init");
static void (WINAPI* MD5Update)(MD5_CTX* context, const unsigned char* input, unsigned int inlen) =
(void (WINAPI* )(MD5_CTX* context, const unsigned char* input, unsigned int inlen))GetProcAddress(hDLL,"MD5Update");
static void (WINAPI* MD5Final)(MD5_CTX* context) = (void (WINAPI*)(MD5_CTX* context))GetProcAddress(hDLL,"MD5Final");

int MD5_Init(MD5_CTX *c)
{
    MD5Init(c);
    return 0;
}

int MD5_Update(MD5_CTX *c, const void *data, size_t len)
{
    MD5Update(c, (const unsigned char* ) data, (unsigned int) len);
    return 0;
}

int MD5_Final(unsigned char *md, MD5_CTX *c)
{
    MD5Final(c);
	memcpy(md, c->digest, sizeof(c->digest));
    return 0;
}

}

#endif

void MD5::Init()
{
    MD5_Init(&m_ctx);
}

void MD5::Update(const void* data, size_t size)
{
    MD5_Update(&m_ctx, data, size);
}

void MD5::Final(void* digest)
{
    MD5_Final(reinterpret_cast<unsigned char*>(digest), &m_ctx);
}

void MD5::Digest(const void *data, size_t size, void *digest)
{
    MD5 md5;
    md5.Update(data, size);
    md5.Final(digest);
}

// 供外部使用的哈希函数：生成16bytes的hash值
void MD5::Hash(const char* pcSrc, void* pcDest)
{
    Digest((const unsigned char *)pcSrc, ::strlen(pcSrc), pcDest);
}

size_t MD5::Hash64String(const char* pcSrc, char* pcHashValue)
{
    unsigned long long digest;
    Hash64(pcSrc, digest);
    return sprintf(pcHashValue, "%llu", digest);
}

std::string MD5::GetHash64String(const char* pcSrc)
{
    char buffer[sizeof("18446744073709551615")]; // UINT64_MAX
    size_t length = Hash64String(pcSrc, buffer);
    return std::string(buffer, length);
}

size_t MD5::Hash64HexString(const char* pcSrc, char* pcHashValue)
{
    unsigned long long digest;
    Hash64(pcSrc, digest);
    return sprintf(pcHashValue, "%016llX", digest);
}

std::string MD5::GetHash64HexString(const char* pcSrc)
{
    char buffer[sizeof("FFFFFFFFFFFFFFFF")]; // UINT64_MAX
    size_t length = Hash64HexString(pcSrc, buffer);
    return std::string(buffer, length);
}

// 供外部使用的64位哈希函数：生成long long的hash值
void MD5::Hash64(const char* pcSrc, long long& ddwHashValue)
{
    unsigned long long hash;
    Hash64(pcSrc, strlen(pcSrc), hash);
    ddwHashValue = hash;
}

// 供外部使用的64位哈希函数：生成unsigned long long的hash值
void MD5::Hash64(const char* pcSrc, unsigned long long& ddwHashValue)
{
    Hash64(pcSrc, strlen(pcSrc), ddwHashValue);
}

// 供外部使用的64位哈希函数：生成16bytes的hash值
void MD5::Hash(const void* pcSrc, size_t dwSrcLen, void* pcDest)
{
    Digest((unsigned char*)pcSrc, dwSrcLen, pcDest);
}

// 供外部使用的64位哈希函数：生成long long的hash值
void MD5::Hash64(const void* pcSrc, size_t dwSrcLen, long long& ddwHashValue)
{
    long long digest[DigestLength / sizeof(ddwHashValue)];
    Digest((unsigned char*)pcSrc, dwSrcLen, (unsigned char*)digest);
    ddwHashValue = digest[0];
}

// 供外部使用的64位哈希函数：生成unsigned long long的hash值
void MD5::Hash64(const void* pcSrc, size_t dwSrcLen, unsigned long long& ddwHashValue)
{
    unsigned long long digest[DigestLength / sizeof(ddwHashValue)];
    Digest((unsigned char*)pcSrc, dwSrcLen, (unsigned char*)digest);
    ddwHashValue = digest[0];
}

unsigned long long MD5::GetHash64(const char* pcSrc)
{
    unsigned long long result;
    Hash64(pcSrc, result);
    return result;
}

unsigned long long MD5::GetHash64(const void* pcSrc, size_t dwsrclen)
{
    unsigned long long result;
    Hash64(pcSrc, dwsrclen, result);
    return result;
}

#ifdef _LP64 // 64 位，long 是 64 位
// 供外部使用的64位哈希函数：生成long 的hash值
void MD5::Hash64(const char* pcSrc, unsigned long & dwHashValue)
{
    Hash64(pcSrc, strlen(pcSrc), dwHashValue);
}

// 供外部使用的64位哈希函数：生成unsigned long的hash值
void MD5::Hash64(const void* pcSrc, size_t dwSrcLen, unsigned long& dwHashValue)
{
    unsigned long digest[DigestLength / sizeof(dwHashValue)];
    Digest((unsigned char*)pcSrc, dwSrcLen, (unsigned char*)digest);
    dwHashValue = digest[0];
}
#endif

