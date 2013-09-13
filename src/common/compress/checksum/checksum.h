/**
 * @file checksum.h
 * @brief
 * @author kypoyin
 * @date 2010-04-29
 */

#ifndef COMMON_COMPRESS_CHECKSUM_H
#define COMMON_COMPRESS_CHECKSUM_H

#include <stdlib.h>
#include "common/base/stdint.h"

namespace intern
{

/** Compute fletcher32 checksum for arbitary data
 *
 * @param data - input data
 * @param len - input data length in bytes
 */
extern uint32_t
fletcher32(const void *data, size_t len);

/** Compute fletcher32 checksum for 16-bit aligned and padded data
 *  slightly faster than fletcher32
 *
 * @param data - input data
 * @param len - input data length in bytes
 */
extern uint32_t
fletcher32a(const uint16_t *data, size_t len);

/** Compute adler32 checksum
 *
 * @param data - input data
 * @param len - input data length in bytes
 */
extern uint32_t
adler32(const void *data, size_t len);

/** Update adler32 checksum incrementally
 *
 * @param adler - current adler32 checksum
 * @param data - input data
 * @param len - input data length in bytes
 */
extern uint32_t
adler32_update(uint32_t adler, const void *data, size_t len);

/** Compute crc32 checksum
 *
 * @param data - input data
 * @param len - input data length in bytes
 */
extern uint32_t
crc32(const void *data, size_t len);

/** Update crc32 checksum incrementally
 *
 * @param crc - current crc32 checksum
 * @param data - input data
 * @param len - input data length in bytes
 */
extern uint32_t
crc32_update(uint32_t crc, const void *data, size_t len);

} /// end namespace intern 

#endif // COMMON_COMPRESS_CHECKSUM_H
