// Copyright 2010 Tencent Inc.
// Author: Yi Wang (yiwang@tencet.com)

#include <stdio.h>

#include <string>

#include "common/tools/codex/format_string_parser.h"
#include "common/tools/codex/format_option.pb.h"
#include "re2/re2.h"

namespace codex {

using std::string;

// This regular expression encodes a format option.  For details about
// the format of an option, please refer to comments in format_string_parser.h.
static const char* kFormatOptionsRegex =
    "([^%]*)"                           // anything that prefix an option
    "%"                                 // the start of an option or "%%"
    "([^%\\.hlcndfsP%]?)"               // nothing or a fill-in character
    "(\\d+)?"                           // nothing or few digits of width
    "(\\.\\d+)?"                        // nothing or precision
    "([hl]*)"                           // nothing or decorator
    "([cndfsP%])"                       // a (must-have) conversion
    "(\\{[\\.\\w]+\\})?";               // nothing or a proto message name


static bool CheckDecorator(const std::string& decorator) {
  if (decorator.size() > 0 &&
      decorator != "h"  &&
      decorator != "hh" &&
      decorator != "l"  &&
      decorator != "ll") {
    return false;
  }
  return true;
}

bool ParseFormatOptions(const std::string& format,
                        FormatOptionSequence* options) {
  static const RE2 format_option_pattern(kFormatOptionsRegex);

  re2::StringPiece input(format);
  options->Clear();

  FormatOption option;
  while (RE2::Consume(&input,
                      format_option_pattern,
                      option.mutable_separation(),
                      option.mutable_prefix(),
                      option.mutable_width(),
                      option.mutable_precision(),
                      option.mutable_decorator(),
                      option.mutable_conversion(),
                      option.mutable_attachment())) {
    if (!CheckDecorator(option.decorator())) {
      fprintf(stderr, "Unknown decorator : %s\n", option.decorator().c_str());
      return false;
    }
    options->add_option()->CopyFrom(option);
  }

  if (input.size() > 0) {
    // Append the rest (imparsable part) of the input format string as
    // the separation of a new option.
    options->add_option()->set_separation(input.as_string());
  }

  return true;
}

}  // namespace codex
