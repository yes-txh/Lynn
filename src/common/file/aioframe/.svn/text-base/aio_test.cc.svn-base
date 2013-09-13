// Copyright (C) 2011, Tencent Inc.
// Author: Qianqiong Zhang (qqzhang@tencent.com)
//         Yongqiang Zou (aaronzou@tencent.com)
//
// Description: Test aio operations.

#ifndef Win32

#include <fcntl.h>

#include "thirdparty/glog/logging.h"
#include "thirdparty/gtest/gtest.h"

#include "common/file/aioframe/aioframe.h"
#include "common/system/concurrency/sync_event.hpp"

using namespace std;

const char* g_test_text = "a test data file.\n";

void ReadCallback(SyncEvent* sync_event, char *buffer, int64_t size, uint32_t error_code) {
    EXPECT_EQ(static_cast<int64_t>(strlen(g_test_text)), size);
    LOG(INFO) << "read size:" << size;
    EXPECT_EQ(0u, error_code);
    LOG(INFO) << "error code:" << error_code;
    LOG(INFO) << "read content:" << buffer;

    EXPECT_STREQ(g_test_text, buffer);

    delete []buffer;
    // notify others
    sync_event->Set();
}

void WriteCallback(SyncEvent* sync_event, int64_t size, uint32_t error_code) {
    EXPECT_EQ(static_cast<int64_t>(strlen(g_test_text)), size);
    LOG(INFO) << "write size:" << size;
    EXPECT_EQ(0u, error_code);
    LOG(INFO) << "error code:" << error_code;

    sync_event->Set();
}

class TEST_AIOFrame : public testing::Test {
public:
    TEST_AIOFrame() : m_aioframe(NULL), m_fd(-1), m_length(0) { };

    void SetUp() {
        LOG(INFO) << "setup env for every test to make sure no resource leak";
        m_aioframe = new aioframe::AIOFrame();

        m_fd = open("url.dat", O_RDWR | O_CREAT, 0755);
        ASSERT_TRUE(m_fd >= 0);
        m_length = strlen(g_test_text);
    }

    void TearDown() {
        LOG(INFO) << "teardown env";

        close(m_fd);

        delete m_aioframe;
        m_aioframe = NULL;
    }

public:
    aioframe::AIOFrame* m_aioframe;
    int m_fd;
    int64_t m_length;
};


TEST_F(TEST_AIOFrame, AsyncWriteTest) {
    SyncEvent* sync_event = new SyncEvent();

    Closure<void, int64_t, uint32_t>* callback = NewClosure(WriteCallback, sync_event);

    EXPECT_EQ(true, m_aioframe->AsyncWrite(m_fd, g_test_text, m_length, 0, callback, NULL));

    EXPECT_TRUE(sync_event->Wait(500)); // ms. Must be notifyed success

    LOG(INFO) << "Async read callback OK, so main thread ongoing";
    delete sync_event;
}

TEST_F(TEST_AIOFrame, AsyncReadTest) {
    SyncEvent* sync_event = new SyncEvent();

    char *buffer = new char[m_length + 1];
    buffer[m_length] = '\0';

    Closure<void, int64_t, uint32_t>* callback = NewClosure(ReadCallback, sync_event, buffer);

    EXPECT_EQ(true, m_aioframe->AsyncRead(m_fd, buffer, m_length, 0, callback, NULL));

    EXPECT_TRUE(sync_event->Wait(500)); // ms. Must be notifyed success

    LOG(INFO) << "Async read callback OK, so main thread ongoing";
    delete sync_event;
}

TEST_F(TEST_AIOFrame, ReadWriteTest) {
    std::string text(g_test_text);
    text += text;

    int64_t expect_size = static_cast<int64_t>(text.size());
    aioframe::StatusCode code;
    int64_t real_size = 0;
    real_size = m_aioframe->Write(m_fd, text.c_str(), expect_size, 0, &code);
    EXPECT_EQ(expect_size, real_size);
    EXPECT_EQ(code, 0);

    LOG(INFO) << "after write, read again to check result.";
    char* buffer = new char[expect_size + 1];
    buffer[expect_size] = '\0';

    real_size = m_aioframe->Read(m_fd, buffer, expect_size, 0, &code);
    EXPECT_EQ(expect_size, real_size);
    EXPECT_EQ(code, 0);
    EXPECT_STREQ(text.c_str(), buffer);
}

#endif
