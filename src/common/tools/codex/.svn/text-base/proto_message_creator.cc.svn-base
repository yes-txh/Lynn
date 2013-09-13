// Copyright 2010 Tencent Inc.
// Author: Yi Wang (yiwang@tencent.com)

#include "common/tools/codex/proto_message_creator.h"

#include <stdio.h>
#include <unistd.h>                     // for get_current_dir_name()

#include <string>
#include <vector>

namespace codex {

using std::string;
using std::vector;


ProtoMessageCreator::ProtoMessageCreator()
    : source_tree_(),
      compiler_(&source_tree_, NULL),
      factory_(compiler_.pool()) {}


ProtoMessageCreator::~ProtoMessageCreator() {}


void ProtoMessageCreator::AddImportPath(const string& import_path) {
  source_tree_.MapPath("", import_path.c_str());
}


bool ProtoMessageCreator::ParseProtoSourceFile(const string& proto_filename) {
  // Check if we can read the proto file.
  FILE* proto_file = fopen(proto_filename.c_str(), "r");
  if (proto_file == NULL) {
    fprintf(stderr, "Cannot open .proto file: %s\n", proto_filename.c_str());
    return false;
  }
  fclose(proto_file);

  // Find out the directory and add it to source tree, so we can find
  // imported proto files in the source tree.
  size_t slash_pos = proto_filename.find_last_of("/\\");
  if (slash_pos != string::npos) {
    source_tree_.MapPath("", proto_filename.substr(0, slash_pos + 1));
  } else {
    source_tree_.MapPath("", get_current_dir_name());
  }

  // The name Importer::Import is kind of misleanding.  It can be
  // renamed "compile" or "build".
  string basename(proto_filename.substr(slash_pos + 1));
  if (compiler_.Import(basename) == NULL) {
    fprintf(stderr, "Failed compiling proto file: %s\n", basename.c_str());
    return false;
  }

  return true;
}


google::protobuf::Message* ProtoMessageCreator::CreateProtoMessage(
    const string& message_name) {
  const google::protobuf::Descriptor* message_desc =
      compiler_.pool()->FindMessageTypeByName(message_name);
  if (message_desc == NULL) {
    fprintf(stderr, "Cannot find symbol: %s in given proto files.\n",
            message_name.c_str());
    return NULL;
  }

  const google::protobuf::Message* prototype_msg =
      factory_.GetPrototype(message_desc);
  if (prototype_msg == NULL) {
    fprintf(stderr, "Cannot create prototype message: %s\n",
            message_name.c_str());
    return NULL;
  }

  return prototype_msg->New();
}

}  // namespace codex
