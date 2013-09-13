// Copyright 2010, Tencent Inc.
// Author: Yongqiang Zou (aaronzou@tencent.com)
//
// test functions of file.cc
// 2011-1-3    create the test file.

#include <ctype.h>

#include "common/file/file.h"
#include "common/file/local_file.h"
#include "common/system/concurrency/thread.hpp"
#include "common/system/concurrency/sync_event.hpp"

#include "thirdparty/glog/logging.h"
#include "thirdparty/gtest/gtest.h"

std::string g_test_dir_name = std::string("") + "file_test_dir";
std::string g_test_file_name = g_test_dir_name + File::kPathSeparator + "file_test.txt";
File* g_test_file_obj = NULL;
File* g_test_file_obj_r = NULL;

void ReadCallback(SyncEvent* sync_event, int64_t size, uint32_t error_code) {
    LOG(INFO) << "File test, read callback is called, result size = " << size << " error_code = " << error_code;
    if (sync_event != NULL) sync_event->Set();
}

void WriteCallback(SyncEvent* sync_event, int64_t size, uint32_t error_code) {
    LOG(INFO) << "File test, write callback is called, result size = " << size << " error_code = " << error_code;
    if (sync_event != NULL) sync_event->Set();
}

class FileTest : public testing::Test {
protected:
    static void SetUpTestCase() {
        //Should fail if init fails.
        ASSERT_EQ(true, File::Init());

        uint32_t error_code = 0;
        const char* dir_name = g_test_dir_name.c_str();
        if (File::CheckExist(dir_name, &error_code)) {
            uint32_t error_code2 = 0;
            File::Remove(dir_name, true, &error_code2);
            EXPECT_EQ(error_code2, ERR_FILE_OK);
            ASSERT_FALSE(File::CheckExist(dir_name, &error_code));
        }
    }

    static void TearDownTestCase() {
        File::CleanUp();
    }
};


