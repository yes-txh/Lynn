/* R_STDLIB.C - platform-specific C library routines for RSAREF
 */

/* Copyright (C) RSA Laboratories, a division of RSA Data Security,
     Inc., created 1991. All rights reserved.
 */
extern "C" {
#include <string.h>
#include "common/crypto/rsa2/rsa2_global.h"
#include "common/crypto/rsa2/rsa2_ref/rsa_source/rsa_ref.h"

    void R_memset( //output, value, len)
        POINTER output,                                             /* output block */
        int value,                                                         /* value */
        unsigned int len)                                        /* length of block */
    {
        if (len)
            memset(output, value, len);
    }

    void R_memcpy( //output, input, len)
        POINTER output,                                             /* output block */
        POINTER input,                                               /* input block */
        unsigned int len)                                       /* length of blocks */
    {
        if (len)
            memcpy(output, input, len);
    }

    int R_memcmp( //firstBlock, secondBlock, len)
        POINTER firstBlock,                                          /* first block */
        POINTER secondBlock,                                        /* second block */
        unsigned int len)                                       /* length of blocks */
    {
        if (len)
            return (memcmp(firstBlock, secondBlock, len));
        else
            return (0);
    }
}
