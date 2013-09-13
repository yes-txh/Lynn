#include <string>
#include <vector>
#include "common/netframe/netframe.hpp"
#include "common/rpc/proto_rpc/streaming/file_service.pb.h"
#include "common/rpc/proto_rpc/streaming/stream.h"
#include "common/rpc/proto_rpc/streaming/stream_manager.h"
#include "common/rpc/proto_rpc/rpc_service.h"
#include "common/system/concurrency/thread.hpp"
#include "glog/logging.h"
#include "gflags/gflags.h"

using namespace rpc;
using namespace rpc::streaming;

DEFINE_string(server_address, "127.0.0.1:50000", "server address");

class FileServiceClient {
public:
    FileServiceClient(FileService::Stub* file_service_stub) {
        CHECK_NOTNULL(file_service_stub);
        m_file_service_stub = file_service_stub;
        google::protobuf::RpcChannel* channel = m_file_service_stub->channel();
        m_stream_manager = new ClientStreamManager(channel);
    }

    virtual ~FileServiceClient() {
        // Before deleting a stream manager, you should call WaitAllStreamClose()
        // to wait until all streams close gracefully.
        // If this function is not called, all streams will be forcibly closed
        // immediately.
        m_stream_manager->WaitAllStreamClose();
        delete m_stream_manager;

        // Release all streams allocated.
        for (size_t i = 0; i < m_streams.size(); ++i) {
            delete m_streams[i];
        }
        // You can also pass a cackback to stream Close function, if you want
        // delete the stream when the close action is completed.
        // The code maybe like this:
        /*
        void DeleteStream(Stream* stream) {
            MutexLocker locker(&m_mutex);
            std::vector<Stream*>::iterator iter =
                find(m_streams.begin(), m_streams.end(), stream);
            if (iter != m_streams.end()) {
                m_streams.erase(iter);
            }
            delete stream;
        }
        Stream* stream;
        Closure<void>* callback = NewClosure(
            this, &FileServiceImpl::DeleteStream, stream);
        stream->Close(callback);
        */
    }

    bool DownloadFile(const std::string& remote_path,
            const std::string& local_path) {
        FILE* file = fopen(local_path.c_str(), "wb");
        if (file == NULL) {
            LOG(INFO) << "Can't open file: " << local_path;
            return false;
        }

        InputStream* stream = new InputStream(m_stream_manager);
        if (!stream->Create(StreamOptions())) {
            LOG(INFO) << "Failed to create input stream.";
            return false;
        }

        RpcController controller;
        DownloadFileRequest request;
        DownloadFileResponse response;
        request.set_stream_id(stream->stream_id());
        request.set_full_path(remote_path);
        m_file_service_stub->DownloadFile(&controller, &request, &response, NULL);
        if (response.successful()) {
            m_streams.push_back(stream);
            LOG(INFO) << "Begin download file process...";
            stream->Read(NewClosure(this,
                    &FileServiceClient::DownloadFileBlock,
                    stream,
                    file));
        } else {
            stream->Close();
            delete stream;
        }
        return response.successful();
    }

