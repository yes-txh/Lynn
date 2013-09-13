#include <common/system/net/socket.hpp>
#include <common/system/time/timestamp.hpp>

int main()
{
    SocketAddressInet4 address("127.0.0.1:10000");
    for (;;)
    {
        long long t0 = GetTimeStampInMs();
        for (int i = 0; i < 1000; ++i)
        {
            StreamSocket connector(AF_INET, IPPROTO_TCP);
            connector.SetLinger(true, 1);
            if (connector.Connect(address))
            {
                static const char message[] = "hello, world!\n";
                connector.SendAll(message, strlen(message));
                char buffer[sizeof(message)];
                size_t received_length;
                connector.Receive(buffer, sizeof(buffer), received_length);
            }
        }
        t0 = GetTimeStampInMs() - t0;
        printf("time=%lld ms\n", t0);
    }
}

