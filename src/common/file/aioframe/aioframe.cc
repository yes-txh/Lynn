// Copyright (C) 2011, Tencent Inc.
// Author: Qianqiong Zhang (qqzhang@tencent.com)
//         Yongqiang Zou (aaronzou@tencent.com)
//
// Description: Implement AIOFrame.

#include "common/file/aioframe/aioframe.h"
#include "common/file/aioframe/work_thread.h"


#include "thirdparty/glog/logging.h"

#ifndef Win32

namespace aioframe {

AIOFrame::AIOFrame() {
    m_worker.reset(new WorkThread());
    CHECK(m_worker->Start()) << "fail to start worker thread in AIOFrame ctor.";
}
AIOFrame::~AIOFrame() {
    m_worker->Stop();
}

bool AIOFrame::AsyncRead(int fd,
                            void* buffer,
                            int64_t size,
                            int64_t start_position,
                            Closure<void, int64_t, uint32_t>* callback,
                            StatusCode* error_code) {
    CHECK_NOTNULL(callback);

    return m_worker->SubmitRequest(::IO_CMD_PREAD,
                                   fd,
                                   buffer,
                                   size,
                                   start_position,
                                   callback,
                                   error_code);
}

bool AIOFrame::AsyncWrite(int fd,
                             const void* buffer,
                             int64_t size,
                             int64_t start_position,
                             Closure<void, int64_t, uint32_t>* callback,
                             StatusCode* error_code) {
    CHECK_NOTNULL(callback);

    return m_worker->SubmitRequest(::IO_CMD_PWRITE,
                                   fd,
                                   const_cast<void*>(buffer),
                                   size,
                                   start_position,
                                   callback,
                                   error_code);
}

int64_t AIOFrame::Read(int fd,
                       void* buffer,
                       int64_t size,
                       int64_t start_position,
                       StatusCode* error_code) {
    SyncEvent sync_event;
    int64_t read_size = 0;

    Closure<void, int64_t, uint32_t>* callback =
        NewClosure(this, &AIOFrame::ReadCallback, &sync_event, &read_size, error_code);

    StatusCode submit_code = 0;
    if (AsyncRead(fd, buffer, size, start_position, callback, &submit_code) < 0) {
        *error_code = submit_code;
        return -1;
    }

    sync_event.Wait();

    return read_size;
}

int64_t AIOFrame::Write(int fd,
                        const void* buffer,
                        int64_t size,
                        int64_t start_position,
                        StatusCode* error_code) {
    SyncEvent sync_event;
    int64_t write_size = 0;

    Closure<void, int64_t, uint32_t>* callback =
        NewClosure(this, &AIOFrame::WriteCallback, &sync_event, &write_size, error_code);

    StatusCode submit_code = 0;
    if (AsyncWrite(fd, buffer, size, start_position, callback, &submit_code) < 0) {
        *error_code = submit_code;
        return -1;
    }

    sync_event.Wait();

    return write_size;
}

void AIOFrame::ReadCallback(
    SyncEvent* sync_event,
    int64_t* read_size,
    StatusCode* read_code,
    int64_t size,
    uint32_t error_code) {
    *read_size = size;
    *read_code = static_cast<StatusCode>(error_code);

    sync_event->Set();
}

void AIOFrame::WriteCallback(
    SyncEvent* sync_event,
    int64_t* write_size,
    StatusCode* write_code,
    int64_t size,
    uint32_t error_code) {
    *write_size = size;
    *write_code = static_cast<StatusCode>(error_code);

    sync_event->Set();
}

}

#endif
