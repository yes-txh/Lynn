/* MD5.H - header file for MD5C.C
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
   rights reserved.

   License to copy and use this software is granted provided that it
   is identified as the "RSA Data Security, Inc. MD5 Message-Digest
   Algorithm" in all material mentioning or referencing this software
   or this function.

   License is also granted to make and use derivative works provided
   that such works are identified as "derived from the RSA Data
   Security, Inc. MD5 Message-Digest Algorithm" in all material
   mentioning or referencing the derived work.

   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.

   These notices must be retained in any copies of any part of this
   documentation and/or software.
 */
#ifndef _RSA_MD5_H_
#define _RSA_MD5_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

    /* MD5 context. */
    typedef struct
    {
        UINT4 state[4];                                   /* state (ABCD) */
        UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
        unsigned char buffer[64];                         /* input buffer */
    } MD5_CTX_rsa, MD5Context_RSA, HASH_MD5_CTX;


    void MD5Init PROTO_LIST((MD5_CTX_rsa*));
    void MD5Update PROTO_LIST((MD5_CTX_rsa*, unsigned char*, unsigned int));
    void MD5Final PROTO_LIST((unsigned char [16], MD5_CTX_rsa*));


#define Hash_MD5Init    MD5Init
#define Hash_MD5Update  MD5Update
#define Hash_MD5Final   MD5Final

    void Hash_Md5HashBuffer(unsigned char* outBuffer, const unsigned char* inBuffer, int length);

#ifdef __cplusplus
}
#endif
#endif
