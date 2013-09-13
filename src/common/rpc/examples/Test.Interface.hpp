RPC_BEGIN_INTERFACE(Test)
RPC_METHOD_0(void, Nop, 1000)
RPC_METHOD_0(int, Return0, 1000)
RPC_METHOD_1(int, Next, Rpc::In<int>, 1000)
RPC_METHOD_1(void, Inc, int&, 1000)
RPC_METHOD_2(int, Add2, int, int, 1000)
RPC_METHOD_3(int, Add3, int, int, int, 1000)
RPC_METHOD_4(int, Add4, int, int, int, int, 1000)
RPC_METHOD_5(int, Add5, int, int, int, int, int, 1000)
RPC_METHOD_6(int, Add6, int, int, int, int, int, int, 1000)

RPC_ASYNC_METHOD_0(void, A_Nop, 1000)
RPC_ASYNC_METHOD_0(int, A_Return0, 1000)
RPC_ASYNC_METHOD_1(int, A_Next, int, 1000)
RPC_ASYNC_METHOD_1(void, A_Inc, int&, 1000)
RPC_ASYNC_METHOD_2(int, A_Add2, int, int, 1000)
RPC_ASYNC_METHOD_3(int, A_Add3, int, int, int, 1000)
RPC_ASYNC_METHOD_4(int, A_Add4, int, int, int, int, 1000)
RPC_ASYNC_METHOD_5(int, A_Add5, int, int, int, int, int, 1000)
RPC_ASYNC_METHOD_6(int, A_Add6, int, int, int, int, int, int, 1000)
RPC_ASYNC_METHOD_4(bool, Div, int, int, Rpc::Out<int&>, Rpc::Out<int&>, 1000)
RPC_END_INTERFACE()
