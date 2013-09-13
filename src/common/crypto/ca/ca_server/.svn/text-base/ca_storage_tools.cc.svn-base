// Copyright 2011, Tencent Inc.
// Author: Chengyu Dong (moraindong@tencent.com)
//
// Tools to set ca storage info.

#include <fstream>
#include <ostream>
#include <iostream>
#include "common/base/module.hpp"
#include "common/base/string/string_number.hpp"
#include "common/crypto/ca/ca_server/ca_storage.pb.h"
#include "common/file/recordio/recordio.h"
#include "glog/logging.h"
#include "thirdparty/gflags/gflags.h"

using namespace ca;

DEFINE_string(read_quota_list, "", "Read quota file list");
DEFINE_string(write_quota_list, "", "Write quota file list");
DEFINE_string(write_role_list, "", "Write role file list");
DEFINE_string(xfs_cluster_name, "", "XFS cluster name");
DEFINE_uint64(default_quota, 100, "default quota value");
// set 10 or less roles as onebox needs
DEFINE_string(role0, "role_0", "role0's name");
DEFINE_string(role1, "role_1", "role1's name");
DEFINE_string(role2, "role_2", "role2's name");
DEFINE_string(role3, "role_3", "role3's name");
DEFINE_string(role4, "role_4", "role4's name");
DEFINE_string(role5, "role_5", "role5's name");
DEFINE_string(role6, "role_6", "role6's name");
DEFINE_string(role7, "role_7", "role7's name");
DEFINE_string(role8, "role_8", "role8's name");
DEFINE_string(role9, "role_9", "role9's name");
DEFINE_string(role10, "role_10", "role10's name");
DEFINE_string(role11, "role_11", "role11's name");
DEFINE_string(role12, "role_12", "role12's name");
DEFINE_string(role13, "role_13", "role13's name");
DEFINE_string(role14, "role_14", "role14's name");
DEFINE_string(role15, "role_15", "role15's name");
DEFINE_string(role16, "role_16", "role16's name");
DEFINE_string(role17, "role_17", "role17's name");
DEFINE_string(role18, "role_18", "role18's name");
DEFINE_string(role19, "role_19", "role19's name");

void ReadFileList() {
    std::ifstream* fstream = new std::ifstream();
    fstream->open(FLAGS_read_quota_list.c_str(), std::ios::in | std::ios::binary);
    if (fstream->fail()) {
        LOG(ERROR) << "fail to open file to read: " << FLAGS_read_quota_list;
        delete fstream;
        return ;
    }

    RecordReader reader(fstream, RecordReaderOptions(RecordReaderOptions::OWN_STREAM));

    QuotaList quota_list;
    while (reader.ReadMessage(&quota_list)) {
        LOG(INFO) << quota_list.DebugString();
    }
}

#define ADD_RECORD(i) \
        role_record = role_list.add_role(); \
        role_record->set_id(i+1); \
        role_record->set_valid(true); \
        quota_record = quota_list.add_quota(); \
        quota_record->set_rid(i+1); \
        cluster_name = FLAGS_xfs_cluster_name; \
        quota_record->set_cluster(cluster_name); \
        quota_record->set_num_chunks(quota_value); \
        quota_record->set_num_files(quota_value); \
        quota_record->set_num_dirs(quota_value);

void WriteFileList(const uint32_t quota_value) {
    RecordWriter writer_quota(new std::ofstream(FLAGS_write_quota_list.c_str(),
                                                std::ios::out | std::ios::binary),
                              RecordWriterOptions(RecordWriterOptions::OWN_STREAM));
    RecordWriter writer_role(new std::ofstream(FLAGS_write_role_list.c_str(),
                                               std::ios::out | std::ios::binary),
                             RecordWriterOptions(RecordWriterOptions::OWN_STREAM));
 
    // Fill in data.
    QuotaList quota_list;
    RoleList  role_list;
    IdNameRecord *role_record;
    QuotaStorageRecord *quota_record;
    std::string cluster_name;
    ADD_RECORD(0);
    role_record->set_name(FLAGS_role0);
    ADD_RECORD(1);
    role_record->set_name(FLAGS_role1);
    ADD_RECORD(2);
    role_record->set_name(FLAGS_role2);
    ADD_RECORD(3);
    role_record->set_name(FLAGS_role3);
    ADD_RECORD(4);
    role_record->set_name(FLAGS_role4);
    ADD_RECORD(5);
    role_record->set_name(FLAGS_role5);
    ADD_RECORD(6);
    role_record->set_name(FLAGS_role6);
    ADD_RECORD(7);
    role_record->set_name(FLAGS_role7);
    ADD_RECORD(8);
    role_record->set_name(FLAGS_role8);
    ADD_RECORD(9);
    role_record->set_name(FLAGS_role9);
    ADD_RECORD(10);
    role_record->set_name(FLAGS_role10);
    ADD_RECORD(11);
    role_record->set_name(FLAGS_role11);
    ADD_RECORD(12);
    role_record->set_name(FLAGS_role12);
    ADD_RECORD(13);
    role_record->set_name(FLAGS_role13);
    ADD_RECORD(14);
    role_record->set_name(FLAGS_role14);
    ADD_RECORD(15);
    role_record->set_name(FLAGS_role15);
    ADD_RECORD(16);
    role_record->set_name(FLAGS_role16);
    ADD_RECORD(17);
    role_record->set_name(FLAGS_role17);
    ADD_RECORD(18);
    role_record->set_name(FLAGS_role18);
    ADD_RECORD(19);
    role_record->set_name(FLAGS_role19);

    CHECK(writer_role.WriteMessage(role_list));
    CHECK(writer_quota.WriteMessage(quota_list));
    CHECK(writer_quota.Flush());
    CHECK(writer_role.Flush());
}

int main(int argc, char* argv[]) {
    //InitAllModules(&argc, &argv, true);
	google::AllowCommandLineReparsing();
	google::ParseCommandLineFlags(&argc, &argv, true);

    if (!FLAGS_read_quota_list.empty()) {
        ReadFileList();
    } else if (!FLAGS_write_quota_list.empty() &&
      !FLAGS_write_role_list.empty()) {
        WriteFileList(FLAGS_default_quota);
    } else {
        LOG(FATAL) << "Usage: ca_storage_tools --read_quota_list=<QuotaFile> or "
                   << "--write_quota_list=<QuotaFile> --write_role_list=<RoleFile>";
    }

    return 0;
}
