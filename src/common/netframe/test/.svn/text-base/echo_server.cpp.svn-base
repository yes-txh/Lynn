#include "common/system/concurrency/thread.hpp"
#include "common/netframe/netframe.hpp"
#include "common/netframe/socket_handler.hpp"
#include "common/system/concurrency/atomic/atomic.hpp"
#include "common/config/cflags.hpp"

using namespace netframe;

Atomic<int> request_count;

class TestStreamSocketHandler : public LineStreamSocketHandler
{
public:
    TestStreamSocketHandler(NetFrame& netframe): LineStreamSocketHandler(netframe)
    {
    }

    virtual bool OnSent(Packet* packet)
    {
        return true;
    }
    void OnReceived(const Packet& packet)
    {
        ++request_count;
        //printf("OnPacketReceived, size=%zd\n", packet.Length());
        if (GetNetFrame().SendPacket(GetEndPoint(), packet.Content(), packet.Length()) < 0)
            fprintf(stderr, "SendPacket error\n");
    }
    virtual void OnConnected(){}
    virtual void OnClose(int error_code)
    {
        printf("OnClosed: %s\n", strerror(error_code));
    }
};

class TestListenSocketHandler : public ListenSocketHandler
{
public:
    TestListenSocketHandler(NetFrame& netframe): ListenSocketHandler(netframe)
    {
    }
    StreamSocketHandler* OnAccepted(SocketId id)
    {
        return new TestStreamSocketHandler(GetNetFrame());
    }
    void OnClose(int error_code)
    {
        printf("OnClosed\n");
    }
};

CFLAGS_DEFINE_FLAG(std::string, listen, cflags::DefaultIs("127.0.0.1:61001"), "the address server listen on");

int main(int argc, char** argv)
{
    if (!CFlags::ParseCommandLine(argc, argv))
        return EXIT_FAILURE;

    NetFrame netframe;
    NetFrame::ListenEndPoint listen_endpoint;

    if (netframe.AsyncListen(SocketAddressInet(Flag_listen.Value()), new TestListenSocketHandler(netframe), 32768) < 0)
    {
        perror("listen");
        return EXIT_FAILURE;
    }

    for (;;)
    {
        ThisThread::Sleep(1000);
        printf("request frequence=%d/s\n", request_count.Value());
        request_count = 0;
    }
}

