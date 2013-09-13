// Copyright (C) 2011, Tencent Inc.
// Author: Qianqiong Zhang (qqzhang@tencent.com)
//
// Description: workthread to wrap all AIO operations.

#ifndef  COMMON_FILE_AIOFRAME_WORK_THREAD_H_
#define  COMMON_FILE_AIOFRAME_WORK_THREAD_H_

#include "common/base/scoped_ptr.h"
#include "common/base/closure.h"
#include "common/base/object_pool.hpp"
#include "common/system/concurrency/base_thread.hpp"

#include "common/file/aioframe/aioframe.h"

#ifndef WIN32

namespace aioframe {

static const int kMaxAIORequest = 1024;
static const int kMaxBatchNumber = 32;

class WorkThread: public BaseThread {
public:
    WorkThread();
    virtual ~WorkThread();

    virtual void Entry();

    bool Start();

    bool Stop();

    bool SubmitRequest(
        io_iocb_cmd_t command,
        int fd,
        void* buffer,
        int64_t size,
        int64_t start_position,
        Closure<void, int64_t, uint32_t>* callback,
        StatusCode* error_code);

private:
    void PrepareRequest(
        struct iocb* cb,
        int fd,
        io_iocb_cmd_t command,
        void* buffer,
        int64_t size,
        int64_t start_position,
        Closure<void, int64_t, uint32_t>* callback) {
        memset(cb, 0, sizeof(*cb));
        cb->aio_fildes = fd;
        cb->aio_lio_opcode = command;
        cb->aio_reqprio = 0;
        cb->u.c.buf = buffer;
        cb->u.c.nbytes = size;
        cb->u.c.offset = start_position;
        cb->data = callback;
    }

    bool DoAIOSubmit(int fd,
                    io_iocb_cmd_t command,
                    void* buffer,
                    int64_t size,
                    int64_t start_position,
                    Closure<void, int64_t, uint32_t>* callback,
                    StatusCode* error_code);

    void ProcessCompletedRequests(int request_number);

private:
    void SetErrorCode(StatusCode* ptr_error_code, const StatusCode error_code) {
        if(ptr_error_code) {
            *ptr_error_code = error_code;
        }
    }

    io_context_t m_context;
    io_event m_events[kMaxBatchNumber];

    scoped_ptr<FixedObjectPool<iocb, kMaxAIORequest> > m_iocb_pool;
};

}  // namespace aioframe

#endif

#endif  // COMMON_FILE_AIOFRAME_WORK_THREAD_H_
