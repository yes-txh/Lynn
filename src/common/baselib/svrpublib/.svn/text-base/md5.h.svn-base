// md5.h
// md5 hash
// ////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_MD5_H_
#define COMMON_BASELIB_SVRPUBLIB_MD5_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

#ifdef __cplusplus
// extern "C"
// {
#endif

    struct MD5Context {
        long buf[4];
        long bits[2];
        unsigned char in[64];
    };

    void Hash_MD5Init(struct MD5Context *context);
    void Hash_MD5Update(struct MD5Context *context, unsigned char const *buf,
                        unsigned len);
    void Hash_MD5Final(struct MD5Context *context, unsigned char *digest);
    void Hash_MD5Transform(long buf[4], long const in[16]);


// * This is needed to make RSAREF happy on some MS-DOS compilers.
    typedef struct MD5Context MD5_CTX;

    void Md5HashBuffer(unsigned char *outBuffer, const unsigned char *inBuffer,
                       int32_t length);

#ifdef __cplusplus
// }
#endif

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_MD5_H_
