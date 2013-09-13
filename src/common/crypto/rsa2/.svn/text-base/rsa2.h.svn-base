// rsa2.h
// wookin@tencent.com

#ifndef _RSA2LIB_H_
#define _RSA2LIB_H_

#include "common/crypto/rsa2/rsa2_global.h"
#include "common/crypto/rsa2/rsa2_ref/rsa_source/rsa_ref.h"

#ifndef WIN32 // linux
#include <stdint.h>
#else // WIN32
#include "common/base/vc_stdint.h"
#endif //

//
// RSACrypt.h: interface for the CRSACrypt class.
//
// RSA加密/解密 （非对称加密）
// 签名用
//

class CRSACrypt
{
public:
    CRSACrypt();
    virtual ~CRSACrypt();

    void InitRandomStruct(int32_t seed);
    int32_t GenerateKeyPair(int32_t bits_count_keysize);
    bool GetPublicKey(R_RSA_PUBLIC_KEY** public_key);
    bool GetPrivateKey(R_RSA_PRIVATE_KEY** private_key);
    void SetPublicKey(R_RSA_PUBLIC_KEY* public_key);
    void SetPrivateKey(R_RSA_PRIVATE_KEY* private_key);
    int32_t PublicEncrypt(unsigned char* src_data, uint32_t src_data_len,
                          unsigned char* dest_data, uint32_t* dest_data_len);

    int32_t PrivateDecrypt(unsigned char* src_data, uint32_t src_data_len,
                           unsigned char* dest_data, uint32_t* dest_data_len);

    int32_t PrivateEncrypt(unsigned char* src_data, uint32_t src_data_len,
                           unsigned char* dest_data, uint32_t* dest_data_len);

    int32_t PublicDecrypt(unsigned char* src_data, uint32_t src_data_len,
                          unsigned char* dest_data, uint32_t* dest_data_len);

    const char* GetRSAErrorText(int32_t error_code);
    void WritePublicKeyDesc(char* buf, char* crlf);
    void WritePrivateKeyDesc(char* buf, char* crlf);

private:
    void WriteBigInteger(char* buf, unsigned char* integer, uint32_t integer_len);

private:
    int32_t m_public_key_ready;
    int32_t m_private_key_ready;
    R_RANDOM_STRUCT m_random_struct;
    R_RSA_PUBLIC_KEY m_public_key;
    R_RSA_PRIVATE_KEY m_private_key;
};

// ------------------------------------------------
typedef struct _tagKeyFileHead
{
    char            fmt[5];  // TKEY
    unsigned char   ver;      // 1
    unsigned char   is_pubkey; // 1:PUBLIC_KEY; 0:PRIVATE_KEY
    unsigned char   key_type;
} KeyFileHead;
// ------------------------------------------------

// 非对程key type
enum
{

    KEYTYPE_ECC = 0, // ECC key
    KEYTYPE_RSA ,    // RSA key
};

#endif // _RSA2LIB_H_
