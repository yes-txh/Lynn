/*
 * rpc_server.cpp
 *
 *  Created on: 2010-7-23
 *      Author: braveli
 */

#include <common/rpc/rpc.hpp>

const Rpc::ObjectId_t ID_TEST = 1;
const Rpc::ObjectId_t ID_CALLBACK = 2;
const Rpc::ObjectId_t ID_CALLBACK_DEMO = 3;

//客户端的回调函数
RPC_BEGIN_PROXY_CLASS(RpcClient)
    RPC_PROXY_METHOD_0(void, ReturnClient, 1000)
RPC_END_PROXY_CLASS()

//定义server为client提供的接口：类名称以及函数名称
RPC_BEGIN_STUB_CLASS(RpcServer)
    RPC_STUB_METHOD_1(void,Test1,Rpc::In<int>)
    RPC_STUB_METHOD_1(int,Test2,int )
    RPC_STUB_METHOD_1(std::string,Test3,Rpc::In<std::string>)
    RPC_STUB_METHOD_2(int,Test4,std::string,std::string )
    RPC_STUB_METHOD_2(int,Test5,Rpc::In<const std::string&>,Rpc::Out<std::string&> )
RPC_END_STUB_CLASS()

//定义server接口函数
RPC_BEGIN_STUB_DISPATCH(RpcServer)
    RPC_STUB_DISPATCH(Test1, 1)
    RPC_STUB_DISPATCH(Test2, 1)
    RPC_STUB_DISPATCH(Test3, 1)
    RPC_STUB_DISPATCH(Test4, 2)
    RPC_STUB_DISPATCH(Test5, 2)
RPC_END_STUB_DISPATCH()

class RpcServer : public RpcServerStub<RpcServer>
{
public:
    //构造函数
    RpcServer(Rpc::ObjectId_t id) :RpcServerStub<RpcServer>(id)
    {
    }

    void Test1( int num )
    {
        printf( "test1 num(client to server): %d\n",num );

        num += 10;
    }

    int Test2( int num )
    {
        printf( "test2 num(client to server): %d\n",num );
        num += 10;
        return num + 10 ;
    }

    std::string Test3( std::string client_str )
    {
        std::string server_str;
        printf( "%s\n",client_str.c_str() );

        server_str = "test3 hello client";
        return server_str;
    }

    int Test4( std::string client_str,std::string server_str )
    {
        printf( "%s\n",client_str.c_str() );

        server_str = "test4 hello! server to client";
        return 4;
    }

    int Test5( const std::string& client_str,std::string& server_str )
    {
        printf( "%s\n",client_str.c_str() );

        server_str = "test5 hello! server to client";
        return 5;
    }
};

int main()
{
    Rpc::Initialize();

    RpcServer rpc_server( ID_CALLBACK_DEMO );

    //rpc服务器监听
    Rpc::Status_t status = Rpc::Listen("127.0.0.1:20000");
    if (Rpc::Status_IsError(status))
    {
        fprintf(stderr, "Listen error: %s\n", Rpc::StatusString(status));
        return EXIT_FAILURE;
    }

    //循环执行
    for (;;)
    {
        sleep(1);
    }
    return 0;
}
