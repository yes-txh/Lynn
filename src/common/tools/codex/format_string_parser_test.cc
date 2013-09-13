// Copyright 2010 Tencent Inc.
// Author: Yi Wang (yiwang@tencet.com)

#include "common/tools/codex/format_string_parser.h"

#include "gtest/gtest.h"

TEST(FormatStringParserTest, AGeneralCase) {
  codex::FormatOptionSequence options;
  EXPECT_TRUE(codex::ParseFormatOptions(
      "key=%05hhd%010.10lf, value=%hn %P{mr.KeyValuePB}lalala.",
      &options));
  EXPECT_EQ(5, options.option_size());

  EXPECT_EQ("key=", options.option(0).separation());
  EXPECT_EQ("0", options.option(0).prefix());
  EXPECT_EQ("5", options.option(0).width());
  EXPECT_EQ("", options.option(0).precision());
  EXPECT_EQ("hh", options.option(0).decorator());
  EXPECT_EQ("d", options.option(0).conversion());
  EXPECT_EQ("", options.option(0).attachment());

  EXPECT_EQ("", options.option(1).separation());
  EXPECT_EQ("0", options.option(1).prefix());
  EXPECT_EQ("10", options.option(1).width());
  EXPECT_EQ(".10", options.option(1).precision());
  EXPECT_EQ("l", options.option(1).decorator());
  EXPECT_EQ("f", options.option(1).conversion());
  EXPECT_EQ("", options.option(1).attachment());

  EXPECT_EQ(", value=", options.option(2).separation());
  EXPECT_EQ("", options.option(2).prefix());
  EXPECT_EQ("", options.option(2).width());
  EXPECT_EQ("", options.option(2).precision());
  EXPECT_EQ("h", options.option(2).decorator());
  EXPECT_EQ("n", options.option(2).conversion());
  EXPECT_EQ("", options.option(2).attachment());

  EXPECT_EQ(" ", options.option(3).separation());
  EXPECT_EQ("", options.option(3).prefix());
  EXPECT_EQ("", options.option(3).width());
  EXPECT_EQ("", options.option(3).precision());
  EXPECT_EQ("", options.option(3).decorator());
  EXPECT_EQ("P", options.option(3).conversion());
  EXPECT_EQ("{mr.KeyValuePB}", options.option(3).attachment());

  EXPECT_EQ("lalala.", options.option(4).separation());
  EXPECT_EQ("", options.option(4).prefix());
  EXPECT_EQ("", options.option(4).width());
  EXPECT_EQ("", options.option(4).precision());
  EXPECT_EQ("", options.option(4).decorator());
  EXPECT_EQ("", options.option(4).conversion());
  EXPECT_EQ("", options.option(4).attachment());
}

TEST(FormatStringParserTest, SeparationThenOption) {
  codex::FormatOptionSequence options;
  EXPECT_TRUE(codex::ParseFormatOptions(
      "key=%05d",
      &options));
  EXPECT_EQ(1, options.option_size());

  EXPECT_EQ("key=", options.option(0).separation());
  EXPECT_EQ("0", options.option(0).prefix());
  EXPECT_EQ("5", options.option(0).width());
  EXPECT_EQ("", options.option(0).precision());
  EXPECT_EQ("", options.option(0).decorator());
  EXPECT_EQ("d", options.option(0).conversion());
  EXPECT_EQ("", options.option(0).attachment());
}

TEST(FormatStringParserTest, OptionThenSeparation) {
  codex::FormatOptionSequence options;
  EXPECT_TRUE(codex::ParseFormatOptions(
      "%05d.\n",
      &options));
  EXPECT_EQ(2, options.option_size());

  EXPECT_EQ("", options.option(0).separation());
  EXPECT_EQ("0", options.option(0).prefix());
  EXPECT_EQ("5", options.option(0).width());
  EXPECT_EQ("", options.option(0).precision());
  EXPECT_EQ("", options.option(0).decorator());
  EXPECT_EQ("d", options.option(0).conversion());
  EXPECT_EQ("", options.option(0).attachment());

  EXPECT_EQ(".\n", options.option(1).separation());
  EXPECT_EQ("", options.option(1).prefix());
  EXPECT_EQ("", options.option(1).width());
  EXPECT_EQ("", options.option(1).precision());
  EXPECT_EQ("", options.option(1).decorator());
  EXPECT_EQ("", options.option(1).conversion());
  EXPECT_EQ("", options.option(1).attachment());
}

TEST(FormatStringParserTest, UnknownDecorator) {
  codex::FormatOptionSequence options;

  EXPECT_FALSE(codex::ParseFormatOptions(
      "%hld", &options));
  EXPECT_FALSE(codex::ParseFormatOptions(
      "%lhd", &options));
  EXPECT_FALSE(codex::ParseFormatOptions(
      "%hhhd", &options));
  EXPECT_FALSE(codex::ParseFormatOptions(
      "%llld", &options));

  EXPECT_TRUE(codex::ParseFormatOptions(
      "%hhd", &options));
  EXPECT_EQ(1, options.option_size());
  EXPECT_EQ("hh", options.option(0).decorator());

  EXPECT_TRUE(codex::ParseFormatOptions(
      "%hd", &options));
  EXPECT_EQ(1, options.option_size());
  EXPECT_EQ("h", options.option(0).decorator());

  EXPECT_TRUE(codex::ParseFormatOptions(
      "%d", &options));
  EXPECT_EQ(1, options.option_size());
  EXPECT_EQ("", options.option(0).decorator());

  EXPECT_TRUE(codex::ParseFormatOptions(
      "%ld", &options));
  EXPECT_EQ(1, options.option_size());
  EXPECT_EQ("l", options.option(0).decorator());

  EXPECT_TRUE(codex::ParseFormatOptions(
      "%lld", &options));
  EXPECT_EQ(1, options.option_size());
  EXPECT_EQ("ll", options.option(0).decorator());
}
