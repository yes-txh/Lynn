#include <signal.h>
#include <iostream>
#include <string>
#include "common/rpc/rpc.hpp"
#include "common/config/cflags.hpp"
#include "common/system/concurrency/atomic/atomic.hpp"
#include "common/rpc/examples/nested_proxy.h"
#include "gtest/gtest.h"

using namespace Demo;

Atomic<unsigned int> ResponseCount = 0;

CFLAGS_DEFINE_FLAG(
    std::string, server_address, cflags::DefaultIs("127.0.0.1:10000"),
    "server address, in host:port format"
);


struct CallContext
{
    std::string message;
    Rpc::AsyncTokenOf<std::string> token;
    void OnComplete(Rpc::Status_t, void*)
    {
        ResponseCount++;
    }
};

TEST(Rpc, DISABLED_Test)
{
    NestedTestProxy test;
    Rpc::GetRemoteObject(Flag_server_address, 1, test);

    try
    {
        std::string message = "hello";
        std::string outmessage = "";

        for (size_t i = 0; i < 100000; i++)
        {
            Rpc::AsyncTokenOf<void> token;
            test.AsyncEcho(message,
                    outmessage,
                    &token,
                    NULL,
                    NULL,
                    NULL);
            while (token.Status() == Rpc::Status_Pending)
                token.Wait();
            if (token.Status() != Rpc::Status_Success)
            {
                std::cout << outmessage << std::endl;
                std::cout << token.Status() << std::endl;
            }
            //ASSERT_TRUE(outmessage == "hello");
        }
        Rpc::AsyncTokenOf<void> token;
        test.AsyncEcho(message,
                outmessage,
                &token,
                NULL,
                NULL,
                NULL);
        while (token.Status() == Rpc::Status_Pending)
            token.Wait();
        std::cout << outmessage << std::endl;
        std::cout << token.Status() << std::endl;
    }
    catch (Rpc::Exception& e)
    {
        std::cerr << e.what() << "\n";
    }
    sleep(2);
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
