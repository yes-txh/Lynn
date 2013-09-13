// RSACrypt.cpp: implementation of the CRSACrypt class.
//
//////////////////////////////////////////////////////////////////////

#include "common/crypto/rsa2/rsa2.h"

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#include "common/crypto/rsa2/rsa2_ref/rsa_source/rsa.h"
}
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRSACrypt::CRSACrypt() : m_public_key_ready(0), m_private_key_ready(0)
{

}

CRSACrypt::~CRSACrypt()
{
    // Clear sensitive information
    memset(&m_random_struct, 0, sizeof(m_random_struct));
    memset(&m_public_key, 0, sizeof(m_public_key));
    memset(&m_private_key, 0, sizeof(m_private_key));
}

void CRSACrypt::InitRandomStruct(int32_t seed)
{
    //static unsigned char seed_byte = seed % 256;
    unsigned char seed_byte = seed % 256;

    uint32_t bytes_needed;

    R_RandomInit(&m_random_struct);

    // Initialize with all zero seed bytes, which will not yield an actual
    //  random number output.
    
    while (1)
    {
        R_GetRandomBytesNeeded(&bytes_needed, &m_random_struct);

        if (bytes_needed == 0)
            break;

        R_RandomUpdate(&m_random_struct, &seed_byte, 1);
    }
}

int32_t CRSACrypt::GenerateKeyPair(int32_t bits_count_keysize)
{
    R_RSA_PROTO_KEY protoKey;
    int32_t status;

    if (bits_count_keysize < MIN_RSA_MODULUS_BITS 
        || bits_count_keysize > MAX_RSA_MODULUS_BITS)
        return -1;

    protoKey.bits = (uint32_t)bits_count_keysize;
    protoKey.useFermat4 = 1;

    memset((unsigned char*)&m_public_key, 0, sizeof(m_public_key));
    memset((unsigned char*)&m_private_key, 0, sizeof(m_private_key));
    status = R_GeneratePEMKeys(&m_public_key,
                               &m_private_key,
                               &protoKey,
                               &m_random_struct);

    if (status)
    {
        return status;
    }

    m_public_key_ready = 1;
    m_private_key_ready = 1;

    return 0;
}

bool CRSACrypt::GetPublicKey(R_RSA_PUBLIC_KEY** public_key)
{
    if (!m_public_key_ready)
        return false;

    *public_key = &m_public_key;

    return true;
}

bool CRSACrypt::GetPrivateKey(R_RSA_PRIVATE_KEY** private_key)
{
    if (!m_private_key_ready)
        return false;

    *private_key = &m_private_key;

    return true;
}

void CRSACrypt::SetPublicKey(R_RSA_PUBLIC_KEY* public_key)
{
    memset(&m_public_key, 0, sizeof(m_public_key));
    memcpy(&m_public_key, public_key, sizeof(m_public_key));

    m_public_key_ready = 1;
}

void CRSACrypt::SetPrivateKey(R_RSA_PRIVATE_KEY* private_key)
{
    memset(&m_private_key, 0, sizeof(m_public_key));
    memcpy(&m_private_key, private_key, sizeof(m_private_key));

    m_private_key_ready = 1;
}

int32_t CRSACrypt::PublicEncrypt(unsigned char* src_data,
                                 uint32_t       src_data_len,
                                 unsigned char* dest_data,
                                 uint32_t*      dest_data_len)
{
    int32_t i;
    int32_t ret = 0;
    uint32_t modulus_len;
    uint32_t max_part_len;
    int32_t part_num;
    uint32_t temp_src_len, temp_dest_len;
    R_RSA_PUBLIC_KEY* public_key;

    if (!GetPublicKey(&public_key))
        return -1;

    modulus_len = (public_key->bits + 7) / 8;
    max_part_len = modulus_len - 11;
    part_num = (src_data_len + max_part_len - 1) / max_part_len;
    *dest_data_len = 0;

    for (i = 0; i < part_num; i++)
    {
        temp_src_len = (i != (part_num - 1)) 
                      ? max_part_len 
                      : (src_data_len - max_part_len * (part_num - 1));

        ret = RSAPublicEncrypt(dest_data + (*dest_data_len),
                                &temp_dest_len,
                                src_data + max_part_len * i,
                                temp_src_len, public_key,
                                &m_random_struct);

        if (ret)
        {
            *dest_data_len = 0;
            break;
        }

        *dest_data_len += temp_dest_len;
    }

    return ret;
}