int32_t main(int32_t argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

TEST_F(FileTest, KeyValueInfoHelperTest) {
    std::string options;
    std::map<std::string, std::string> options_map;
    KeyValueInfoHelper::ParseKeyValueInfo(options, &options_map);
    EXPECT_EQ(0u, options_map.size());

    //ignore options without ":" or '='.
    bool rst = false;
    options = "a";
    rst = KeyValueInfoHelper::ParseKeyValueInfo(options, &options_map);
    EXPECT_EQ(0u, options_map.size());
    EXPECT_EQ(rst, false);

    options = "a=";
    rst = KeyValueInfoHelper::ParseKeyValueInfo(options, &options_map);
    EXPECT_EQ(0u, options_map.size());
    EXPECT_EQ(rst, false);

    options = "r=1";
    rst = KeyValueInfoHelper::ParseKeyValueInfo(options, &options_map);
    EXPECT_EQ(1u, options_map.size());
    EXPECT_EQ(true, rst);
    std::string newOptions;
    rst = KeyValueInfoHelper::CreateKeyValueInfo(options_map, &newOptions);
    EXPECT_EQ(options, newOptions);
    EXPECT_EQ(true, rst);

    options = "fid=123456:r=5";
    rst = KeyValueInfoHelper::ParseKeyValueInfo(options, &options_map);
    EXPECT_EQ(2u, options_map.size());
    EXPECT_TRUE(options_map.find("r") != options_map.end());
    EXPECT_EQ(true, rst);

    std::string replica = options_map["r"];
    int r = atoi(replica.c_str());
    LOG(INFO) << "parsed replica: --" << r << "--";
    EXPECT_EQ(5, r);
    rst = KeyValueInfoHelper::CreateKeyValueInfo(options_map, &newOptions);
    EXPECT_EQ(options, newOptions);
    EXPECT_EQ(true, rst);

    std::string key("fid");
    std::string value("33311124");
    std::string info;

    rst = KeyValueInfoHelper::AppendKeyValueInfo(key, value, NULL);
    EXPECT_EQ(rst, false);
    rst = KeyValueInfoHelper::AppendKeyValueInfo("", value, NULL);
    EXPECT_EQ(rst, false);
    rst = KeyValueInfoHelper::AppendKeyValueInfo(key, "", NULL);
    EXPECT_EQ(rst, false);

    rst = KeyValueInfoHelper::AppendKeyValueInfo(key, value, &info);
    EXPECT_EQ(rst, true);
    EXPECT_EQ(info, "fid=33311124");

    rst = KeyValueInfoHelper::AppendKeyValueInfo(key, value, &info);
    EXPECT_EQ(rst, true);
    EXPECT_EQ(info, "fid=33311124:fid=33311124");
}

TEST_F(FileTest, FilePathParserTest) {
    uint32_t file_back_factor = 1;
    uint32_t cache_buf_len = 5;
    uint32_t cache_data_interval = 10;

    std::string file_name("/xfs/sz/dir1/my_file.txt");
    std::string temp_full_path = file_name + ":r=5:bufsz=100000:bufit=1000";
    std::string full_path = temp_full_path;

    // Parse additional options in file path.
    std::map<std::string, std::string> options_map;
    size_t optionIdx = temp_full_path.find(":");
    EXPECT_TRUE(optionIdx != std::string::npos);

    if (optionIdx != std::string::npos) {
        // has additional options in file path.
        bool parse_ok = KeyValueInfoHelper::ParseKeyValueInfo(
                                temp_full_path.substr(optionIdx + 1),
                                &options_map);
        EXPECT_EQ(parse_ok, true);

        // Overwrite options in OpenFileOption.
        if (options_map.find("r") != options_map.end()) {
            const std::string& r_str = options_map["r"];
            file_back_factor = atoi(r_str.c_str());
        }
        EXPECT_EQ(file_back_factor, 5u);
        if (options_map.find("bufsz") != options_map.end()) {
            const std::string& sz_str = options_map["bufsz"];
            cache_buf_len = atoi(sz_str.c_str());
        }
        EXPECT_EQ(cache_buf_len, 100000u);
        if (options_map.find("bufit") != options_map.end()) {
            const std::string& it_str = options_map["bufit"];
            cache_data_interval = atoi(it_str.c_str());
        }
        EXPECT_EQ(cache_data_interval, 1000u);

        // The real path should skip the options.
        full_path = temp_full_path.substr(0, optionIdx);
        EXPECT_EQ(full_path, file_name);
        LOG(INFO) << "input path:" << temp_full_path;
        LOG(INFO) << "parsed path:" << full_path;
    }
}

TEST_F(FileTest, AddDir) {
    uint32_t error_code = 0;
    const char* dir_name = g_test_dir_name.c_str();
    EXPECT_EQ(0, File::AddDir(dir_name, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);

    EXPECT_EQ(true, File::CheckExist(dir_name, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);

    EXPECT_EQ(true, File::CheckExist(".", &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
    // add again.
    EXPECT_EQ(-1, File::AddDir(dir_name, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_ENTRY_EXIST);
}

TEST_F(FileTest, Open) {
    int open_mode = File::ENUM_FILE_OPEN_MODE_W;
    uint32_t error_code = 0;
    // First test local file.
    const char* file_name = g_test_file_name.c_str();

    OpenFileOptions options;
    options.backup_factor = 3; // back_factor.
    g_test_file_obj = File::Open(file_name, open_mode, options, &error_code);
    ASSERT_TRUE(g_test_file_obj != NULL);
    EXPECT_EQ(error_code, ERR_FILE_OK);
    EXPECT_EQ(g_test_file_obj->GetFileImplName(), LOCAL_FILE_PREFIX);
    EXPECT_TRUE(File::CheckExist(file_name, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
}

// @brief:   测试local_file.cc的WriteFile
TEST_F(FileTest, WriteFile) {
    uint32_t error_code = 0;
    const char* test_string = "this is a text file,hello world";
    std::string buf_str(test_string);
    const char* buf = buf_str.c_str();
    int64_t test_string_len = strlen(test_string) + 1;
    int64_t wirten_len = g_test_file_obj->Write(buf,
                                           test_string_len,
                                           &error_code);
    EXPECT_EQ(test_string_len, wirten_len);
    EXPECT_EQ(error_code, ERR_FILE_OK);
    g_test_file_obj->Flush(&error_code);
    EXPECT_EQ(error_code, ERR_FILE_OK);
}


// @brief:   测试local_file.cc的SeekFile
TEST_F(FileTest, SeekFile) {
    int open_mode = File::ENUM_FILE_OPEN_MODE_R;
    uint32_t error_code = 0;
    // First test local file.
    const char* file_name = g_test_file_name.c_str();

    OpenFileOptions options;
    g_test_file_obj_r = File::Open(file_name, open_mode, options, &error_code);
    ASSERT_TRUE(g_test_file_obj_r != NULL);

    int64_t offset = 5;
    int64_t offset_ret = 0;
    offset_ret = g_test_file_obj_r->Seek(5, SEEK_SET, &error_code);
    EXPECT_EQ(offset, offset_ret);
    EXPECT_EQ(error_code, ERR_FILE_OK);
}

// @brief:   测试local_file.cc的ReadFile
TEST_F(FileTest, ReadFile) {
    uint32_t error_code = 0;
    char buf[5];

    // after seek.
    int64_t readn_len = g_test_file_obj_r->Read(buf, sizeof(buf) - 1, &error_code);
    EXPECT_EQ(4, readn_len);
    int cmp_result = memcmp(buf, "is a", 4);
    EXPECT_EQ(0, cmp_result);
    EXPECT_EQ(error_code, ERR_FILE_OK);
}

// @brief:   测试AsyncWriteFile
TEST_F(FileTest, AsyncWriteFile) {
    const char* test_string = "this is a text file,hello world";
    std::string buf_str(test_string);
    const char* buf = buf_str.c_str();

    int64_t test_string_len = strlen(test_string) + 1;

    SyncEvent sync_event;
    Closure<void, int64_t, unsigned int> *callback =
        NewClosure(WriteCallback, &sync_event);

    uint32_t error_code;
    g_test_file_obj->Seek(0, SEEK_END, &error_code);
    if (g_test_file_obj->SupportAsync()) {
        EXPECT_EQ(0,
            g_test_file_obj->AsyncWrite(buf,
                test_string_len, callback, 1, &error_code));
        LOG(INFO) << "AsyncWriteFile returned.";
    }
    LOG(INFO) << "to wait async operation callback";
    sync_event.Wait();

    EXPECT_EQ(error_code, ERR_FILE_OK);
}

// @brief:   测试AsyncReadFrom
TEST_F(FileTest, AsyncReadFrom) {
    SyncEvent sync_event;
    char buf[128] = {0};
    Closure<void, int64_t, unsigned int> *callback =
        NewClosure(ReadCallback, &sync_event);

    uint32_t error_code = 0;
    EXPECT_EQ(0, g_test_file_obj_r->Seek(0, SEEK_SET, &error_code));

    if (g_test_file_obj_r->SupportAsync()) {
        EXPECT_EQ(0,
            g_test_file_obj_r->AsyncReadFrom(buf,
                sizeof(buf) - 1, 5, callback, 1, &error_code));
        LOG(INFO) << "AsyncReadFrom returned.";
    }

    LOG(INFO) << "to wait async operation callback";
    sync_event.Wait();

    EXPECT_EQ(error_code, ERR_FILE_OK);
    EXPECT_TRUE(strcmp(buf, "is a text file,hello world") == 0);
}

TEST_F(FileTest, AsyncReadFromWrong) {
    SyncEvent sync_event;
    char buf[128] = {0};
    Closure<void, int64_t, unsigned int> *callback =
        NewClosure(ReadCallback, &sync_event);

    uint32_t error_code = 0;
    EXPECT_EQ(0, g_test_file_obj_r->Seek(0, SEEK_SET, &error_code));

    if (g_test_file_obj_r->SupportAsync()) {
        EXPECT_EQ(0,
            g_test_file_obj_r->AsyncReadFrom(buf,
                sizeof(buf) - 1, 50000, callback, 1, &error_code));
        LOG(INFO) << "AsyncReadFrom returned.";
    }

    LOG(INFO) << "to wait async operation callback";
    if (error_code == ERR_FILE_OK) {
        sync_event.Wait();
    }

    EXPECT_EQ(ERR_FILE_OK, error_code);
}

// @brief:   测试local_file.cc的TellFile
TEST_F(FileTest, TellFile) {
    uint32_t error_code = 0;

    // 先seek到文件中一个位置，再来验证Tell
    EXPECT_EQ(5, g_test_file_obj->Seek(5, SEEK_SET, &error_code));
    EXPECT_EQ(5, g_test_file_obj->Tell(&error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
}

// @brief:   测试local_file.cc的Truncate
TEST_F(FileTest, Truncate) {
    uint32_t error_code;
    // 之前的数据都还在缓冲里，得flush到文件中
    g_test_file_obj->Flush(&error_code);
    EXPECT_EQ(error_code, ERR_FILE_OK);

    int64_t file_size = File::GetSize(g_test_file_name.c_str(), &error_code);
    int64_t new_size;
    if (file_size > 5) {
        new_size = 5;
    } else {
        new_size = 0;
    }
    EXPECT_EQ(0, g_test_file_obj->Truncate(new_size, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);

    file_size = File::GetSize(g_test_file_name.c_str(), &error_code);
    EXPECT_EQ(new_size, (int64_t)file_size);
    g_test_file_obj->Flush(&error_code);
    EXPECT_EQ(error_code, ERR_FILE_OK);
}

TEST_F(FileTest, Close) {
    uint32_t error_code = 0;

    EXPECT_TRUE(g_test_file_obj->Close(&error_code) == 0);
    EXPECT_EQ(error_code, ERR_FILE_OK);

    EXPECT_TRUE(g_test_file_obj_r->Close(&error_code) == 0);
    EXPECT_EQ(error_code, ERR_FILE_OK);
    // Must delete the created File object.
    delete g_test_file_obj;
    delete g_test_file_obj_r;
    g_test_file_obj = NULL;
    g_test_file_obj_r = NULL;
}

// @brief:   测试local_file.cc的CheckExist
TEST_F(FileTest, CheckExist) {
    uint32_t error_code = 0;
    const char* file_name = g_test_file_name.c_str();

    EXPECT_TRUE(File::CheckExist(file_name, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
}

// @brief:   测试local_file.cc的List
TEST_F(FileTest, List) {
    uint32_t error_code;

    std::vector<AttrsInfo> file_info;
    AttrsMask mask;
    mask.file_size = 1;
    EXPECT_EQ(0, File::List(g_test_dir_name.c_str(), &mask, &file_info, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
    EXPECT_EQ(1u, file_info.size());
    EXPECT_EQ(file_info[0].file_name, "file_test.txt");
    EXPECT_EQ((uint32_t)5, file_info[0].file_size);

    LOG(INFO) << "input list name: " << g_test_dir_name.c_str();
    std::vector<AttrsInfo>::iterator iter;
    for(iter = file_info.begin(); iter != file_info.end(); ++iter) {
        LOG(INFO) << iter->file_name;
    }

    EXPECT_EQ(0, File::List(g_test_file_name.c_str(), &mask, &file_info, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
    LOG(INFO) << "input list name: " << g_test_file_name.c_str();
    for(iter = file_info.begin(); iter != file_info.end(); ++iter) {
        LOG(INFO) << iter->file_name;
    }
    EXPECT_EQ(1u, file_info.size());
    EXPECT_EQ(file_info[0].file_name, "file_test.txt");
}

// @brief:   测试local_file.cc的List
TEST_F(FileTest, Du) {
    uint32_t error_code;
    int64_t ret = File::Du(g_test_file_name.c_str(), &error_code);
    EXPECT_EQ(5, ret);
}

// @brief:   测试local_file.cc的Rename
TEST_F(FileTest, Rename) {
    uint32_t error_code;

    //rename it.
    std::string new_name = g_test_file_name + ".new";
    EXPECT_EQ(0, File::Rename(g_test_file_name.c_str(), new_name.c_str(), &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
    EXPECT_TRUE(File::CheckExist(new_name.c_str(), &error_code));
    EXPECT_FALSE(File::CheckExist(g_test_file_name.c_str(), &error_code));

    EXPECT_EQ(0, File::Rename(new_name.c_str(), g_test_file_name.c_str(), &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
    EXPECT_FALSE(File::CheckExist(new_name.c_str(), &error_code));
    EXPECT_TRUE(File::CheckExist(g_test_file_name.c_str(), &error_code));
}

TEST_F(FileTest, ListWithPattern) {
    uint32_t error_code;
    bool is_recursive = true;
    std::string work_dir("./tmp/");
    File::Remove(work_dir.c_str(), is_recursive, &error_code);

    ASSERT_EQ(0, File::AddDir(work_dir.c_str(), &error_code));
    ASSERT_EQ(0, File::Copy(g_test_file_name.c_str(), (work_dir + "file1.txt").c_str(), &error_code));
    ASSERT_EQ(0, File::Copy(g_test_file_name.c_str(), (work_dir + "file2.txt").c_str(), &error_code));
    ASSERT_EQ(0, File::Copy(g_test_file_name.c_str(), (work_dir + "file3.txt").c_str(), &error_code));

    std::vector<AttrsInfo> files;
    AttrsMask mask;
    EXPECT_EQ(0, File::List("./tmp/file*.txt", &mask, &files, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
    EXPECT_EQ(files.size(), 3u);
    LOG(INFO) << "matched file count " << files.size();

    EXPECT_EQ(0, File::List("./tmp/file1.txt", &mask, &files, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
    EXPECT_EQ(files.size(), 1u);

    EXPECT_EQ(0, File::List("./tmp/file1*.txt", &mask, &files, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
    EXPECT_EQ(files.size(), 1u);

    EXPECT_EQ(0, File::List("./tmp/*", &mask, &files, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
    EXPECT_EQ(files.size(), 3u);

    EXPECT_EQ(0, File::List("*", &mask, &files, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
    LOG(INFO) << "matched file count for *: " << files.size();
    size_t count = files.size();

    EXPECT_EQ(0, File::List(".", &mask, &files, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
    LOG(INFO) << "matched file count for .: " << files.size();
    EXPECT_EQ(count, files.size());

    EXPECT_EQ(-1, File::List("./tmp*/file1.txt", &mask, &files, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_FAIL);

    // clean up
    EXPECT_EQ(0, File::Remove(work_dir.c_str(), is_recursive, &error_code));
}

// @brief:   测试local_file.cc的Remove
TEST_F(FileTest, Remove) {
    uint32_t error_code;
    bool is_recursive = true;
    // 删除前
    EXPECT_TRUE(File::CheckExist(g_test_dir_name.c_str(), &error_code));
    EXPECT_EQ(0, File::Remove(g_test_dir_name.c_str(), is_recursive, &error_code));
    CHECK_EQ(error_code, ERR_FILE_OK);
    // 删除后
    EXPECT_FALSE(File::CheckExist(g_test_dir_name.c_str(), &error_code));
}

TEST_F(FileTest, AsyncAppendFile) {
    int open_mode = File::ENUM_FILE_OPEN_MODE_A;

    const char* file_name = "test_append";
    // first remove, so that to setup a clean env.
    File::Remove(file_name, false, NULL);

    uint32_t error_code = 0;
    File* file = File::Open(file_name, open_mode, OpenFileOptions(), &error_code);
    EXPECT_TRUE(file != NULL);
    EXPECT_EQ(error_code, ERR_FILE_OK);

    open_mode = File::ENUM_FILE_OPEN_MODE_R;
    File* file_r = File::Open(file_name, open_mode, OpenFileOptions(), &error_code);
    EXPECT_TRUE(file_r != NULL);
    EXPECT_EQ(error_code, ERR_FILE_OK);

    const char *test_string = "this is a text file,hello world";
    std::string buf_str(test_string);
    const char* buf = buf_str.c_str();
    int64_t test_string_len = strlen(test_string) + 1;

    SyncEvent sync_event_write1;
    SyncEvent sync_event_write2;
    Closure<void, int64_t, unsigned int> *callback1 =
        NewPermanentClosure(WriteCallback, &sync_event_write1);
    Closure<void, int64_t, unsigned int> *callback2 =
        NewPermanentClosure(WriteCallback, &sync_event_write2);
    if (file->SupportAsync()) {
        EXPECT_EQ(0,
            file->AsyncWrite(buf, test_string_len, callback1, 1, &error_code));
        EXPECT_EQ(0,
            file->AsyncWrite(buf, test_string_len, callback2, 1, &error_code));
    }
    LOG(INFO) << "sleep to wait async operation callback";
    sync_event_write1.Wait();
    sync_event_write2.Wait();

    EXPECT_EQ(error_code, ERR_FILE_OK);
    delete callback1;
    delete callback2;

    LOG(INFO) << "to read after append";
    SyncEvent sync_event;
    Closure<void, int64_t, unsigned int> *read_callback =
        NewPermanentClosure(ReadCallback, &sync_event);

    // Read data to verify
    char* read_buf = new char[test_string_len * 2];
    if (file_r->SupportAsync()) {
        EXPECT_EQ(0,
            file_r->AsyncReadFrom(read_buf, test_string_len * 2, 0, read_callback, 1, &error_code));
    }
    LOG(INFO) << "to wait async operation callback";
    sync_event.Wait();
    LOG(INFO) << "to async operation callback OK";

    EXPECT_EQ(error_code, ERR_FILE_OK);

    delete read_callback;
    delete []read_buf;

    File::Remove(file_name, false, NULL);
}

TEST_F(FileTest, GetPrefix) {
    std::string file_name = "/xfs/sz/test4Options.dat:r=5:bufsz=1024";
    std::string prefix = File::GetFilePrefix(file_name.c_str());
    EXPECT_EQ(prefix, "/xfs/");
}

