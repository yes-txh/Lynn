RPC_BEGIN_INTERFACE(CallbackDemo)
RPC_METHOD_1(void, RegisterCallback, RPC_CALLBACK(Callback), 1000)
RPC_METHOD_0(void, RequestCallback, 1000)
RPC_END_INTERFACE()

