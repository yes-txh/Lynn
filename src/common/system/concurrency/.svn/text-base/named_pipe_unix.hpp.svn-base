// Copyright (c) 2011, Tencent Inc. All rights reserved.
// modified by ivanhuang at 20101115

#ifndef COMMON_SYSTEM_CONCURRENCY_NAMED_PIPE_UNIX_HPP
#define COMMON_SYSTEM_CONCURRENCY_NAMED_PIPE_UNIX_HPP

#ifndef _WIN32

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include "common/base/uncopyable.hpp"

/**
 @brief 用于进程间通信的命名管道
*/

class NamedPipe : private Uncopyable
{
public:
    enum NamedPipeType {
        kReadOnly, kWriteOnly,
    };
public:
    NamedPipe();
    ~NamedPipe();

    bool Open(
        const char *fifo_name,
        NamedPipeType fifo_type,
        bool nonblocking = false,
        mode_t mode = 0755
    );

    void Close();
    bool SetNonblock(bool onoff = true);
    bool IsNonblock() const;
    int  Read(void *buffer, size_t buffer_len);
    int  Write(const void *buffer, size_t buffer_len);
    int  GetFd() const { return m_fd; }
private:
    bool Reopen();
    bool DoOpen();
private:
    std::string  m_name;  // 命名管道文件名
    int          m_mode;
    bool         m_nonblocking;
    int          m_fd;          // 命名管道文件句柄
    NamedPipeType m_type;       // 命名管道的读写状态
};

#endif // _WIN32

#endif // COMMON_SYSTEM_CONCURRENCY_NAMED_PIPE_UNIX_HPP
