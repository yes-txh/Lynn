// RSATest.cpp : Defines the entry point for the console application.
//

// #include "ServerPubLib.h"

#include "common/crypto/rsa2/rsa2.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


int32_t test_PublicEncrypt_PrivateDecrypt(CRSACrypt* rsa_obj);
int32_t test_PrivateEncrypt_PublicDecrypt(CRSACrypt* rsa_obj);
int32_t test_SetKet(CRSACrypt* rsa_obj);

int32_t main(int32_t argc, char* argv[])
{
    // 初始化随机数
    CRSACrypt rsa_obj;
    srand((uint32_t)time(0));
    rsa_obj.InitRandomStruct((int32_t)rand());

    // 产生公钥和私钥的键值对
    rsa_obj.GenerateKeyPair(500);  // 参数大于96，小于1024

    // 显示公钥和私钥的内容
    char* buf = new char[2048];
    char crlf[16] = "\r\n";
    memset(buf, 0, 2048);
    rsa_obj.WritePublicKeyDesc(buf, crlf);
    printf("%s\r\n", buf);

    printf("\r\n");

    memset(buf, 0, 2048);
    rsa_obj.WritePrivateKeyDesc(buf, crlf);
    printf("%s\r\n", buf);
    printf("\r\n");


    delete []buf;

    // -------------------------------------
    // case1:
    // 用公钥加密一个文件，再用私钥来解密
    printf("***************case1 start**************\r\n");

    if (!test_PublicEncrypt_PrivateDecrypt(&rsa_obj))
        printf("case1: PublicEncrypt and PrivateDecrypt OK\r\n");
    else
        printf("case1: PublicEncrypt and PrivateDecrypt fail\r\n");

    printf("***************case1 finish**************\r\n");
    printf("\r\n");

    // -------------------------------------
    // case2:
    // 用私钥加密一个文件，再用公钥来解密
    printf("***************case2 start**************\r\n");

    if (!test_PrivateEncrypt_PublicDecrypt(&rsa_obj))
        printf("case2: PrivateEncrypt and PublicDecrypt OK\r\n");
    else
        printf("case2: PrivateEncrypt and PublicDecrypt fail\r\n");

    printf("***************case2 finish**************\r\n");
    printf("\r\n");


    return 0;
}




// 用公钥加密一个文件，再用私钥来解密
int32_t test_PublicEncrypt_PrivateDecrypt(CRSACrypt* rsa_obj)
{
    unsigned char case_str[1024] = "case: this is a PublicEncrypt_PrivateDecrypt test for rsa encryption algorithm";
    unsigned char src_data[1024] = {0};
    memcpy(src_data, case_str, 1024);
    uint32_t   src_data_len = strlen(reinterpret_cast<char*>(src_data));
    unsigned char temp_data[1024] = {0};
    uint32_t temp_data_len = src_data_len;
    memcpy(temp_data, src_data, src_data_len);
    unsigned char dest_data[1024] = {0};
    uint32_t dest_data_len = 0;
    printf("the original string is: %s\r\n", src_data);
    printf("the original len is: %d\r\n", src_data_len);

    // 公钥加密
    rsa_obj->PublicEncrypt(src_data,
                           src_data_len,
                           dest_data,
                           &dest_data_len);
    printf("After PublicEncrypt, the string is: %s\r\n", dest_data);
    printf("After PublicEncrypt, the len is: %d\r\n", dest_data_len);

    memset(src_data, 0, 1024);

    // 私钥解密
    rsa_obj->PrivateDecrypt(dest_data,
                            dest_data_len,
                            src_data,
                            &src_data_len);
    printf("After PrivateDecrypt, the string is: %s\r\n", src_data);
    printf("After PrivateDecrypt, the len is: %d\r\n", src_data_len);

    // 校验
    int32_t cmp_result = memcmp(temp_data, src_data, src_data_len);

    if ((!cmp_result) && (temp_data_len == src_data_len))
        return 0;
    else
        return -1;
}


// 用私钥加密一个文件，再用公钥来解密
int32_t test_PrivateEncrypt_PublicDecrypt(CRSACrypt* rsa_obj)
{
    unsigned char case_str[1024] = "case: this is a PrivateEncrypt_PublicDecrypt test for rsa encryption algorithm";
    unsigned char src_data[1024] = {0};
    memcpy(src_data, case_str, 1024);
    uint32_t   src_data_len = strlen(reinterpret_cast<char*>(src_data));
    unsigned char temp_data[1024] = {0};
    uint32_t temp_data_len = src_data_len;
    memcpy(temp_data, src_data, src_data_len);
    unsigned char dest_data[1024] = {0};
    uint32_t dest_data_len = 0;
    printf("the original string is: %s\r\n", src_data);
    printf("the original len is: %d\r\n", src_data_len);

    // 私钥加密
    rsa_obj->PrivateEncrypt(src_data,
                            src_data_len,
                            dest_data,
                            &dest_data_len);
    printf("After PrivateEncrypt, the string is: %s\r\n", dest_data);
    printf("After PrivateEncrypt, the len is: %d\r\n", dest_data_len);

    memset(src_data, 0, 1024);

    // 公钥解密
    rsa_obj->PublicDecrypt(dest_data,
                           dest_data_len,
                           src_data,
                           &src_data_len);
    printf("After PublicDecrypt, the string is: %s\r\n", src_data);
    printf("After PublicDecrypt, the len is: %d\r\n", src_data_len);

    // 校验
    int32_t cmp_result = memcmp(temp_data, src_data, src_data_len);

    if ((!cmp_result) && (temp_data_len == src_data_len))
        return 0;
    else
        return -1;

}