int32_t CRSACrypt::PrivateDecrypt(unsigned char* src_data,
                                  uint32_t       src_data_len,
                                  unsigned char* dest_data,
                                  uint32_t*      dest_data_len)
{
    int32_t i;
    int32_t ret = 0;
    uint32_t modulus_len;
    int32_t part_num;
    uint32_t temp_src_len, temp_dest_len;
    R_RSA_PRIVATE_KEY* private_key;

    if (!GetPrivateKey(&private_key))
        return -1;

    modulus_len = (private_key->bits + 7) / 8;

    if (src_data_len % modulus_len != 0)
        return -1;

    part_num = src_data_len / modulus_len;
    *dest_data_len = 0;

    for (i = 0; i < part_num; i++)
    {
        temp_src_len = modulus_len;
        ret = RSAPrivateDecrypt(dest_data + (*dest_data_len),
                                 &temp_dest_len,
                                 src_data + modulus_len * i,
                                 temp_src_len,
                                 private_key);

        if (ret)
        {
            *dest_data_len = 0;
            break;
        }

        *dest_data_len += temp_dest_len;
    }

    return ret;
}

int32_t CRSACrypt::PrivateEncrypt(unsigned char* src_data,
                                  uint32_t       src_data_len,
                                  unsigned char* dest_data,
                                  uint32_t*      dest_data_len)
{
    int32_t i;
    int32_t ret = 0;
    uint32_t modulus_len;
    uint32_t max_part_len;
    int32_t part_num;
    uint32_t temp_src_len, temp_dest_len;
    R_RSA_PRIVATE_KEY* private_key;

    if (!GetPrivateKey(&private_key))
        return -1;

    modulus_len = (private_key->bits + 7) / 8;
    max_part_len = modulus_len - 11;
    part_num = (src_data_len + max_part_len - 1) / max_part_len;
    *dest_data_len = 0;

    for (i = 0; i < part_num; i++)
    {
        temp_src_len = (i != (part_num - 1))
                      ? max_part_len
                      : (src_data_len - max_part_len * (part_num - 1));
        ret = RSAPrivateEncrypt(dest_data + (*dest_data_len),
                                 &temp_dest_len,
                                 src_data + max_part_len * i,
                                 temp_src_len,
                                 private_key);

        if (ret)
        {
            *dest_data_len = 0;
            break;
        }

        *dest_data_len += temp_dest_len;
    }

    return ret;
}

int32_t CRSACrypt::PublicDecrypt(unsigned char* src_data,
                                 uint32_t       src_data_len,
                                 unsigned char* dest_data,
                                 uint32_t*      dest_data_len)
{
    int32_t i;
    int32_t ret = 0;
    uint32_t modulus_len;
    int32_t part_num;
    uint32_t temp_src_len, temp_dest_len;
    R_RSA_PUBLIC_KEY* public_key;

    if (!GetPublicKey(&public_key))
        return -1;

    modulus_len = (public_key->bits + 7) / 8;

    /////////////////////////////////
    // frankyhe 2.3
    // ±‹√‚modulus_len == 0≤˙…˙“Ï≥£
    if (modulus_len == 0)
    {
        return -1 ;
    }

    if (src_data_len % modulus_len != 0)
        return -1;

    part_num = src_data_len / modulus_len;
    *dest_data_len = 0;

    for (i = 0; i < part_num; i++)
    {
        temp_src_len = modulus_len;
        ret = RSAPublicDecrypt(dest_data + (*dest_data_len),
                               &temp_dest_len,
                               src_data + modulus_len * i,
                               temp_src_len,
                               public_key);

        if (ret)
        {
            *dest_data_len = 0;
            break;
        }

        *dest_data_len += temp_dest_len;
    }

    return ret;
}

const char* CRSACrypt::GetRSAErrorText(int32_t error_code)
{
    const char* error_string;
    char buf[80];

    // Convert the rsa error code to a string if it is recognized.
    switch (error_code)
    {
    case RE_CONTENT_ENCODING:
        error_string = 
            "(Encrypted) content has RFC 1113 encoding error";
        break;
    case RE_DATA:
        error_string = 
            "(Encrypted) data has error";
        break;
    case RE_DIGEST_ALGORITHM:
        error_string = 
            "Message-digest algorithm is invalid";
        break;
    case RE_ENCODING:
        error_string = 
            "Block has RFC 1113 encoding error";
        break;
    case RE_KEY:
        error_string = 
            "Recovered DES key cannot decrypt encrypted content";
        break;
    case RE_KEY_ENCODING:
        error_string = 
            "Encrypted key has RFC 1113 encoding error";
        break;
    case RE_LEN:
        error_string = 
            "Encrypted key length or signature length is out of range";
        break;
    case RE_MODULUS_LEN:
        error_string = 
            "Modulus length is out of range";
        break;
    case RE_NEED_RANDOM:
        error_string = 
            "Random structure is not seeded";
        break;
    case RE_PRIVATE_KEY:
        error_string = 
            "Private key cannot encrypt message digest,"
            " or cannot decrypt encrypted key";
        break;
    case RE_PUBLIC_KEY:
        error_string = 
            "Public key cannot encrypt data encryption key,"
            " or cannot decrypt signature";
        break;
    case RE_SIGNATURE:
        error_string = 
            "Signature is incorrect";
        break;
    case RE_SIGNATURE_ENCODING:
        error_string = 
            "(Encrypted) signature has RFC 1113 encoding error";
        break;
    case RE_ENCRYPTION_ALGORITHM:
        error_string = 
            "Encryption algorithm is invalid";
        break;
    default:
        sprintf(buf, "ErrorCode 0x%04X", error_code);
        error_string = buf;
        break;
    }

    return error_string;
}

