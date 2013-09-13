#ifndef COMMON_RPC_PROTO_RPC_STREAMING_PACKET_H
#define COMMON_RPC_PROTO_RPC_STREAMING_PACKET_H

#include <string>
#include "common/base/closure.h"
#include "common/base/stdint.h"
#include "common/rpc/proto_rpc/streaming/stream.h"
#include "common/rpc/proto_rpc/streaming/streaming_service.pb.h"

namespace rpc {
namespace streaming {

struct Packet {
    enum Type {
        TYPE_UNKNOWN = 0,
        TYPE_READ,
        TYPE_WRITE,
        TYPE_EOF,
        TYPE_ABORT,
        TYPE_CLOSE,
    };
    Packet() : type(TYPE_UNKNOWN), id(-1), data(NULL), callback(NULL), timer_id(-1) {}
    Type type;
    int64_t id;
    std::string* data;
    CompletionCallback* callback;
    uint64_t timer_id;
};

} // namespace streaming
} // namespace rpc

#endif // COMMON_RPC_PROTO_RPC_STREAMING_PACKET_H
