/// @example test.cpp
/// ��ʾ���ʹ�� RPC ��ܵĲ��Գ���

#include <common/rpc/rpc.hpp>

#include "common/rpc/examples/callback_proxy.h"
#include "common/rpc/examples/callback_demo_stub.h"
#include "common/rpc/examples/callback.hpp"

class CallbackDemo : public CallbackDemoStub<CallbackDemo>
{
public:
    CallbackDemo(Rpc::ObjectId_t id) :
        CallbackDemoStub<CallbackDemo>(id)
    {
    }
    void RegisterCallback(const CallbackProxy& callback)
    {
        printf("CallbackDemo::RegisterCallback\n");
        m_Callback = callback;
    }
    void RequestCallback()
    {
        printf("CallbackDemo::RequestCallback\n");
        m_Callback.AsyncOnCallback(NULL, NULL, NULL, NULL);
    }
private:
    CallbackProxy m_Callback;
};

int main()
{
    Rpc::Initialize();

    CallbackDemo demo(ID_CALLBACK_DEMO);
    Rpc::Status_t status = Rpc::Listen("127.0.0.1:20000");
    if (Rpc::Status_IsError(status))
    {
        fprintf(stderr, "Listen error: %s\n", Rpc::StatusString(status));
        return EXIT_FAILURE;
    }

    for (;;)
    {
        sleep(1);
    }
    return 0;
}
