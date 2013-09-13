#include <common/system/concurrency/thread.hpp>
#include <common/netframe/netframe.hpp>
#include <common/netframe/socket_handler.hpp>

using namespace netframe;

class TestStreamSocketHandler : public LineStreamSocketHandler
{
public:
    TestStreamSocketHandler(NetFrame& netframe): LineStreamSocketHandler(netframe)
    {
    }

    virtual bool OnSent(Packet* packet)
    {
        GetNetFrame().CloseEndPoint(GetEndPoint());
        return true;
    }
    void OnReceived(const Packet& packet)
    {
        //printf("OnPacketReceived, size=%zd\n", packet.Length());
        if (GetNetFrame().SendPacket(GetEndPoint(), packet.Content(), packet.Length()) < 0)
            fprintf(stderr, "SendPacket error\n");
    }
    virtual void OnConnected(){}
    virtual void OnClose(int error_code)
    {
        //printf("OnClosed\n");
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
        StreamSocket socket;
        socket.Attach(id.GetFd());
        socket.SetLinger(true, 1);
        socket.Detach();
        return new TestStreamSocketHandler(GetNetFrame());
    }
    void OnConnected(){}
    void OnClose(int error_code)
    {
    }
};

int main()
{
    NetFrame netframe;
    NetFrame::ListenEndPoint listen_endpoint;
    if (netframe.AsyncListen(SocketAddressInet("127.0.0.1:10000"), new TestListenSocketHandler(netframe), 32768) < 0)
        return EXIT_FAILURE;
    sleep(-1);
}

