// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASE_BIT_MASK_ENUM_HPP
#define COMMON_BASE_BIT_MASK_ENUM_HPP

/*
程序中经常需要用枚举来定义位掩码，比起 #define 当然好很多，比起 const 则有类型
方面的安全性，位掩码常常组合后使用，但是 C++ 中一旦对枚举进行了位运算，结果就退
化为整形，失去了原来的类型，使得接口中的位掩码不得不定义为 int，失去了类型安全
性。

C# 的枚举支持 Flag 属性，Flag 枚举位运算的结果还是枚举类型，很好。

在 C++ 中可以利用运算符重载可以解决这个问题，使得位掩码枚举进行与或运算后还能保
持原来的类型。
*/

// GLOBAL_NOLINT(runtime/int)

#define DEFINE_BIT_MASK_ENUM_OPERATIONS(Type) \
inline Type EnumBitMaskOr(Type lhs, Type rhs) { return lhs | rhs; } \
inline Type EnumBitMaskAnd(Type lhs, Type rhs) { return lhs & rhs; }

DEFINE_BIT_MASK_ENUM_OPERATIONS(int)
DEFINE_BIT_MASK_ENUM_OPERATIONS(unsigned int)
DEFINE_BIT_MASK_ENUM_OPERATIONS(long)
DEFINE_BIT_MASK_ENUM_OPERATIONS(unsigned long)
DEFINE_BIT_MASK_ENUM_OPERATIONS(long long)
DEFINE_BIT_MASK_ENUM_OPERATIONS(unsigned long long)

#undef DEFINE_BIT_MASK_ENUM_OPERATIONS

#define DEFINE_BIT_MASK_ENUM(Type) \
inline Type operator|(Type lhs, Type rhs) \
{ \
    return Type(EnumBitMaskOr(lhs, rhs)); \
} \
inline Type operator&(Type lhs, Type rhs) \
{ \
    return Type(EnumBitMaskAnd(lhs, rhs)); \
} \
inline Type& operator|=(Type& lhs, Type rhs) \
{ \
    lhs = lhs | rhs; \
    return lhs; \
} \
inline Type& operator&=(Type& lhs, Type rhs) \
{ \
    lhs = lhs & rhs; \
    return lhs; \
}

//////////////////////////////////////////////////////////////////////////////
// demo

#if 0
enum FileOpenFlags
{
    FileOpenFlags_Create = 1,
    FileOpenFlags_Truncate = 2,
};

DEFINE_BIT_MASK_ENUM(FileOpenFlags);

FileOpenFlags DefaultFileOpenFlags =
    FileOpenFlags_Create | FileOpenFlags_Truncate;

int main()
{
    FileOpenFlags flags;
    flags |= FileOpenFlags_Create;
}
#endif

#endif // COMMON_BASE_BIT_MASK_ENUM_HPP

