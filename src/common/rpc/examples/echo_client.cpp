#include <signal.h>
#include <iostream>

#include "common/rpc/rpc.hpp"
#include "common/config/cflags.hpp"
#include "common/rpc/examples/echo_proxy.h"
#include "gtest/gtest.h"

using namespace Demo;

CFLAGS_DEFINE_FLAG(
    std::string, server_address, cflags::DefaultIs("127.0.0.1:20000"),
    "server address, in host:port format"
);

CFLAGS_DEFINE_FLAG(
    int, request_number, cflags::DefaultIs(500000),
    "request number"
);

CFLAGS_DEFINE_FLAG(
    bool, run_latency_test, cflags::DefaultIs(true),
    "run latency test"
);

CFLAGS_DEFINE_FLAG(
    bool, run_read_test, cflags::DefaultIs(true),
    "run read test"
);

CFLAGS_DEFINE_FLAG(
    bool, run_write_test, cflags::DefaultIs(true),
    "run write test"
);

CFLAGS_DEFINE_FLAG(
    bool, run_read_write_test, cflags::DefaultIs(true),
    "run read write test"
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
    EchoProxy echo;
    Rpc::GetRemoteObject(Flag_server_address, 2, echo);

    try
    {
        echo.Get(1024*1024, NULL, 10000000);

        std::string message = "hello";
        echo.Echo0(message, NULL, 1000000);
        ASSERT_TRUE(message == "hello");

        message = echo.Echo1(message);
        ASSERT_TRUE(message == "hello");

        echo.Echo2(message, message);
        ASSERT_TRUE(message == "hello");

        message = echo.Echo1(message);

        {
            Rpc::AsyncTokenOf<std::string> token;
            Rpc::Status_t status = echo.AsyncEcho1(message, &token, NULL, NULL, NULL);
            if (status == Rpc::Status_Pending)
                token.Wait();

            ASSERT_TRUE(token.Result() == "hello");
        }

        {
            std::string outmessage = "";
            Rpc::AsyncTokenOf<void> token;
            Rpc::Status_t status = echo.AsyncEcho2(message, outmessage, &token, NULL, NULL, NULL);
            if (status == Rpc::Status_Pending)
                token.Wait();

            ASSERT_TRUE(outmessage == "hello");
        }

        {
            CallContext c;
            c.message = "hello";
            Rpc::Status_t status = echo.AsyncEcho1(
                                       c.message,
                                       &c.token,
                                       RPC_MAKE_MEMBER_CALLBACK(CallContext, OnComplete), &c,
                                       NULL
                                   );

            if (status == Rpc::Status_Pending)
                c.token.Wait();

            ASSERT_TRUE(c.token.Result() == "hello");
        }

    }
    catch (Rpc::Exception& e)
    {
        std::cerr << e.what() << "\n";
    }
}

#include <common/system/time/timestamp.hpp>

TEST(Rpc, DISABLED_Performance)
{
    try
    {
        EchoProxy echo;
        Rpc::GetRemoteObject(Flag_server_address, 2, echo);

        long long t;
        std::string s;
#if 0
        // random size
        s.resize(1000 * 1024, 'A');
        t = GetTimeStamp();
        size_t total_size = 0;
        for (int i = 0; i < 100; ++i)
        {
            size_t size = rand() % s.size();
            echo.Get(size, NULL, 5000);
            total_size += size;
        }
        t = GetTimeStampInUs() - t;
        std::cout << "Random test throughput: " << size_t(total_size / (t / 1000000.0)) << " Bytes/s\n";
#endif
        // test latency
        Rpc::Status_t status;
        s.clear();
        if (Flag_run_latency_test.Value())
        {
            t = GetTimeStampInUs();
            for (int n = 0; n < Flag_request_number.Value(); ++n)
            {
                echo.Echo0(s, &status, 50000);
                if (status != Rpc::Status_Success)
                {
                    std::cout << StatusString(status);
                    fflush(stdout);
                }
            }
            t = GetTimeStampInUs() - t;
            std::cout << "latency test: " << 500000 / (t / 1000000.0) << "/s\n";
        }

        if (Flag_run_read_test.Value())
        {
            // test throughput
            s.resize(1000 * 1024, 'A');
            t = GetTimeStampInUs();
            for (int i = 0; i < 5000; ++i)
            {
                echo.Get(s.size(), &status, 5000);
                if (status != Rpc::Status_Success)
                {
                    std::cout << StatusString(status);
                    fflush(stdout);
                }
            }
            t = GetTimeStampInUs() - t;
            std::cout << "Read throughput: " << size_t(5000 * s.size() / t) << " MB/s\n";
        }

        if (Flag_run_write_test.Value())
        {
            // test throughput
            s.resize(1000 * 1024, 'A');
            t = GetTimeStampInUs();
            for (int i = 0; i < 5000; ++i)
            {
                echo.Set(s, &status, 5000);
                if (status != Rpc::Status_Success)
                {
                    std::cout << StatusString(status);
                    fflush(stdout);
                }
            }
            t = GetTimeStampInUs() - t;
            std::cout << "Write throughput: " << size_t(5000 * s.size() / t) << " MB/s\n";
        }

        if (Flag_run_read_write_test.Value())
        {
            // test throughput
            s.resize(1000 * 1024, 'A');
            t = GetTimeStampInUs();
            for (int i = 0; i < 5000; ++i)
            {
                echo.Echo0(s, &status, 5000);
                if (status != Rpc::Status_Success)
                {
                    std::cout << StatusString(status);
                    fflush(stdout);
                }
            }
            t = GetTimeStampInUs() - t;
            std::cout << "Read+Write throughput: " << size_t(2 * 5000 * s.size() / t) << " MB/s\n";
        }
    }
    catch (Rpc::Exception& exception)
    {
        std::cerr << "Fatal RPC error:" << exception.what() << "\n";
    }
}

#if 0
TEST(Rpc, DISABLED_Interactive)
{
    EchoProxy echo;
    Rpc::GetRemoteObject(Flag_server_address, 1, echo);
    std::cout << "> ";
    std::string s;
    while (std::getline(std::cin, s) && !s.empty())
    {
        echo.ToUpper(s);
        std::cout << "< " << s << "\n> ";
    }
}
#endif

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
