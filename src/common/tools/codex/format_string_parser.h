// Copyright 2010 Tencent Inc.
// Author: Yi Wang (yiwang@tencet.com)
//
// Parse the format string supported by codex command flags --record and --bin.

#ifndef CODEX_FORMAT_STRING_PARSER_H_
#define CODEX_FORMAT_STRING_PARSER_H_

#include <string>

#include "common/tools/codex/format_options.pb.h"

namespace codex {

// Parse a format string consisting of a sequence of options, where
// each option consisting of the following parts:
//  - separation: a string between two options, before the first
//      option, or after the last one.
//  - prefix: a character that will be inserted before printed option
//      to fill the specified width of the printed option.
//  - width: a numerical value specify the width of the printed option.
//  - precision: '.' + a numerical value used to specified the number of
//      digits after the original point when we print a floating point value.
//  - decorator: character 'h' denoting ``half'' or 'l' denoting ``long''.
//      The decorator works when coupled with certain conversions:
//       - "lln" or "lld" means 64bit  integer value
//       - "ln"  or "ld"  means 32bit  integer value
//       - "n"   or "d"   means 32bit  integer value
//       - "hn"  or "hd"  means 16bit  integer value
//       - "hhn" or "hhd" means 8bit   integer value
//       - "f"  means a 32bit single-precision float point value
//       - "lf" means a 64bit double-precision float point value
//  - conversion: a character:
//      'c' : an (8bit) character
//      'n' : a network integer
//      'd' : a architecture-dependent integer
//      'f' : a float point value
//      's' : a string
//      'P' : human-reable representation of a protocal message
//      '%' : the character '%'
//  - attachment: a string enclosed in '{' and '}', used to denote the
//      name of a protocol message when conversion is 'P'.
//
bool ParseFormatOptions(const std::string& format,
                        FormatOptionSequence* options);

}  // namespace codex

#endif  // CODEX_FORMAT_STRING_PARSER_H_