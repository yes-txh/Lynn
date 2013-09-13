#include <signal.h>
#include <iostream>
#include <fstream>

#include "common/rpc/rpc.hpp"
#include "common/config/cflags.hpp"

#include "common/rpc/examples/proto_proxy.h"
#include "common/rpc/examples/addressbook.pb.h"
#include "gtest/gtest.h"

using namespace Demo;
using namespace std;

CFLAGS_DEFINE_FLAG(
    std::string, server_address, cflags::DefaultIs("127.0.0.1:40000"),
    "server address, in host:port format"
);

// Iterates though all people in the AddressBook and prints info about them.
void ListPeople(const AddressBook& address_book)
{
    for (int i = 0; i < address_book.person_size(); i++)
    {
        const Person& person = address_book.person(i);
        cout << "Person ID: " << person.id() << endl;
        cout << "  Name: " << person.name() << endl;
        if (person.has_email())
        {
            cout << "  E-mail address: " << person.email() << endl;
        }

        for (int j = 0; j < person.phone_size(); j++)
        {
            const Person::PhoneNumber& phone_number = person.phone(j);
            switch (phone_number.type())
            {
                case Person::MOBILE:
                    cout << "  Mobile phone #: ";
                    break;
                case Person::HOME:
                    cout << "  Home phone #: ";
                    break;
                case Person::WORK:
                    cout << "  Work phone #: ";
                    break;
            }
            cout << phone_number.number() << endl;
        }
    }
}

TEST(Rpc, Echo)
{
    ProtobufTestProxy echo;
    Rpc::GetRemoteObject(Flag_server_address, 1, echo);

    try
    {
        AddressBook message, message1;
        {
            // Read the existing address book.
            std::fstream input("address.dat", std::ios::in | std::ios::binary);
            if (!message.ParseFromIstream(&input))
            {
                std::cerr << "Failed to parse address book." << std::endl;
            }
        }
        echo.Echo0(message);            // echo0

        message1 = echo.Echo1(message);
        std::cout << "============echo1 result=========" << endl;
        ListPeople(message1);
        std::cout << "============echo1 result=========" << endl;

        message1.Clear();
        echo.Echo2(message, message1);
        std::cout << "============echo2 result=========" << endl;
        ListPeople(message1);
        std::cout << "============echo2 result=========" << endl;

        {
            Rpc::AsyncTokenOf<AddressBook> token;
            Rpc::Status_t status = echo.AsyncEcho1(message, &token, NULL, NULL, NULL);
            if (status == Rpc::Status_Pending)
                token.Wait();
            std::cout << "============echo3 result=========" << endl;
            ListPeople(token.Result());
            std::cout << "============echo3 result=========" << endl;
        }

        {
            AddressBook outmessage;
            Rpc::AsyncTokenOf<void> token;  // echo 4
            Rpc::Status_t status = echo.AsyncEcho2(message, outmessage, &token, NULL, NULL, NULL);
            if (status == Rpc::Status_Pending)
                token.Wait();
        }
    }
    catch (Rpc::Exception& e)
    {
        std::cerr << e.what() << "\n";
    }
}

#include <common/system/time/timestamp.hpp>

TEST(Rpc, Performance)
{
    try
    {
        ProtobufTestProxy echo;
        Rpc::GetRemoteObject(Flag_server_address, 1, echo);

        long long t;
        AddressBook s;
        // test latency
        for (int n = 0; n < 100; ++n)
        {
            t = GetTimeStampInUs();
            for (int i = 0; i < 10000; ++i)
            {
                echo.Echo0(s);
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