    bool UploadFile(const std::string& remote_path,
            const std::string& local_path) {
        FILE* file = fopen(local_path.c_str(), "rb");
        if (file == NULL) {
            LOG(INFO) << "Can't open file: " << local_path;
            return false;
        }

        OutputStream* stream = new OutputStream(m_stream_manager);
        if (!stream->Create(StreamOptions())) {
            LOG(INFO) << "Failed to create input stream.";
            return false;
        }

        RpcController controller;
        UploadFileRequest request;
        UploadFileResponse response;
        request.set_stream_id(stream->stream_id());
        request.set_full_path(remote_path);
        m_file_service_stub->UploadFile(&controller, &request, &response, NULL);
        if (response.successful()) {
            m_streams.push_back(stream);
            LOG(INFO) << "Begin upload file process...";
            UploadFileBlock(stream, file, ERROR_SUCCESSFUL, NULL);
        } else {
            stream->Close();
            delete stream;
        }
        return response.successful();
    }

private:
    void DownloadFileBlock(InputStream* stream,
            FILE* file,
            ErrorCode error_code,
            const std::string* data) {
        if (error_code != ERROR_SUCCESSFUL) {
            // Encounter an error, close the file fd.
            fclose(file);
            // --------------------------------------------------------------
            // If you don't care about the error code, you can process it
            // simply using a single line :
            // stream->Close();
            // --------------------------------------------------------------
            // Process according to the error code:
            if (error_code == ERROR_END_OF_STREAM) {
                LOG(INFO) << "End of the stream, quit normally.";
                stream->Close();
            } else if (error_code == ERROR_STREAM_CLOSED) {
                LOG(INFO) << "The stream has been closed somewhere else. "
                    << "No more operations.";
            } else if (error_code == ERROR_STREAM_ABORTED) {
                LOG(INFO) << "Remote side has aborted the stream."
                    << " Nothing to do but close the stream.";
                stream->Close();
            } else {
                LOG(INFO) << "Encounter some unexpected error on the stream, "
                    << "error code: " << error_code
                    << ", Using Abort to inform remote side the stream is closed"
                    << " but not finished. ";
                stream->Abort();
            }
            return;
        }
        // Assume disk i/o always succeeds.
        fwrite(data->data(), 1, data->size(), file);
        stream->Read(NewClosure(this, &FileServiceClient::DownloadFileBlock, stream, file));
    }

    void UploadFileBlock(OutputStream* stream,
            FILE* file,
            ErrorCode error_code,
            const std::string* data) {
        if (error_code != ERROR_SUCCESSFUL) {
            // Encounter an error, close the file fd.
            fclose(file);
            // --------------------------------------------------------------
            // If you don't care about the error code, you can process it
            // simply using a single line :
            // stream->Close();
            // --------------------------------------------------------------
            // Process according to the error code:
            if (error_code == ERROR_END_OF_STREAM) {
                LOG(INFO) << "End of the stream, quit normally.";
                stream->Close();
            } else if (error_code == ERROR_STREAM_CLOSED) {
                LOG(INFO) << "The stream has been closed somewhere else. "
                    << "No more operations.";
            } else if (error_code == ERROR_STREAM_ABORTED) {
                LOG(INFO) << "Remote side has aborted the stream."
                    << " Nothing to do but close the stream.";
                stream->Close();
            } else {
                LOG(INFO) << "Encounter some unexpected error on the stream, "
                    << "error code: " << error_code
                    << ", Using Abort to inform remote side the stream is closed"
                    << " but not finished. ";
                stream->Abort();
            }
            return;
        }
        char buffer[8192];
        size_t size = fread(buffer, 1, sizeof(buffer), file);
        if (size <= 0) {
            // --------------------------------------------------------------
            // If you don't care about the error code, you can process it
            // simply using a single line :
            // stream->Close();
            // --------------------------------------------------------------
            if (feof(file)) {
                // End of file.
                stream->Close();
            } else {
                // Encounter an error. Abort the stream.
                stream->Abort();
            }
            fclose(file);
            return;
        }
        std::string tmp(buffer, size);
        stream->Write(&tmp, NewClosure(this, &FileServiceClient::UploadFileBlock, stream, file));
    }

private:
    FileService::Stub* m_file_service_stub;
    ClientStreamManager* m_stream_manager;
    std::vector<Stream*> m_streams;
};

int main(int argc, char** argv)
{
    scoped_ptr<netframe::NetFrame> net_frame(new netframe::NetFrame());
    scoped_ptr<rpc::RpcChannel> rpc_channel(
            new rpc::RpcChannel(net_frame.get()));
    {
        std::vector<std::string> server_addresses;
        SplitString(FLAGS_server_address, ",", &server_addresses);
        if (!rpc_channel->StartClient(server_addresses)) {
            return EXIT_FAILURE;
        }

        FileService::Stub file_service_stub(rpc_channel.get());
        FileServiceClient client(&file_service_stub);
        client.DownloadFile("server_to_download", "local_path");
        client.UploadFile("remote_path", "client_to_upload");
    }
    rpc_channel.reset(NULL);
    net_frame.reset(NULL);

    return EXIT_SUCCESS;
}
