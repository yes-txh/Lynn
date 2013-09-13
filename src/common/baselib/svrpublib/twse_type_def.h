/************************************************************
  Copyright (C), 1998-2006, Tencent Technology Commpany Limited

  文件名称: twse_type_def.h
  作者: swordshao
  日期: 2006.12.12
  版本: 1.0
  模块描述: 平台无关的数据类型定义
            每个数据类型的定义以T开头，T表示类型(Type)的意思。

  修改历史:
      <author>    <time>   <version >   <desc>
     swordshao  2006.12.12    1.0        创建
*************************************************************/

#ifndef COMMON_BASELIB_SVRPUBLIB_TWSE_TYPE_DEF_H_
#define COMMON_BASELIB_SVRPUBLIB_TWSE_TYPE_DEF_H_

//
// VC工程使用UNICODE字符集的时候,
// CHAR将和windows include下其定义的宽字节CHAR冲突，
// 使用多字节字符集可避免这个冲突.
//
// wookin, 2010-06-04
//

// TODO(wookin) : xxx
// 使用下面这些类型

// int16_t
// uint16_t
// int32_t
// uint32_t
// int64_t
// uint64_t

#ifndef WIN32 // linux
#include <stdint.h>
#else // WIN32
#include "common/baselib/svrpublib/vc_stdint.h"
#endif //

// printf int64_t, uint64_t
// printf("val = %"FMTd64, (int64_t)val);


//
// ---start define FMTd64, FMTu64---
//
#ifndef FMTd64

#ifdef WIN32
// ---START of win32---
#define FMTd64    "lld"
#define FMTu64    "llu"
#define FMTsize   "u"
// ---END of win32---
#else
// ---START of linux---
// m64
#if (__WORDSIZE == 64)
#define FMTd64    "ld"
#define FMTu64    "lu"
#define FMTsize   FMTu64
#else
// m32
#define FMTd64    "lld"
#define FMTu64    "llu"
#define FMTsize   "u"
#endif //
// ---END of linux---
#endif // linux

#endif // !FMTd64
// ---end define FMTd64, FMTu64---

#endif // COMMON_BASELIB_SVRPUBLIB_TWSE_TYPE_DEF_H_
