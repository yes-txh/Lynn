/***********************************************************************
����ͨ��Э����ԴУ��
{"ָ���ַ���"}{�����(�޷�������)}{CRC32(ָ���ַ���+�����)}

����ָ���ַ���Ϊ"SOSO" 5�ֽ�
�����                   4�ֽ�
CRC32                   4�ֽ�
��Ϊ�������ǰ׺��������ԴУ��
***********************************************************************/

//////////////////////////////////////////////////////////////////////////
// modified by ivanhuang at 20101104
// 1.�޸Ĵ�����
// 2.����UnitTest
//////////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASE_PROTOCLO_ENCRYPTION_H_ // NOLINT
#define COMMON_BASE_PROTOCLO_ENCRYPTION_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
#include <WinSock.h>
#define int32_t INT32
#else
#include <arpa/inet.h>
#endif // _WIN32

#include "common/crypto/hash/crc.hpp"

enum {
    kWebSearchEncryptionKey         = 0x7899B13Cu,
    kWebSearchEncryptionWordsMaxLen = 5,
};

#pragma pack(push, 1)

// ������ԴУ��αЭ��ͷ
struct PseudoProtoHead {
    char            encry_words[kWebSearchEncryptionWordsMaxLen];
    unsigned int    random_num;
    unsigned int    crc_check_sum;

    PseudoProtoHead()
    {
        // ͷ���ַ���
        const char *string = "SOSO";
        memcpy(encry_words, string, strlen(string) + 1);
    }

    void ToNetOrder()
    {
        random_num    = htonl(random_num);
        crc_check_sum = htonl(crc_check_sum);
    }

    void ToHostOrder()
    {
        random_num    = ntohl(random_num);
        crc_check_sum = ntohl(crc_check_sum);
    }
};

#pragma pack(pop)

// �������ԴУ��αЭ��ͷУ����
class BaseEncryption
{
public:
    static void Init()
    {
        srand(time(NULL));
    }

    // У��αЭ��ͷ
    // buffer��Ӧ����ͷ�����ݣ����ȹ̶�
    static bool IsValidPseudoProHead(void *data)
    {
        PseudoProtoHead *pseudo_head =
            reinterpret_cast<PseudoProtoHead *>(data);

        pseudo_head->ToHostOrder();

        bool ret = pseudo_head->crc_check_sum ==
                    CRC32Hash32(/*kWebSearchEncryptionKey, */
                    data, sizeof(pseudo_head->encry_words) +
                    sizeof(uint32_t)) ? true : false;

        pseudo_head->ToNetOrder();

        return ret;
    }

    // ����αЭ��ͷ���ݣ��ⲿ�ṩbuffer:���ɵ�αЭ��ͷ��Ϊ�������ǰ׺һͬ����
    static void SetPseudoProHead(PseudoProtoHead *pseudo_head)
    {
        pseudo_head->random_num = rand();

        pseudo_head->crc_check_sum =
            CRC32Hash32(/*kWebSearchEncryptionKey, */
            pseudo_head, sizeof(pseudo_head->encry_words) + sizeof(int32_t));

        pseudo_head->ToNetOrder();
    }

private:
    BaseEncryption();
    BaseEncryption(const BaseEncryption &rhs);

    ~BaseEncryption();

    // return *this just to supress the warnning below
    // warning: no return statement in function returning non-void
    BaseEncryption & operator = (const BaseEncryption &rhs) { return *this; }
};

#endif // COMMON_BASE_PROTOCLO_ENCRYPTION_H_ // NOLINT
