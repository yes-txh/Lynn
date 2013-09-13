// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)

#include <list>
#include <utility>
#include "common/rpc/proto_rpc/rpc_controller.h"
#include "common/rpc/proto_rpc/rpc_meta.pb.h"
#include "common/rpc/proto_rpc/rpc_request_queue.h"
#include "common/system/concurrency/mutex.hpp"
// includes from thirdparty
#include "glog/logging.h"
#include "protobuf/descriptor.h"

namespace rpc {

namespace {

static const int64_t kUndispachedRequestConnectionId = -1;

// Call this function when a rpc request is being canceled, because of timeout
// or connection being closed.
static void CancelRpcRequest(RpcRequest* rpc_request) {
    CHECK(rpc_request->controller->Failed())
        << "The controller must be set as failed when being canceled.";
    rpc_request->done->Run();
    delete rpc_request;
}

} // namespace

// struct RpcRequest;
RpcRequest::RpcRequest(const int64_t sequence_id,
                       const int64_t expected_timeout,
                       const google::protobuf::MethodDescriptor* method,
                       RpcController* controller,
                       const google::protobuf::Message* request,
                       google::protobuf::Message* response,
                       google::protobuf::Closure* done)
    : sequence_id(sequence_id),
      expected_timeout(expected_timeout),
      method(method),
      controller(controller),
      request(request),
      response(response),
      done(done) {
    controller->set_sequence_id(sequence_id);
}

// class RpcRequestQueue
RpcRequestQueue::RpcRequestQueue()
    : m_request_mutex(new SimpleMutex()),  // non-recursive.
      m_request_queue(new RequestQueue()),
      m_request_connection_queue(new RequestSecondaryQueue()),
      m_request_timeout_queue(new RequestSecondaryQueue()) {
}

// Response callbacks for all pending requests would be called.
RpcRequestQueue::~RpcRequestQueue() {
    scoped_ptr<RequestQueue> request_queue;
    {
        Mutex::Locker locker(*m_request_mutex);
        request_queue.swap(m_request_queue);
    }

    for (RequestQueue::const_iterator i = request_queue->begin();
         i != request_queue->end();
         ++i) {
        RpcRequest* rpc_request = i->second;
        rpc_request->controller->SetFailed(
            "Possibly session disconnected, all requests discarded.");
        rpc_request->controller->set_status(RpcController::Status_Canceled);
        CancelRpcRequest(i->second);
    }
}

// Add a new request (all information about a request) to the queue.
// It assumes the sequence id of rpc request has been allocated and it's
// caller's responsibility to guarantee no duplicated sequence id.
void RpcRequestQueue::AddRequest(RpcRequest* rpc_request) {
    CHECK_GE(rpc_request->sequence_id, 0);
    CHECK_GE(rpc_request->controller->connection_id(),
             kUndispachedRequestConnectionId);
    CHECK_GE(rpc_request->expected_timeout, 0);
    {
        Mutex::Locker locker(*m_request_mutex);
        CHECK(m_request_queue->insert(
            std::make_pair(rpc_request->sequence_id, rpc_request)).second)
            << "Duplicated sequence id in request queue: "
            << rpc_request->sequence_id;

        RequestSecondaryKey request_connection(
            rpc_request->sequence_id, rpc_request->controller->connection_id());
        CHECK(m_request_connection_queue->insert(request_connection).second)
            << "Duplicated sequence id in connection queue: "
            << request_connection.sequence_id
            << ", connection id: "
            << request_connection.secondary_key;

        RequestSecondaryKey request_timeout(rpc_request->sequence_id,
                                            rpc_request->expected_timeout);
        CHECK(m_request_timeout_queue->insert(request_timeout).second)
            << "Duplicated sequence id in timeout queue: "
            << request_timeout.sequence_id
            << ", timeout: "
            << request_timeout.secondary_key;
    }
}

// Remove the request from queue and return the that request.
// Return NULL if the sequence id doesn't exist.
RpcRequest* RpcRequestQueue::RemoveRequest(const int64_t sequence_id) {
    RpcRequest* rpc_request;
    {
        Mutex::Locker locker(*m_request_mutex);
        RequestQueue::iterator it = m_request_queue->find(sequence_id);
        if (it == m_request_queue->end()) {
            return NULL;
        }

        rpc_request = it->second;
        m_request_queue->erase(it);

        RequestSecondaryKey request_connection(
            rpc_request->sequence_id, rpc_request->controller->connection_id());
        CHECK_EQ(1u, m_request_connection_queue->erase(request_connection))
            << "Connection queue missing an expected item: "
            << request_connection.sequence_id
            << ", connection id: "
            << request_connection.secondary_key;

        RequestSecondaryKey request_timeout(rpc_request->sequence_id,
                                            rpc_request->expected_timeout);
        CHECK_EQ(1u, m_request_timeout_queue->erase(request_timeout))
            << "Timeout queue missing an expected item: "
            << request_timeout.sequence_id
            << ", timeout: "
            << request_timeout.secondary_key;
    }

    return rpc_request;
}

void RpcRequestQueue::ReleaseConnection(int64_t connection_id,
                                        bool cancel_requests) {
    std::list<RpcRequest*> released_requests;
    {
        Mutex::Locker locker(*m_request_mutex);
        RequestSecondaryKey request_connection(INT64_MIN, connection_id);
        RequestSecondaryQueue::iterator it_begin =
            m_request_connection_queue->lower_bound(request_connection);
        request_connection.sequence_id = INT64_MAX;
        RequestSecondaryQueue::iterator it_end =
            m_request_connection_queue->upper_bound(request_connection);

        for (RequestSecondaryQueue::iterator i = it_begin; i != it_end; ) {
            RequestSecondaryQueue::iterator it_connection = i++;
            RequestQueue::iterator it =
                m_request_queue->find(it_connection->sequence_id);
            CHECK(it != m_request_queue->end())
                << "Request queue missing an expected item: "
                << it_connection->sequence_id
                << ", timeout: "
                << it_connection->secondary_key;

            m_request_connection_queue->erase(it_connection);

            RpcRequest* rpc_request = it->second;
            if (cancel_requests) {
                released_requests.push_back(rpc_request);

                m_request_queue->erase(it);

                RequestSecondaryKey request_connection(
                    rpc_request->sequence_id, rpc_request->expected_timeout);
                CHECK_EQ(1u, m_request_timeout_queue->erase(request_connection))
                    << "Timeout queue missing an expected item: "
                    << request_connection.sequence_id
                    << ", timeout: "
                    << request_connection.secondary_key;
            } else {
                rpc_request->controller->set_connection_id(
                    kUndispachedRequestConnectionId);

                RequestSecondaryKey request_connection(
                    rpc_request->sequence_id, rpc_request->controller->connection_id());
                CHECK(m_request_connection_queue->insert(request_connection).second)
                    << "Duplicated sequence id in connection queue: "
                    << request_connection.sequence_id
                    << ", connection id: "
                    << request_connection.secondary_key;
            }
        }
    }

    for (std::list<RpcRequest*>::const_iterator i = released_requests.begin();
         i != released_requests.end();
         ++i) {
        RpcRequest* rpc_request = *i;
        rpc_request->controller->SetFailed("Connection closed.");
        rpc_request->controller->set_status(RpcController::Status_Canceled);
        CancelRpcRequest(rpc_request);
    }
}

RpcRequest* RpcRequestQueue::RedispatchRequest(int64_t connection_id) {
    CHECK_GE(connection_id, 0);
    {
        Mutex::Locker locker(*m_request_mutex);
        RequestSecondaryQueue::iterator it_connection =
            m_request_connection_queue->begin();
        if ((it_connection == m_request_connection_queue->end()) ||
            (it_connection->secondary_key != kUndispachedRequestConnectionId)) {
            return NULL;
        }

        RequestQueue::iterator it =
            m_request_queue->find(it_connection->sequence_id);
        CHECK(it != m_request_queue->end())
            << "Request queue missing an expected item: "
            << it_connection->sequence_id
            << ", timeout: "
            << it_connection->secondary_key;

        RpcRequest* rpc_request = it->second;
        rpc_request->controller->set_connection_id(connection_id);

        RequestSecondaryKey request_connection(it_connection->sequence_id,
                                               connection_id);
        m_request_connection_queue->erase(it_connection);
        CHECK(m_request_connection_queue->insert(request_connection).second)
            << "Duplicated sequence id in connection queue: "
            << request_connection.sequence_id
            << ", connection id: "
            << request_connection.secondary_key;
        return rpc_request;
    }
}

void RpcRequestQueue::CheckTimeoutRequest(int64_t current_time) {
    std::list<RpcRequest*> timeout_requests;
    {
        Mutex::Locker locker(*m_request_mutex);
        while (!m_request_timeout_queue->empty()) {
            RequestSecondaryQueue::iterator it_timeout =
                m_request_timeout_queue->begin();
            if (it_timeout->secondary_key >= current_time) {
                break;
            }

            RequestQueue::iterator it =
                m_request_queue->find(it_timeout->sequence_id);
            CHECK(it != m_request_queue->end())
                << "Request queue missing an expected item: "
                << it_timeout->sequence_id
                << ", timeout: "
                << it_timeout->secondary_key;

            // It's a timeout request, put it in the timeout requests and cancel
            // it later.
            // Note: we don't cancel it here as canceling would invoke user's
            // callback and user possibly invoke other functions of rpc request
            // queue which needs to acquire the lock, like the destructor. So we
            // put all timeout requests in a temporary queue on stack and cancel
            // them later.
            RpcRequest* rpc_request = it->second;
            timeout_requests.push_back(rpc_request);

            m_request_queue->erase(it);

            RequestSecondaryKey request_connection(
                rpc_request->sequence_id,
                rpc_request->controller->connection_id());
            CHECK_EQ(1u, m_request_connection_queue->erase(request_connection))
                << "Connection queue missing an expected item: "
                << request_connection.sequence_id
                << ", connection id: "
                << request_connection.secondary_key;

            m_request_timeout_queue->erase(it_timeout);
        }
    }

    for (std::list<RpcRequest*>::const_iterator i = timeout_requests.begin();
         i != timeout_requests.end();
         ++i) {
        RpcRequest* rpc_request = *i;
        rpc_request->controller->SetFailed("Request timeout.");
        rpc_request->controller->set_status(RpcController::Status_Timeout);
        CancelRpcRequest(rpc_request);
    }
}

bool RpcRequestQueue::CancelRequest(int64_t sequence_id) {
    RpcRequest* rpc_request;
    {
        Mutex::Locker locker(*m_request_mutex);
        RequestQueue::iterator it = m_request_queue->find(sequence_id);
        if (it == m_request_queue->end()) {
            return false;
        }

        rpc_request = it->second;
        m_request_queue->erase(it);

        RequestSecondaryKey request_connection(
            rpc_request->sequence_id, rpc_request->controller->connection_id());
        CHECK_EQ(1u, m_request_connection_queue->erase(request_connection))
            << "Connection queue missing an expected item: "
            << request_connection.sequence_id
            << ", connection id: "
            << request_connection.secondary_key;

        RequestSecondaryKey request_timeout(rpc_request->sequence_id,
                                            rpc_request->expected_timeout);
        CHECK_EQ(1u, m_request_timeout_queue->erase(request_timeout))
            << "Timeout queue missing an expected item: "
            << request_timeout.sequence_id
            << ", timeout: "
            << request_timeout.secondary_key;
    }
    rpc_request->controller->SetFailed("User canceled.");
    rpc_request->controller->set_status(RpcController::Status_Canceled);
    CancelRpcRequest(rpc_request);
    return true;
}

} // namespace rpc
