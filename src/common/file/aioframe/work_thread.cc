// Copyright (C) 2011, Tencent Inc.
// Author: Qianqiong Zhang (qqzhang@tencent.com)
//
// Description: WorkThread implementation.

#include <sys/syscall.h>
#include <unistd.h>

#include "common/file/aioframe/work_thread.h"

#ifndef Win32

namespace aioframe {

WorkThread::WorkThread()
    : m_context(0), m_iocb_pool(new FixedObjectPool<iocb, kMaxAIORequest>()) {
}

WorkThread::~WorkThread() {
}

bool WorkThread::Start() {
    if(m_context != 0) return true;

    m_context = 0;
    if (syscall(SYS_io_setup, kMaxAIORequest, &m_context) < 0) {
        return false;
    }
    return BaseThread::Start();
}

bool WorkThread::Stop() {
    if (m_context == 0) return true;

    SendStopRequest();
    Join();

    syscall(SYS_io_destroy, m_context);
    m_context = 0;

    return true;
}

bool WorkThread::SubmitRequest(
    io_iocb_cmd_t command,
    int fd,
    void* buffer,
    int64_t size,
    int64_t start_position,
    Closure<void, int64_t, uint32_t>* callback,
    StatusCode* error_code) {

    return DoAIOSubmit(fd, command, buffer, size, start_position, callback, error_code);
}

bool WorkThread::DoAIOSubmit(int fd,
                            io_iocb_cmd_t command,
                            void* buffer,
                            int64_t size,
                            int64_t start_position,
                            Closure<void, int64_t, uint32_t>* callback,
                            StatusCode* error_code) {
    iocb* cb = m_iocb_pool->Acquire();
    PrepareRequest(cb, fd, command, buffer, size, start_position, callback);

    // TODO(aaronzou): syscall don't return error code like EAGAIN, which is by libaio.
    int ret = syscall(SYS_io_submit, m_context, 1, &cb);
    if (0 <= ret) {
        SetErrorCode(error_code, ret);
        return true;
    }
    switch (ret) {
        case EAGAIN:
        case ENOMEM:
            // Too busy.
            // Should retry? No. Let the AIOFrame application to decide retry policy.
            // Must avoid sleep to blocking possible network thread.
            SetErrorCode(error_code, ret);
            break;
        default:
            SetErrorCode(error_code, ret);
            break;
    }

    return false;
}

void WorkThread::Entry() {
    timespec timeout = {0, 1000000 * 20};
    while (!IsStopRequested()) {
        long n = syscall(SYS_io_getevents, m_context, 1, kMaxBatchNumber, m_events, &timeout);
        if (0 >= n) continue;
        ProcessCompletedRequests(n);
    }
}

void WorkThread::ProcessCompletedRequests(int request_number) {
    for (int i = 0; i < request_number; ++i) {
        iocb* cb = static_cast<iocb*>(m_events[i].obj);

        Closure<void, int64_t, uint32_t>* callback =
            static_cast<Closure<void, int64_t, uint32_t>*>(cb->data);
        if (callback != NULL) {
            callback->Run(m_events[i].res, m_events[i].res2);
        }

        m_iocb_pool->Release(cb);
    }  // for
}

}  // namespace aioframe

#endif
