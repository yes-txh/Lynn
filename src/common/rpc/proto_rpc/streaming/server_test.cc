#include <vector>
#include <signal.h>
#include "common/rpc/proto_rpc/streaming/file_service.pb.h"
#include "common/rpc/proto_rpc/streaming/stream.h"
#include "common/rpc/proto_rpc/streaming/stream_manager.h"
#include "common/rpc/proto_rpc/streaming/streaming_service.h"
#include "common/rpc/proto_rpc/http_service.h"
#include "common/rpc/proto_rpc/rpc_service.h"
#include "common/system/concurrency/thread.hpp"
// includes from thirdparty
#include "glog/logging.h"
#include "gflags/gflags.h"
#include "protobuf/service.h"

using namespace rpc;
using namespace rpc::streaming;

DEFINE_string(server_address, "127.0.0.1:50000", "the server address");

class FileServiceImpl : public FileService {
public:
    FileServiceImpl(ServerStreamManager* stream_manager) {
        CHECK_NOTNULL(stream_manager);
        m_stream_manager = stream_manager;
    }
    virtual ~FileServiceImpl() {
        //for (size_t i = 0; i < m_streams.size(); ++i) {
        //    m_streams[i]->Close();
        //}
        // Delete all streams when quit.
        for (size_t i = 0; i < m_streams.size(); ++i) {
            // Before deleting a stream, you MUST MAKE SURE the stream is closed
            // successfully, or the behavior will be unpredictable.
            m_streams[i]->WaitUntilClose();
            delete m_streams[i];
        }
        // You can also pass a cackback to stream Close function to delete the
        // stream. The code maybe like this:
        /*
        void DeleteStream(Stream* stream) {
            // No need call WaitUntilClose() because this function is a callback,
            // When it's called, the stream surely is closed successfully.
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
        LOG(INFO) << "In file service dtor.";
    }

    virtual void DownloadFile(google::protobuf::RpcController* controller,
            const DownloadFileRequest* request,
            DownloadFileResponse* response,
            google::protobuf::Closure* done) {
        FILE* file = fopen(request->full_path().c_str(), "rb");
        response->set_successful(file != NULL);
        OutputStream* stream = new OutputStream(m_stream_manager, request->stream_id());
        if (file != NULL) {
            m_streams.push_back(stream);
            DownloadFileBlock(stream, file, ERROR_SUCCESSFUL, NULL);
        } else {
            stream->Close();
            delete stream;
            LOG(INFO) << "file not exist: " << request->full_path();
        }
        done->Run();
    }

    virtual void UploadFile(google::protobuf::RpcController* controller,
            const UploadFileRequest* request,
            UploadFileResponse* response,
            google::protobuf::Closure* done) {
        FILE* file = fopen(request->full_path().c_str(), "wb");
        response->set_successful(file != NULL);
        InputStream* stream = new InputStream(m_stream_manager, request->stream_id());
        if (file != NULL) {
            m_streams.push_back(stream);
            stream->Read(NewClosure(this, &FileServiceImpl::UploadFileBlock, stream, file));
        } else {
            stream->Close();
            delete stream;
            LOG(INFO) << "failed to write to file: " << request->full_path();
        }
        done->Run();
    }

private:
    void DownloadFileBlock(OutputStream* stream,
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
        CompletionCallback* callback =
            NewClosure(this, &FileServiceImpl::DownloadFileBlock, stream, file);
        stream->Write(&tmp, callback);
    }

    void UploadFileBlock(InputStream* stream,
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
        stream->Read(NewClosure(this, &FileServiceImpl::UploadFileBlock, stream, file));
    }
private:
    std::vector<Stream*> m_streams;
    ServerStreamManager* m_stream_manager;
};

bool g_quit = false;

static void SignalIntHandler(int)
{
    g_quit = true;
}

int main(int argc, char** argv)
{
    rpc::HttpServer http_server;
    rpc::RpcService rpc_service(&http_server);
    rpc::RpcServerChannel server_channel(&rpc_service);

    StreamingServiceImpl streaming_service;
    rpc_service.RegisterService(&streaming_service);
    FileServiceImpl file_service(streaming_service.mutable_stream_manager());
    rpc_service.RegisterService(&file_service);

    if (!http_server.StartServer(FLAGS_server_address)) {
        return 1;
    }

    signal(SIGINT, SignalIntHandler);
    signal(SIGTERM, SignalIntHandler);

    while (!g_quit) {
        ThisThread::Sleep(1000);
    }

    return EXIT_SUCCESS;
}
