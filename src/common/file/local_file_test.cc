//////////////////////////////////////////////////////////////////////////
// @file:   local_file_test.cc
// @brief:  测试local_file.cc
// @author: bradzhang@tencent
// @time:   2010-9
// 修改历史:
//          <author>    <time>
//          joeytian    2010-11-12
//          aaronzou    2011-01-10
//////////////////////////////////////////////////////////////////////////
// 测试local_file的各项功能

#include "common/config/cflags.hpp"
#include "common/file/local_file.h"
#include "common/system/concurrency/thread.hpp"
#include "thirdparty/glog/logging.h"
#include "thirdparty/gtest/gtest.h"

std::string g_test_dir_name = std::string("") + "file_test_dir";
std::string g_test_file_name = g_test_dir_name + File::kPathSeparator + "file_test.txt";

// 全局的localfile
LocalFile* g_local_file = NULL;

// 定义回调函数
void ReadCallback(int64_t size, uint32_t error_code) {
    LOG(INFO) << "LocalFile test, read callback is called, result size = " << size << " error_code = " << error_code;
}

void WriteCallback(int64_t size, uint32_t error_code) {
    LOG(INFO) << "LocalFile test, write callback is called, result size = " << size << " error_code = " << error_code;
}


class TEST_LocalFile:public testing::Test
{
protected:
    static void SetUpTestCase()
    {
        //google::InitGoogleLogging("LocalFileTest");
        bool init_ok = File::Init();
        EXPECT_EQ(true, init_ok);

        // 先删除可能遗留的测试文件夹
        uint32_t error_code = 0;
        const char* dir_name = g_test_dir_name.c_str();
        bool is_recursive = true;
        if (File::CheckExist(dir_name, &error_code))
             EXPECT_EQ(0, File::Remove(dir_name, is_recursive, &error_code));        
    }

    static void TearDownTestCase()
    {
        File::CleanUp();
    }
};

////////////////////////////////////////////////////////////////////////
// 测试的main函数
int32_t main(int32_t argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}

