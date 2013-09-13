// Copyright 2011, Tencent Inc.
// Author: Hangjun Ye (hansye@tencent.com)
// Xiaokang Liu (hsiaokangliu@tencent.com)
//
// Implement the rpc streaming service.

#ifndef COMMON_RPC_PROTO_RPC_STREAMING_STREAMING_SERVICE_H
#define COMMON_RPC_PROTO_RPC_STREAMING_STREAMING_SERVICE_H

#include "common/rpc/proto_rpc/streaming/streaming_service.pb.h"

namespace rpc {
namespace streaming {

class ServerStreamManager;

class StreamingServiceImpl : public StreamingService {
public:
    StreamingServiceImpl();
    virtual ~StreamingServiceImpl();

    ServerStreamManager* mutable_stream_manager() {
        return m_stream_manager;
    }

private:
    virtual void CreateInputStream(
            google::protobuf::RpcController* controller,
            const CreateStreamRequest* request,
            CreateStreamResponse* response,
            google::protobuf::Closure* done);

    virtual void CreateOutputStream(
            google::protobuf::RpcController* controller,
            const CreateStreamRequest* request,
            CreateStreamResponse* response,
            google::protobuf::Closure* done);

    virtual void CloseInputStream(
            google::protobuf::RpcController* controller,
            const CloseStreamRequest* request,
            CloseStreamResponse* response,
            google::protobuf::Closure* done);

    virtual void CloseOutputStream(
            google::protobuf::RpcController* controller,
            const CloseStreamRequest* request,
            CloseStreamResponse* response,
            google::protobuf::Closure* done);

    virtual void DownloadPacket(
            google::protobuf::RpcController* controller,
            const DownloadPacketRequest* request,
            DownloadPacketResponse* response,
            google::protobuf::Closure* done);

    virtual void UploadPacket(
            google::protobuf::RpcController* controller,
            const UploadPacketRequest* request,
            UploadPacketResponse* response,
            google::protobuf::Closure* done);
protected:
    ServerStreamManager* m_stream_manager;
};

} // namespace streaming
} // namespace rpc

#endif // COMMON_RPC_PROTO_RPC_STREAMING_STREAMING_SERVICE_H
