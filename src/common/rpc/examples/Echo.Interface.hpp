RPC_BEGIN_INTERFACE(Echo)
/// 参数不做任何改动返回
RPC_METHOD_1(void,        Echo0, Rpc::InOut<std::string&>, 100)

/// 返回源串
RPC_METHOD_1(std::string, Echo1, Rpc::In<const std::string&>, 100)

/// 返回指定长度的空串
RPC_METHOD_1(std::string, Get, size_t, 100)

/// 发送字符串
RPC_METHOD_1(void, Set, const std::string&, 100)

/// 返回在第二个参数中
RPC_ASYNC_METHOD_2(void,  Echo2, Rpc::In<const std::string&>, Rpc::Out<std::string&>, 100)

/// 转为大写
RPC_METHOD_1(void,        ToUpper, Rpc::InOut<std::string&>, 100)
RPC_END_INTERFACE()