void CRSACrypt::WritePublicKeyDesc(char* buf, char* crlf)
{
    char temp_buf[1024];

    buf[0] = 0x00;
    sprintf(temp_buf, "Public Key, %u bits:", m_public_key.bits);
    strcat(buf, temp_buf);
    strcat(buf, crlf);
    sprintf(temp_buf, "  modulus: ");
    strcat(buf, temp_buf);
    WriteBigInteger(temp_buf,
                    m_public_key.modulus,
                    sizeof(m_public_key.modulus));
    strcat(buf, temp_buf);
    strcat(buf, crlf);
    sprintf(temp_buf, "  exponent: ");
    strcat(buf, temp_buf);
    WriteBigInteger(temp_buf,
                    m_public_key.exponent,
                    sizeof(m_public_key.exponent));
    strcat(buf, temp_buf);
}

void CRSACrypt::WritePrivateKeyDesc(char* buf, char* crlf)
{
    char temp_buf[1024];

    buf[0] = 0x00;
    sprintf(temp_buf, "Private Key, %u bits:", m_private_key.bits);
    strcat(buf, temp_buf);
    strcat(buf, crlf);
    sprintf(temp_buf, "  modulus: ");
    strcat(buf, temp_buf);
    WriteBigInteger(temp_buf,
                    m_private_key.modulus,
                    sizeof(m_private_key.modulus));
    strcat(buf, temp_buf);
    strcat(buf, crlf);
    sprintf(temp_buf, "  public exponent: ");
    strcat(buf, temp_buf);
    WriteBigInteger(temp_buf,
                    m_private_key.publicExponent,
                    sizeof(m_private_key.publicExponent));
    strcat(buf, temp_buf);
    strcat(buf, crlf);
    sprintf(temp_buf, "  exponent: ");
    strcat(buf, temp_buf);
    WriteBigInteger(temp_buf,
                    m_private_key.exponent,
                    sizeof(m_private_key.exponent));
    strcat(buf, temp_buf);
    strcat(buf, crlf);
    sprintf(temp_buf, "  prime 1: ");
    strcat(buf, temp_buf);
    WriteBigInteger(temp_buf,
                    m_private_key.prime[0],
                    sizeof(m_private_key.prime[0]));
    strcat(buf, temp_buf);
    strcat(buf, crlf);
    sprintf(temp_buf, "  prime 2: ");
    strcat(buf, temp_buf);
    WriteBigInteger(temp_buf,
                    m_private_key.prime[1],
                    sizeof(m_private_key.prime[1]));
    strcat(buf, temp_buf);
    strcat(buf, crlf);
    sprintf(temp_buf, "  prime exponent 1: ");
    strcat(buf, temp_buf);
    WriteBigInteger(temp_buf,
                    m_private_key.primeExponent[0],
                    sizeof(m_private_key.primeExponent[0]));
    strcat(buf, temp_buf);
    strcat(buf, crlf);
    sprintf(temp_buf, "  prime exponent 2: ");
    strcat(buf, temp_buf);
    WriteBigInteger(temp_buf,
                    m_private_key.primeExponent[1],
                    sizeof(m_private_key.primeExponent[1]));
    strcat(buf, temp_buf);
    strcat(buf, crlf);
    sprintf(temp_buf, "  coefficient: ");
    strcat(buf, temp_buf);
    WriteBigInteger(temp_buf,
                    m_private_key.coefficient,
                    sizeof(m_private_key.coefficient));
    strcat(buf, temp_buf);
}

// Write the byte string 'integer' to buffer, skipping over leading zeros. 
void CRSACrypt::WriteBigInteger(char*          buf, 
                                unsigned char* integer,
                                uint32_t       integer_len)
{
    char temp_buf[4];

    while (*integer == 0 && integer_len > 0)
    {
        integer++;
        integer_len--;
    }

    buf[0] = 0x00;

    if (integer_len == 0)
    {
        // Special case, just print a zero.
        strcat(buf, "00");
        return;
    }

    for (; integer_len > 0; integer_len--)
    {
        sprintf(temp_buf, "%02X ", (uint32_t)(*integer++));
        strcat(buf, temp_buf);
    }
}
