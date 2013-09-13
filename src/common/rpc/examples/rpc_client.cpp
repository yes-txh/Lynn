/*
 * rpc_client.cpp
 *
 *  Created on: 2010-7-23
 *      Author: braveli
 */

#include <common/rpc/rpc.hpp>
#include <common/rpc/proxy.hpp>

const Rpc::ObjectId_t ID_TEST = 1;
const Rpc::ObjectId_t ID_CALLBACK = 2;
const Rpc::ObjectId_t ID_CALLBACK_DEMO = 3;

using namespace Rpc;
/*
 * client给server提供的结果返回接口：类名称和函数
 */
RPC_BEGIN_STUB_CLASS(RpcClient)
    RPC_STUB_METHOD_0(void, ReturnClient)
RPC_END_STUB_CLASS()

/*
 * 结果返回接口的具体实现
 */
RPC_BEGIN_STUB_DISPATCH(RpcClient)
    RPC_STUB_DISPATCH(ReturnClient, 0)
RPC_END_STUB_DISPATCH()

class RpcClient : public RpcClientStub<RpcClient>
{
public:
    //构造函数
    RpcClient(const Rpc::ObjectId_t& id) :RpcClientStub<RpcClient>(id)
    {
    }

    void ReturnClient()
    {
        printf("server return result to client\n");
    }
};

//服务器端给客户端提供的接口函数
RPC_BEGIN_PROXY_CLASS(RpcServer)
    RPC_PROXY_METHOD_1(void,Test1,Rpc::In<int>,1000 )
    RPC_PROXY_METHOD_1(int,Test2,int,1000 )
    RPC_PROXY_METHOD_1(std::string,Test3,Rpc::In<std::string>,1000 )
    RPC_PROXY_METHOD_2(int,Test4,std::string,std::string,1000 )
    RPC_PROXY_METHOD_2(int,Test5,Rpc::In<const std::string&>,Rpc::Out<std::string&>,1000 )
RPC_END_PROXY_CLASS()

int main()
{
    Rpc::Initialize();

    RpcServerProxy demo_proxy;
    //连接服务器端
    Rpc::GetRemoteObject("127.0.0.1:20000", ID_CALLBACK_DEMO, demo_proxy);

    int in_value = 99;
    int ok;

    RpcClient rpc_client(ID_CALLBACK);

    //test 1
    printf( "test1,in_value init num is :%d\n",in_value );
    demo_proxy.Test1( in_value );
    printf( "test1,in_value final num is :%d\n",in_value );

    //test 2
    in_value = 100;
    ok = demo_proxy.Test2( in_value );
    printf( "test2,ok value is: %d\n",ok );

    //test 3
    std::string client_str = "test3 hello! client to server";
    std::string server_str = "hello!";
    server_str = demo_proxy.Test3( client_str );
    printf("%s\n",server_str.c_str() );

    //test 4
    client_str = "test4 hello! client to server";
    ok = demo_proxy.Test4( client_str,server_str );
    printf("test4 server return num: %d\n%s\n",ok,server_str.c_str() );

    //test 5
    client_str = "test5 hello! client to server";
    server_str ="hello";
    ok = demo_proxy.Test5( client_str,server_str, NULL, 100000);
    printf("test5 server return num: %d\n%s\n",ok,server_str.c_str() );

    return 0;
}
