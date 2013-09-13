// sha1.h
// wookin@tencent
//
/*
*    sha1.h
*
*    Copyright (C) 1998
*    Paul E. Jones <paulej@arid.us>
*    All Rights Reserved.
*
*****************************************************************************
*    $Id: sha1.h, v 1.6 2004/03/27 18:02:26 paulej Exp $
*****************************************************************************
*
*    Description:
*         This class implements the Secure Hashing Standard as defined
*         in FIPS PUB 180-1 published April 17, 1995.
*
*        Many of the variable names in this class, especially the single
*        character names, were used because those were the names used
*        in the publication.
*
*         Please read the file sha1.cpp for more information.
*
*/
//////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_SHA1_H_
#define COMMON_BASELIB_SVRPUBLIB_SHA1_H_

#include "common/baselib/svrpublib/twse_type_def.h"

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

class SHA1 {
public:

    SHA1();
    virtual ~SHA1();

    /*
    *    Re-initialize the class
    */
    void Reset();

    /*
    *    Returns the message digest
    */
    bool Result(uint32_t*    message_digest_array);

    /*
    *    Provide input to SHA1
    */
    void Input(const unsigned char* message_array,
               uint32_t length);
    void Input(const char* message_array,
               uint32_t length);
    void Input(unsigned char message_element);
    void Input(char message_element);
    SHA1& operator<<(const char *message_array);
    SHA1& operator<<(const unsigned char *message_array);
    SHA1& operator<<(const char message_element);
    SHA1& operator<<(const unsigned char message_element);

private:

    /*
    *    Process the next 512 bits of the message
        */
    void ProcessMessageBlock();

    /*
    *    Pads the current message block to 512 bits
    */
    void PadMessage();

    /*
    *    Performs a circular left shift operation
    */
    inline uint32_t CircularShift(int32_t bits, uint32_t word) const;

    uint32_t        m_h[5];                 // Message digest buffers

    uint32_t        m_length_low;           // Message length in bits
    uint32_t        m_length_high;          // Message length in bits

    unsigned char   m_message_block[64];    // 512-bit message blocks
    int32_t         m_message_block_index;  // Index into message block array

    bool            m_computed;             // Is the digest computed ?
    bool            m_corrupted;            // Is the message digest corruped ?
};

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_SHA1_H_