// @brief:   测试local_file.cc的AddDir
TEST_F(TEST_LocalFile, AddDir)
{
    uint32_t error_code = 0;
    const char* dir_name = g_test_dir_name.c_str();
    EXPECT_EQ(0, File::AddDir(dir_name,&error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
    EXPECT_EQ(true, File::CheckExist(dir_name,&error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
    EXPECT_EQ(true, File::CheckExist(".",&error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
    EXPECT_EQ(File::CheckExist("......",&error_code), false);
    EXPECT_EQ(error_code, ERR_FILE_FAIL);
}

// @brief:   测试local_file.cc的OpenFile
TEST_F(TEST_LocalFile, Open)
{  
    int open_mode = File::ENUM_FILE_OPEN_MODE_R | File::ENUM_FILE_OPEN_MODE_W;
    const char* file_name = g_test_file_name.c_str();
    uint32_t error_code = 0;
    g_local_file = (LocalFile*)File::Open(file_name, open_mode, OpenFileOptions(), &error_code);
    EXPECT_TRUE(g_local_file != NULL);    
    EXPECT_EQ(error_code, ERR_FILE_OK);    
}

// @brief:   测试local_file.cc的CheckExist
TEST_F(TEST_LocalFile, CheckExist)
{
    const char* file_name = g_test_file_name.c_str();

    uint32_t error_code = 0;
    EXPECT_TRUE(File::CheckExist(file_name, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
}

// @brief:   测试local_file.cc的WriteFile
TEST_F(TEST_LocalFile, WriteFile)
{
    uint32_t error_code = 0;
    const char *test_string = "this is a text file,hello world";
    std::string buf_str(test_string);
    const char* buf = buf_str.c_str();
    int64_t test_string_len = strlen(test_string) + 1;
    int64_t wirten_len = g_local_file->Write(buf,
                                           test_string_len,
                                           &error_code);
    EXPECT_EQ(test_string_len, wirten_len);    
    EXPECT_EQ(error_code, ERR_FILE_OK);
    g_local_file->Flush(&error_code);
    EXPECT_EQ(error_code, ERR_FILE_OK);
}


// @brief:   测试local_file.cc的SeekFile
TEST_F(TEST_LocalFile, SeekFile)
{
    uint32_t error_code = 0;
    int64_t offset = 5;
    int64_t offset_ret = 0;
    offset_ret = g_local_file->Seek(5, SEEK_SET, &error_code);
    EXPECT_EQ(offset, offset_ret);
    EXPECT_EQ(error_code, ERR_FILE_OK);
}

// @brief:   测试local_file.cc的ReadFile
TEST_F(TEST_LocalFile, ReadFile)
{    
    uint32_t error_code = 0;
    char buf[5];

    int64_t readn_len = g_local_file->Read(buf, sizeof(buf) - 1,&error_code);
    EXPECT_EQ(4, readn_len);
    int cmp_result = memcmp(buf, "is a", 4);
    EXPECT_EQ(0, cmp_result);
    EXPECT_EQ(error_code, ERR_FILE_OK);    
}

// @brief:   测试local_file.cc的AsyncWriteFile
TEST_F(TEST_LocalFile, AsyncWriteFile)
{
    const char *test_string = "this is a text file,hello world";
    std::string buf_str(test_string);
    const char* buf = buf_str.c_str();
    int64_t test_string_len = strlen(test_string) + 1;

    Closure<void, int64_t, unsigned int> *callback = 
        NewPermanentClosure(WriteCallback);
    uint32_t error_code;
    g_local_file->Seek(0, SEEK_END, &error_code);
    if (g_local_file->SupportAsync()) {
        EXPECT_EQ(0, 
            g_local_file->AsyncWrite(buf, test_string_len, callback, 1, &error_code));
    }
    LOG(INFO) << "sleep to wait async operation callback";
    ThisThread::Sleep(1000);
    EXPECT_EQ(error_code, ERR_FILE_OK);
    delete callback;    
}

// @brief:   测试local_file.cc的AsyncReadFrom
TEST_F(TEST_LocalFile, AsyncReadFrom)
{
    char buf[128] = {0};
    Closure<void, int64_t, unsigned int> *callback = 
        NewPermanentClosure(ReadCallback);
    uint32_t error_code = 0;
    EXPECT_EQ(0, g_local_file->Seek(0, SEEK_SET, &error_code));

    if (g_local_file->SupportAsync()) {
        EXPECT_EQ(0, 
            g_local_file->AsyncReadFrom(buf, sizeof(buf) - 1, 3, callback, 1, &error_code));
    }
   
    LOG(INFO) << "sleep to wait async operation callback";
    ThisThread::Sleep(1000);
    EXPECT_EQ(error_code, ERR_FILE_OK);
    EXPECT_TRUE(strcmp(buf, "s is a text file,hello world") == 0);
    delete callback;
}


// @brief:   测试local_file.cc的TellFile
TEST_F(TEST_LocalFile, TellFile)
{    
    uint32_t error_code = 0;

    // 先seek到文件中一个位置，再来验证Tell
    EXPECT_EQ(5, g_local_file->Seek(5, SEEK_SET, &error_code));
    EXPECT_EQ(5, g_local_file->Tell(&error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
}

// @brief:   测试local_file.cc的Truncate
TEST_F(TEST_LocalFile, Truncate)
{
    uint32_t error_code;
    const char* file_name = g_test_file_name.c_str();
    // 之前的数据都还在缓冲里，得flush到文件中
    g_local_file->Flush(&error_code);
    EXPECT_EQ(error_code, ERR_FILE_OK);
    int64_t file_size = File::GetSize(file_name, &error_code);
    int64_t new_size;
    if (file_size > 5)
    {
        new_size = 5;
    } 
    else
    {
        new_size = 0;   
    }
    EXPECT_EQ(0, g_local_file->Truncate(new_size, &error_code));
    EXPECT_EQ(error_code, ERR_FILE_OK);
    file_size = File::GetSize(file_name, &error_code);    
    EXPECT_EQ(new_size, (int64_t)file_size);
    g_local_file->Flush(&error_code);
    EXPECT_EQ(error_code, ERR_FILE_OK);
}

// @brief:   测试local_file.cc的GetFileSize
TEST_F(TEST_LocalFile, GetFileSize)
{
    uint32_t error_code;
    const char* file_name = g_test_file_name.c_str();
    int64_t ret = File::GetSize(file_name, &error_code);
    EXPECT_EQ(5, ret);
    EXPECT_EQ(error_code, ERR_FILE_OK);
}
TEST_F(TEST_LocalFile, Close) {
    int32_t ret = g_local_file->Close();
    EXPECT_EQ(0, ret);
    delete g_local_file;
    g_local_file = NULL;
}

// @brief:   测试local_file.cc的List
TEST_F(TEST_LocalFile, List)
{
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
TEST_F(TEST_LocalFile, Du)
{
    uint32_t error_code;
    const char* file_name = g_test_file_name.c_str();
    int64_t ret = File::Du(file_name,&error_code);
    EXPECT_EQ(5, ret);
}

// @brief:   测试local_file.cc的Rename
TEST_F(TEST_LocalFile, Rename)
{
    uint32_t error_code;
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

// @brief:   测试local_file.cc的Remove
TEST_F(TEST_LocalFile, Remove)
{
    uint32_t error_code;
    const char* dir_name = g_test_dir_name.c_str();
    bool is_recursive = true;
    // 删除前
    EXPECT_TRUE(File::CheckExist(dir_name, &error_code));
    EXPECT_EQ(0, File::Remove(dir_name, is_recursive, &error_code)); 
    CHECK_EQ(error_code,ERR_FILE_OK);
    // 删除后
    EXPECT_FALSE(File::CheckExist(dir_name, &error_code));
}

TEST_F(TEST_LocalFile, AsyncAppendFile)
{
    // A | R is the only mode to pass the test
    int open_mode = File::ENUM_FILE_OPEN_MODE_A | File::ENUM_FILE_OPEN_MODE_R;
    // int open_mode = File::ENUM_FILE_OPEN_MODE_A | File::ENUM_FILE_OPEN_MODE_W;
    // int open_mode = File::ENUM_FILE_OPEN_MODE_A;

    const char* file_name = "test_append";
    // first remove, so that to setup a clean env.
    File::Remove(file_name, false, NULL);

    uint32_t error_code = 0;
    LocalFile* file = (LocalFile*)File::Open(file_name, open_mode, OpenFileOptions(), &error_code);
    EXPECT_TRUE(file != NULL);
    EXPECT_EQ(error_code, ERR_FILE_OK);

    const char *test_string = "this is a text file,hello world";
    std::string buf_str(test_string);
    const char* buf = buf_str.c_str();
    int64_t test_string_len = strlen(test_string) + 1;

    Closure<void, int64_t, unsigned int> *callback =
        NewPermanentClosure(WriteCallback);
    if (file->SupportAsync()) {
        EXPECT_EQ(0,
            file->AsyncWrite(buf, test_string_len, callback, 1, &error_code));
        EXPECT_EQ(0,
            file->AsyncWrite(buf, test_string_len, callback, 1, &error_code));
    }
    LOG(INFO) << "sleep to wait async operation callback";
    while(true) {
        if (File::GetSize(file_name) == 2 * test_string_len) {
            break;
        }
        ThisThread::Sleep(10);
    }
    EXPECT_EQ(error_code, ERR_FILE_OK);
    delete callback;

    LOG(INFO) << "to read after append";
    Closure<void, int64_t, unsigned int> *read_callback =
        NewPermanentClosure(ReadCallback);

    // Read data to verify
    char* read_buf = new char[test_string_len * 2];
    if (file->SupportAsync()) {
        EXPECT_EQ(0,
            file->AsyncReadFrom(read_buf, test_string_len * 2, 0, read_callback, 1, &error_code));
    }
    LOG(INFO) << "sleep to wait async operation callback";
    ThisThread::Sleep(1000);
    EXPECT_EQ(error_code, ERR_FILE_OK);

    delete []read_buf;

    File::Remove(file_name, false, NULL);
}

