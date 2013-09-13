#include <signal.h>
#include <iostream>

#include "common/rpc/rpc.hpp"
#include "common/config/cflags.hpp"
#include "common/system/time/timestamp.hpp"
#include "common/rpc/examples/pointer_proxy.h"
#include "gtest/gtest.h"

using namespace Demo;

CFLAGS_DEFINE_FLAG(
    std::string, server_address, cflags::DefaultIs("127.0.0.1:30000"),
    "server address, in host:port format"
);

struct CallContext
{
    std::string message;
    Rpc::AsyncTokenOf<std::string> token;
    void OnComplete(Rpc::Status_t, void*)
    {
        std::string& result = token.Result();
        std::cout << "[In Callback]" << result << std::endl;
    }
};

TEST(Rpc, DISABLED_Echo)
{
    PointerProxy echo;
    Rpc::GetRemoteObject(Flag_server_address, 1, echo);

    try
    {
        std::string message = "hello";
        std::string s1 = echo.Echo1(message);
        ASSERT_TRUE(s1 == "hello");

        echo.ToUpper(&message);
        ASSERT_TRUE(message == "HELLO");

        std::string result;
        echo.Echo3(message, &result);
        ASSERT_TRUE(result == "HELLO");

        {
            std::string outmessage = "";
            Rpc::AsyncTokenOf<void> token;
            Rpc::Status_t status = echo.AsyncEcho2(message, &outmessage, &token, NULL, NULL, NULL);
            if (status == Rpc::Status_Pending)
                token.Wait();

            ASSERT_TRUE(outmessage == "HELLO");
        }
    }
    catch (Rpc::Exception& e)
    {
        std::cerr << e.what() << "\n";
    }
}


TEST(Rpc, DISABLED_Performance)
{
    try
    {
        PointerProxy echo;
        Rpc::GetRemoteObject(Flag_server_address, 1, echo);

        long long t;
        std::string s = "hello";
        for (int n = 0; n < 100; ++n)
        {
            t = GetTimeStampInUs();
            for (int i = 0; i < 10000; ++i)
            {
                std::string s1 = echo.Echo1(s, NULL, 50000);
                //ASSERT_TRUE(s == s1);
            }
            t = GetTimeStampInUs() - t;
            std::cout << "latency test: " << 10000 / (t / 1000000.0) << "/s\n";
        }
    }
    catch (Rpc::Exception& exception)
    {
        std::cerr << "Fatal RPC error:" << exception.what() << "\n";
    }
}

void signalhandler(int signo)
{
    exit(128+signo);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    if (!CFlags::ParseCommandLine(argc, argv))
        return EXIT_FAILURE;

    Rpc::Initialize();

    signal(SIGINT, signalhandler);

    return RUN_ALL_TESTS();
}
