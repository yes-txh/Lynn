////////////////////////////////////////////////////////////////////////////////
// ��Ȩ��Ϣ��(C) 2003, Jerry.CS.HUST.China
// �ļ�����  MD5.h
// ���ߣ�    Jerry
// ���������������ݽ���MD5����
// �汾��    V1.1.2003.02.25
// �޸���ʷ��
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

    /// ��ʼ�����߸�λ
    void Init();

    /// �ۻ�������
    void Update(const void* data, size_t size);

    /// �õ����ս��
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

    /// ���ⲿʹ�õĹ�ϣ���������� 16 bytes �� hash ֵ
    DEPRECATED_BY(Digest) static void Hash(const char* pcSrc, void* pcDest);

    /// ���ⲿʹ�õ�64λ��ϣ���������� 16 bytes �� hash ֵ
    DEPRECATED_BY(Digest) static void Hash(const void* pcSrc, size_t dwSrcLen, void* pcDest);

    /// ���ⲿʹ�õ� 64 λ��ϣ���������� long long ���ַ�����ʽ hash ֵ���������������� 21
    /// @return �ַ����ĳ��ȣ�������ĩβ�� \0
    static size_t Hash64String(const char* pcSrc, char* pcHashValue);

    /// ���ⲿʹ�õ�64λ��ϣ���������� unsigned long long ���ַ�����ʽ hash ֵ���������������� 21
    static std::string GetHash64String(const char* pcSrc);

    /// ���ⲿʹ�õ�64λ��ϣ���������� unsigned long long ��ʮ�������ַ�����ʽ hash ֵ���������������� 17
    /// @return �ַ����ĳ��ȣ�������ĩβ�� \0
    static size_t Hash64HexString(const char* pcSrc, char* pcHashValue);

    /// ���ⲿʹ�õ�64λ��ϣ���������� unsigned long long ��ʮ�������ַ�����ʽ hash ֵ
    static std::string GetHash64HexString(const char* pcSrc);

    /// ���ⲿʹ�õ�64λ��ϣ����������long long��hashֵ��
    /// �ѷ�������Ҫ���з������� hash
    DEPRECATED static void Hash64(const char* pcSrc, long long& ddwHashValue);

    /// ���ⲿʹ�õ�64λ��ϣ����������long long��hashֵ��
    /// �ѷ�������Ҫ���з������� hash
    DEPRECATED static void Hash64(const void* pcsrc, size_t dwsrclen, long long& ddwhashvalue);

    /// ���ⲿʹ�õ�64λ��ϣ����������unsigned long long��hashֵ
    static void Hash64(const char* pcSrc, unsigned long long& ddwHashValue);

    /// ���ⲿʹ�õ�64λ��ϣ���������� unsigned long long �� hash ֵ
    static void Hash64(const void* pcSrc, size_t dwsrclen, unsigned long long& ddwHashValue);
#ifdef _LP64 // Unix 64 λ�£�long �� 64 λ��
    /// ���ⲿʹ�õ�64λ��ϣ���������� long �� hash ֵ
    static void Hash64(const char* pcSrc, unsigned long & ddwHashValue);
    /// ���ⲿʹ�õ�64λ��ϣ���������� unsigned long �� hash ֵ
    static void Hash64(const void* pcSrc, size_t dwsrclen, unsigned long& ddwHashValue);
#endif
    /// ���ⲿʹ�õ� 64 λ��ϣ���������� long long �� hash ֵ
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

