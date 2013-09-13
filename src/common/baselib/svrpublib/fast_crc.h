#ifndef COMMON_BASELIB_SVRPUBLIB_FASTCRC32_H_
#define COMMON_BASELIB_SVRPUBLIB_FASTCRC32_H_
#include "common/baselib/svrpublib/twse_type_def.h"
#include "common/baselib/svrpublib/base_config.h"
_START_XFS_BASE_NAMESPACE_

// FastCRC32可以对字符串进行累加计算crc，在编译器请对该文件开启o2优化
// old_crc分解为4个byte
// |---byte0----|---byte1----|---byte2----|---byte3----|
// byte0记录字符串的第0，3，6,......3n个字符的crc8累加结果
// byte1记录字符串的第1，4，7,......3n+1个字符crc8累加结果
// byte2都是字符串的第2，5，8,......3n+2个字符crc8累加结果
// byte3的格式为|--bit0--|--bit1--|--bit2--|--bit3--|--bit4--|--bit5--|--bit6--|--bit7--|
// bit0-bit1用来记录下次要累加的起始位置
// bit2-bit4用来记录当前CRC32的版本号
// bit5-bit7空闲
uint32_t FastCRC(uint32_t old_crc, const unsigned char* dat, uint32_t len);

_END_XFS_BASE_NAMESPACE_
#endif // COMMON_BASELIB_SVRPUBLIB_FASTCRC32_H_
