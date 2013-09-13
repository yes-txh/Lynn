// Copyright (c) 2011, Tencent Inc. All rights reserved.
// Author: CHEN Feng <phongchen@tencent.com>

#ifndef COMMON_SYSTEM_CONCURRENCY_NAMED_PIPE_HPP
#define COMMON_SYSTEM_CONCURRENCY_NAMED_PIPE_HPP

#if defined __unix__
#include "common/system/concurrency/named_pipe_unix.hpp"
#elif defined _WIN32
#include "common/system/concurrency/named_pipe_windows.hpp"
#else
#error Unknown platform
#endif

#endif // COMMON_SYSTEM_CONCURRENCY_NAMED_PIPE_HPP

