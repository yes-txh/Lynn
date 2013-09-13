#include "common/system/net/socket.hpp"
#include <gtest/gtest.h>

TEST(Socket, IPAddress)
{
    ASSERT_TRUE(IPAddress("1.2.3.4") == IPAddress(1, 2, 3, 4));
    ASSERT_TRUE(IPAddress("0.0.0.0") == IPAddress::Any());
    ASSERT_TRUE(IPAddress("127.0.0.1") == IPAddress::Loopback());
    ASSERT_TRUE(IPAddress("255.255.255.255") == IPAddress::Broadcast());
    ASSERT_TRUE(IPAddress("255.255.255.255") == IPAddress::None());

    ASSERT_TRUE(IPAddress("0.0.0.0") == IPAddress("0.0.0.0"));
    ASSERT_TRUE(IPAddress("0.0.0.0") != IPAddress("0.0.0.1"));
    ASSERT_TRUE(IPAddress("0.0.0.0") < IPAddress("0.0.0.1"));
    ASSERT_TRUE(IPAddress("1.0.0.0") > IPAddress("0.1.1.1"));

    ASSERT_TRUE(IPAddress("0.0.0.1") >= IPAddress("0.0.0.0"));
    ASSERT_TRUE(IPAddress("0.0.0.0") >= IPAddress("0.0.0.0"));
    ASSERT_TRUE(IPAddress("0.0.0.0") >= IPAddress("0.0.0.0"));

    ASSERT_TRUE(IPAddress("0.0.0.0") <= IPAddress("0.0.0.0"));
    ASSERT_TRUE(IPAddress("0.0.0.0") <= IPAddress("0.0.0.1"));

    ASSERT_TRUE(IPAddress("0.0.0.255").IsBroadcast());
    ASSERT_TRUE(!IPAddress("255.0.0.0").IsBroadcast());

    ASSERT_TRUE(IPAddress("127.0.0.1").IsLoopback());
    ASSERT_TRUE(IPAddress("127.1.1.1").IsLoopback());
    ASSERT_TRUE(!IPAddress("128.0.0.1").IsLoopback());

    ASSERT_TRUE(IPAddress("10.0.0.1").IsPrivate());
    ASSERT_TRUE(IPAddress("192.168.0.1").IsPrivate());
    ASSERT_TRUE(IPAddress("172.16.0.1").IsPrivate());
    ASSERT_TRUE(IPAddress("172.31.0.1").IsPrivate());
    ASSERT_TRUE(!IPAddress("172.11.0.1").IsPrivate());
    ASSERT_TRUE(!IPAddress("172.32.0.1").IsPrivate());
}

TEST(Socket, SocketAddressInet4)
{
    SocketAddressInet4 address("192.168.0.1:1000");
}

TEST(Socket, SocketAddressInet)
{
    SocketAddressInet address("192.168.0.1:1000");
}

#ifdef __unix__
TEST(Socket, SocketAddressUnix)
{
    SocketAddressUnix address("/data/local.socket");
}
#endif

TEST(Socket, SocketAddressStorage)
{
    SocketAddressInet address("192.168.0.1:1000");
    SocketAddressStorage storage;
    storage = address;
    EXPECT_EQ(address.ToString(), storage.ToString());
}

TEST(Socket, SocketAddress)
{
    SocketAddressInet address("192.168.0.1:1000");
    SocketAddressStorage storage;
    SocketAddress& ss = storage;
    ss = address;
    EXPECT_EQ(address.ToString(), ss.ToString());
}
