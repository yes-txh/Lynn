// Copyright (c) 2010, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_BASE_PREPROCESS_H
#define COMMON_BASE_PREPROCESS_H

/// Converts the parameter X to a string after macro replacement
/// on X has been performed.
/// example: PP_STRINGIZE(UCHAR_MAX) -> "255"
#define PP_STRINGIZE(X) PP_DO_STRINGIZE(X)
#define PP_DO_STRINGIZE(X) #X

/// Helper macro to join 2 tokens
/// example: PP_JOIN(UCHAR_MAX, SCHAR_MIN) -> 255(-128)
/// The following piece of macro magic joins the two
/// arguments together, even when one of the arguments is
/// itself a macro (see 16.3.1 in C++ standard). The key
/// is that macro expansion of macro arguments does not
/// occur in PP_DO_JOIN2 but does in PP_DO_JOIN.
#define PP_JOIN(X, Y) PP_DO_JOIN(X, Y)
#define PP_DO_JOIN(X, Y) PP_DO_JOIN2(X, Y)
#define PP_DO_JOIN2(X, Y) X##Y

/// prevent macro substitution for function-like macros
/// if macro 'min()' was defined:
/// 'int min()' whill be substituted, but
/// 'int min PP_PREVENT_MACRO_SUBSTITUTION()' will not be substituted.
#define PP_PREVENT_MACRO_SUBSTITUTION

#endif // COMMON_BASE_PREPROCESS_H

