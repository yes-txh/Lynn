#include "common/netframe/netframe.hpp"
#include "common/netframe/socket_handler.hpp"
#include "common/system/net/socket.hpp"
#include "common/system/concurrency/thread.hpp"
#include "common/system/time/timestamp.hpp"
#include "common/config/cflags.hpp"

using namespace netframe;

CFlags::Flag<std::string> Flag_server(
    __FILE__,
    "server",
    cflags::DefaultIs("127.0.0.1:61001"),
    "the address server listen on"
);

static const char message[] =
"hello, worldhello, worldhello, world!!!!!!hello, world!\n";
size_t message_length = strlen(message);

void test_sync()
{
    SocketAddressInet4 address(Flag_server);
    StreamSocket connector(AF_INET, IPPROTO_TCP);
    if (connector.Connect(address))
    {
        for (;;)
        {
            long long t0 = GetTimeStampInMs();
            for (int i = 0; i < 40000; ++i)
            {
                connector.SendAll(message, message_length);
                char buffer[sizeof(message)];
                size_t received_length;
                connector.ReceiveAll(buffer, message_length, received_length);
            }

            t0 = GetTimeStampInMs() - t0;
            printf("time=%lld ms\n", t0);
        }
    }
}

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
        if (GetNetFrame().SendPacket(GetEndPoint(), message, message_length) < 0)
            fprintf(stderr, "SendPacket error\n");
    }
    virtual void OnConnected()
    {
        printf("OnConnected\n");
        if (GetNetFrame().SendPacket(GetEndPoint(), message, message_length) < 0)
        {
            fprintf(stderr, "SendPacket error\n");
        }
    }
    virtual void OnClose(int error_code)
    {
        printf("OnClosed, error: %s\n", strerror(error_code));
    }
};

void test_netframe()
{
    NetFrame netframe;

    SocketAddressInet server_address(Flag_server);

    int error = netframe.AsyncConnect(server_address, new TestStreamSocketHandler(netframe), 128);
    if (error < 0)
    {
        fprintf(stderr, "AsyncListen error: %s\n", strerror(-error));
        return;
    }

    for (;;)
    {
        ThisThread::Sleep(-1);
    }
}

int main(int argc, char** argv)
{
    if (!CFlags::ParseCommandLine(argc, argv))
        return EXIT_FAILURE;

    test_sync();
    // test_netframe();
}

