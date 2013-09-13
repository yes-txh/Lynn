// Copyright (c) 2011, Tencent Inc. All rights reserved.
// ivanhuang @ 20101106

#include "common/system/concurrency/named_pipe.hpp"
#include "gtest/gtest.h"

#ifndef _WIN32

TEST(NamedPipe, OpenOpenClose) {
    const char *pipe_name = "ivanhuang";

    NamedPipe pipe_4_read;
    ASSERT_TRUE(pipe_4_read.Open(pipe_name, NamedPipe::kReadOnly, true));

    NamedPipe pipe_4_write;
    ASSERT_TRUE(pipe_4_write.Open(pipe_name, NamedPipe::kWriteOnly, true));

    pipe_4_read.Close();
    pipe_4_write.Close();

    ASSERT_FALSE(pipe_4_write.Open(pipe_name, NamedPipe::kWriteOnly, true));
}

TEST(NamedPipe, ReadWrite) {
    const char *pipe_name = "ivanhuang";

    char read_buffer[1024];
    size_t read_len = 1024;

    NamedPipe pipe_4_read;
    ASSERT_TRUE(pipe_4_read.Open(pipe_name, NamedPipe::kReadOnly, true));

    NamedPipe pipe_4_write;
    ASSERT_TRUE(pipe_4_write.Open(pipe_name, NamedPipe::kWriteOnly, true));

    ASSERT_LE(pipe_4_read.Read(read_buffer, read_len), 0);

    ASSERT_GT(pipe_4_write.Write(pipe_name, strlen(pipe_name) + 1), 0);
    ASSERT_GT(pipe_4_read.Read(read_buffer, read_len), 0);

    ASSERT_STRCASEEQ(pipe_name, read_buffer);
}

#endif
