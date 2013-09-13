// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)
//
// Define rpc request and rpc request queue, both for server and client side.

#ifndef RPC_PROTO_RPC_RPC_REQUEST_QUEUE_H_
#define RPC_PROTO_RPC_RPC_REQUEST_QUEUE_H_

#include <map>
#include <set>
#include <string>
#include "common/base/scoped_ptr.h"
#include "common/base/stdint.h"
#include "common/base/string/string_piece.hpp"
// includes from thirdparty
#include "protobuf/service.h"

class SimpleMutex;

namespace rpc {

class RpcController;
class RpcMeta;

// All data about one rpc request.
// We don't use closure as we need to re-schedule a rpc request to another
// session, and then can't bind rpc request to a session tightly.
struct RpcRequest {
    // The id of the request, which is unique within the session pool.
    int64_t sequence_id;
    // The expected time when the request should finish, in microseconds.
    int64_t expected_timeout;
    const google::protobuf::MethodDescriptor* method;
    RpcController* controller;
    const google::protobuf::Message* request;
    google::protobuf::Message* response;
    google::protobuf::Closure* done;

    RpcRequest(int64_t sequence_id,
               int64_t expected_timeout,
               const google::protobuf::MethodDescriptor* method,
               RpcController* controller,
               const google::protobuf::Message* request,
               google::protobuf::Message* response,
               google::protobuf::Closure* done);
};

// A simple queue for pending rpc requests.
class RpcRequestQueue {
public:
    typedef std::map<int64_t, RpcRequest*> RequestQueue;

    RpcRequestQueue();
    // Response callbacks for all pending requests would be called.
    ~RpcRequestQueue();

    // Add a new request (all information about a request) to the queue.
    // It assumes the sequence id of rpc request has been allocated and it's
    // caller's responsibility to guarantee no duplicated sequence id.
    void AddRequest(RpcRequest* rpc_request);

    // Cancel a request, remove it from pending request queue and run it's
    // callback immidiately.
    bool CancelRequest(int64_t sequence_id);

    // Remove the request from queue and return the that request.
    // Return NULL if the sequence id doesn't exist.
    RpcRequest* RemoveRequest(int64_t sequence_id);

    // Release all pending requests for a connection, and cancel them if
    // "cancel_requests" is true. If "cancel_requests" is false, the request
    // will not be canceled and just flaged as "undispatched" by set connection
    // id to -1.
    // It's called when a connection is closed.
    void ReleaseConnection(int64_t connection_id, bool cancel_requests);

    // Redispatch an "undispatched" request to a new connection id and return
    // it. Return NULL if no "undispatched" request anymore.
    RpcRequest* RedispatchRequest(int64_t connection_id);

    // Check all request whose expected timeout have been older than current
    // time and cancel the requests. It also garbage collects the timeout queue.
    void CheckTimeoutRequest(int64_t current_time);

private:
    // We need to sort the rpc requests using a secondary key other than the
    // sequence id, e.g. the connection id or expected timeout.
    struct RequestSecondaryKey {
        int64_t sequence_id;
        int64_t secondary_key;

        RequestSecondaryKey(int64_t sequence_id, int64_t secondary_key)
            : sequence_id(sequence_id), secondary_key(secondary_key) {
        }

        // We compare the secondary key firstly to ensure the item with least
        // secondary key is ranked firstly in the set.
        bool operator < (const RequestSecondaryKey& rhs) const {
            if (secondary_key < rhs.secondary_key) {
                return true;
            } else if (secondary_key > rhs.secondary_key) {
                return false;
            } else {
                return sequence_id < rhs.sequence_id;
            }
        }
    };

    // The sequence id is always unique, so no duplicated key in this set.
    // The RequestSecondaryKey has included all information so we use set
    // instead of map.
    typedef std::set<RequestSecondaryKey> RequestSecondaryQueue;

    // The mutex to protect state of pending requests.
    scoped_ptr<SimpleMutex> m_request_mutex;
    // It owns the pending requests.
    scoped_ptr<RequestQueue> m_request_queue;
    // The pending requests sorted by their connection ids.
    scoped_ptr<RequestSecondaryQueue> m_request_connection_queue;
    // The pending requests sorted by their expected timeout.
    scoped_ptr<RequestSecondaryQueue> m_request_timeout_queue;
};

} // namespace rpc

#endif // RPC_PROTO_RPC_RPC_REQUEST_QUEUE_H_
