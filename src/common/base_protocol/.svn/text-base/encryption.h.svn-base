/***********************************************************************
网络通信协议来源校验
{"指定字符串"}{随机数(无符号整型)}{CRC32(指定字符串+随机数)}

其中指定字符串为"SOSO" 5字节
随机数                   4字节
CRC32                   4字节
作为网络包的前缀用来做来源校验
***********************************************************************/

//////////////////////////////////////////////////////////////////////////
// modified by ivanhuang at 20101104
// 1.修改代码风格
// 2.增加UnitTest
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

// 数据来源校验伪协议头
struct PseudoProtoHead {
    char            encry_words[kWebSearchEncryptionWordsMaxLen];
    unsigned int    random_num;
    unsigned int    crc_check_sum;

    PseudoProtoHead()
    {
        // 头部字符串
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

// 网络包来源校验伪协议头校验类
class BaseEncryption
{
public:
    static void Init()
    {
        srand(time(NULL));
    }

    // 校验伪协议头
    // buffer对应的是头部数据，当度固定
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

    // 生成伪协议头数据，外部提供buffer:生成的伪协议头作为网络包的前缀一同发送
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
