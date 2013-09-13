// Copyright 2010 Tencent Inc.
// Author: Yi Wang (yiwang@tencet.com)

#include "common/tools/codex/proto_message_creator.h"

#include <string>
#include <vector>

#include "common/tools/codex/format_option.pb.h"
#include "common/tools/codex/format_options.pb.h"
#include "gtest/gtest.h"
#include "protobuf/compiler/parser.h"
#include "protobuf/descriptor.h"
#include "protobuf/descriptor_database.h"
#include "protobuf/dynamic_message.h"
#include "protobuf/io/tokenizer.h"
#include "protobuf/io/zero_copy_stream_impl.h"

// FIXME(phongchen): disable failed test
TEST(DISABLED_ProtoMessageCreatorTest, AGeneralCase) {
  using std::string;
  using std::vector;

  // Create a serialization of FormatOptionSequence for testing comparison.
  string serialization;
  codex::FormatOptionSequence option_sequence;
  option_sequence.add_option()->set_separation("key");
  option_sequence.SerializeToString(&serialization);

  // We try to parse real proto files in our project.  Note that we have two
  // .proto files: format_option.proto defining codex.FormatOption, and
  // format_options.proto defining codex.FormatOptionSequence, and
  // codex.FormatOptionSequence depends on codex.Format, and thus
  // format_options.proto has to import format_option.proto.  Here we parse
  // only format_options.proto, but not the imported file, format_option.proto.
  codex::ProtoMessageCreator creator;
  ASSERT_TRUE(creator.ParseProtoSourceFile("./format_options.proto"));

  google::protobuf::Message* msg =
      creator.CreateProtoMessage("codex.FormatOptionSequence");
  ASSERT_TRUE(msg->ParseFromString(serialization));
  EXPECT_EQ("option { separation: \"key\" }", msg->ShortDebugString());
}
