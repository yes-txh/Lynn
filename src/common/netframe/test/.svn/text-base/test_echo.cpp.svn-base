#include <common/system/concurrency/thread.hpp>
#include <common/netframe/netframe.hpp>
#include <common/netframe/socket_handler.hpp>

using namespace netframe;

void TestInitAndExit()
{
    for (int i= 0; i < 10; ++i)
    {
        NetFrame netframe(4);
    }
}

const size_t MaxPacketSize = 1024*1024;

std::string message(MaxPacketSize - 1, 'A');
std::string& message_ref = (message += '\n');

class TestStreamSocketHandler : public LineStreamSocketHandler
{
public:
    TestStreamSocketHandler(NetFrame& netframe): LineStreamSocketHandler(netframe)
    {
    }

    virtual bool OnSent(Packet* packet)
    {
        printf("OnPacketSent, size=%zd\n", packet->Length());
        return true;
    }
    void OnReceived(const Packet& packet)
    {
        printf("OnPacketReceived, size=%zd\n", packet.Length());
        if (GetNetFrame().SendPacket(GetEndPoint(), message.data(), message.length()) < 0)
            fprintf(stderr, "SendPacket error\n");
    }
    virtual void OnConnected()
    {
        printf("OnConnected\n");
        GetNetFrame().SendPacket(GetEndPoint(), message.data(), message.length());
    }
    virtual void OnClose(int error_code)
    {
        printf("%s: %s\n", __PRETTY_FUNCTION__, strerror(error_code));
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
    void OnConnected()
    {
        printf("%s\n", __PRETTY_FUNCTION__);
    }
    void OnClose(int error_code)
    {
        printf("%s: %s\n", __PRETTY_FUNCTION__, strerror(error_code));
    }
};

void TestEcho()
{
    NetFrame netframe(1);

    SocketAddressInet server_address("127.0.0.1:9998");
    int error = netframe.AsyncListen(server_address, new TestListenSocketHandler(netframe), MaxPacketSize);
    if (error < 0)
    {
        fprintf(stderr, "AsyncListen error: %s\n", strerror(-error));
        return;
    }

    error = netframe.AsyncConnect(server_address, new TestStreamSocketHandler(netframe), MaxPacketSize);
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

int main()
{
    TestEcho();
    //TestInitAndExit();
}
