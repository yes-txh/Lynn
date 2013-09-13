// Copyright (C) 2011, Tencent Inc.
// Author: Qianqiong Zhang (qqzhang@tencent.com)
//         Yongqiang Zou (aaronzou@tencent.com)
//
// Description: Encapsulate sync and async Read/Write inferface for native AIO.

#ifndef  COMMON_FILE_AIOFRAME_AIOFRAME_H_
#define  COMMON_FILE_AIOFRAME_AIOFRAME_H_

#ifndef WIN32
#include <libaio.h>
#include <stdint.h>
#endif

#include "common/base/scoped_ptr.h"
#include "common/base/closure.h"
#include "common/system/concurrency/spinlock.hpp"
#include "common/system/concurrency/sync_event.hpp"
#include "common/base/stdint.h"

namespace aioframe {

class WorkThread;

typedef int32_t StatusCode;

class AIOFrame {
public:
    AIOFrame();
    ~AIOFrame();

    // Return true for success to submit AIO request.
    // Return false for errors and error_code for reason.
    // Real operation data size and error code are parameters in callback.
    bool AsyncRead(int fd,
                      void* buffer,
                      int64_t size,
                      int64_t start_position,
                      Closure<void, int64_t, uint32_t>* callback,
                      StatusCode* error_code = NULL);

    // Return true for success to submit AIO request.
    // Return false for errors and error_code for reason.
    // Real operation data size and error code are parameters in callback.
    bool AsyncWrite(int fd,
                       const void* buffer,
                       int64_t size,
                       int64_t start_position,
                       Closure<void, int64_t, uint32_t>* callback,
                       StatusCode* error_code = NULL);

    // Simuate sync read by async read. Return the read bytes, or less than 0 for error.
    int64_t Read(int fd,
                 void* buffer,
                 int64_t size,
                 int64_t start_position,
                 StatusCode* error_code = NULL);

    // Simuate sync write by async write. Return the write bytes, or less than 0 for error.
    int64_t Write(int fd,
                  const void* buffer,
                  int64_t size,
                  int64_t start_position,
                  StatusCode* error_code = NULL);

private:
    void ReadCallback(
        SyncEvent* sync_event,
        int64_t* read_size,
        StatusCode* read_code,
        int64_t size,
        uint32_t error_code);

    void WriteCallback(
        SyncEvent* sync_event,
        int64_t* write_size,
        StatusCode* read_code,
        int64_t size,
        uint32_t error_code);

    Spinlock m_lock;
    scoped_ptr<WorkThread>  m_worker;
};

}  // aioframe


#endif  // COMMON_FILE_AIOFRAME_AIOFRAME_H_
