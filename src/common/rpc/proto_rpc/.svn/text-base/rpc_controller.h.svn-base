// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)
//
// The implementation of rpc controller.

#ifndef RPC_PROTO_RPC_RPC_CONTROLLER_H_
#define RPC_PROTO_RPC_RPC_CONTROLLER_H_

#include <list>
#include <string>
#include "common/base/closure.h"
#include "common/base/stdint.h"
// includes from thirdparty
#include "protobuf/service.h"

namespace rpc {

class RpcService;

// This is per request.
class RpcController : public google::protobuf::RpcController {
public:
    enum Status {
        Status_Init = 0,
        Status_Pending,
        Status_Success,
        Status_NoMethodName,
        Status_InvalidMethodName,
        Status_ServiceNotFound,
        Status_MethodNotFound,
        Status_Timeout,
        Status_Canceled,
        Status_NoSessionFound,
        Status_SerializationError,
    };
    RpcController()
        : m_connection_id(-1),
          m_sequence_id(-1),
          m_status(0),
          m_handler(NULL),
          m_failed(false),
          m_canceled(false) {
    }
    virtual ~RpcController();

    int64_t connection_id() const { return m_connection_id; }
    void set_connection_id(const int64_t connection_id_value) {
        m_connection_id = connection_id_value;
    }

    int64_t sequence_id() const { return m_sequence_id; }
    void set_sequence_id(const int64_t sequence_id_value) {
        m_sequence_id = sequence_id_value;
    }

    // TODO(hansye): Define and implement error code.
    int32_t status() const { return m_status; }
    void set_status(int32_t status_value) {
        m_status = status_value;
    }

    void SetHandler(RpcService* rpc_service) {
        m_handler = rpc_service;
    }

    bool canceled() const { return m_canceled; }
    void set_canceled(const bool canceled_value) { 
        m_canceled = canceled_value;
    }

    // Implements client-side methods.
    // As we support "dual" rpc, they are "request" side methods more precisely,
    // or the "rpc client", not the "connection client".
    virtual void Reset() {
        m_connection_id = -1;
        m_sequence_id = -1;
        m_status = 0;
        m_failed = false;
        m_canceled = false;
        m_reason.clear();
        m_cancel_listeners.clear();
    }
    virtual bool Failed() const { return m_failed; }
    virtual std::string ErrorText() const { return m_reason; }
    // Actually the current implementatoin is NOT correct, even several open
    // source protobuf rpc implementations are doing this.
    // The semantics of this function should be the cliend side send a request
    // to server side to cancel an ongoing RPC call, all listeners on server
    // side would be notified.
    // TODO(hansye): re-consider the implementation.
    virtual void StartCancel() {
        m_canceled = true;
        for (std::list<google::protobuf::Closure*>::const_iterator i =
                 m_cancel_listeners.begin();
             i != m_cancel_listeners.end();
             ++i) {
            (*i)->Run();
        }
        m_cancel_listeners.clear();
    }

    // Implements server-side methods.
    // Similarly, they are "response" side methods more precisely.
    virtual void SetFailed(const std::string& reason) {
        m_failed = true;
        m_reason = reason;
    }
    virtual bool IsCanceled() const { return m_canceled; }
    virtual void NotifyOnCancel(google::protobuf::Closure* callback) {
        m_cancel_listeners.push_back(callback);
    }

private:
    // Customized for our rpc implementation.
    int64_t m_connection_id;
    int64_t m_sequence_id;
    int32_t m_status;
    RpcService* m_handler;

    // Required by google::protobuf::RpcController interface.
    bool m_failed;
    std::string m_reason;
    bool m_canceled;
    std::list<google::protobuf::Closure*> m_cancel_listeners;
};

} // namespace rpc

#endif // RPC_PROTO_RPC_RPC_CONTROLLER_H_
