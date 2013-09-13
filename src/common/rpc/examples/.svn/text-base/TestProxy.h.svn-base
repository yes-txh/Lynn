
// auto generated code, don't edit, edit corresponding .idl please

#ifndef TEST_PROXY_H
#define TEST_PROXY_H

class TestProxy : public Rpc::ProxyImpl
{
public:
	static const char* GetClassName() { return "TestProxy"; }

	Rpc::Status_t AsyncNop(
		Rpc::AsyncTokenOf<void>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke0("Nop" "_0", method_id, token, callback, context, param, timeout);
	}

	void Nop(Rpc::Status_t* status = NULL, int timeout = 1000)
	{
		Rpc::AsyncTokenOf<void> async_token;
		return SyncInvokeReturn(
			GetClassName(), "Nop",
			AsyncNop(&async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncReturn0(
		Rpc::AsyncTokenOf<int>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke0("Return0" "_0", method_id, token, callback, context, param, timeout);
	}

	int Return0(Rpc::Status_t* status = NULL, int timeout = 1000)
	{
		Rpc::AsyncTokenOf<int> async_token;
		return SyncInvokeReturn(
			GetClassName(), "Return0",
			AsyncReturn0(&async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncNext(
		Rpc::In<int>  a1,
		Rpc::AsyncTokenOf<int>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke1<Rpc::In<int> >("Next" "_1", method_id, a1, token, callback, context, param, timeout);
	}

	int Next(Rpc::In<int>  a1, Rpc::Status_t* status = NULL, int timeout = 1000)
	{
		Rpc::AsyncTokenOf<int> async_token;
		return SyncInvokeReturn(
			GetClassName(), "Next",
			AsyncNext(a1, &async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncInc(
		int& a1,
		Rpc::AsyncTokenOf<void>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke1<int&>("Inc" "_1", method_id, a1, token, callback, context, param, timeout);
	}

	void Inc(int& a1, Rpc::Status_t* status = NULL, int timeout = 1000)
	{
		Rpc::AsyncTokenOf<void> async_token;
		return SyncInvokeReturn(
			GetClassName(), "Inc",
			AsyncInc(a1, &async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncAdd2(
		int a1,
		int a2,
		Rpc::AsyncTokenOf<int>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke2<
			int,
			int
		>(
			"Add2" "_2", method_id,
			a1, a2,
			token, callback, context, param,
			timeout
		);
	}

	int Add2(
		int a1,
		int a2,
		Rpc::Status_t* status = NULL,
		int timeout = 1000
	)
	{
		Rpc::AsyncTokenOf<int> async_token;
		return SyncInvokeReturn(
			GetClassName(), "Add2",
			AsyncAdd2(a1, a2, &async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncAdd3(
		int a1,
		int a2,
		int a3,
		Rpc::AsyncTokenOf<int>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke3<
			int,
			int,
			int
		>(
			"Add3" "_3", method_id,
			a1, a2, a3,
			token, callback, context, param,
			timeout
		);
	}

	int Add3(
		int a1,
		int a2,
		int a3,
		Rpc::Status_t* status = NULL,
		int timeout = 1000)
	{
		Rpc::AsyncTokenOf<int> async_token;
		return SyncInvokeReturn(
			GetClassName(), "Add3",
			AsyncAdd3(a1, a2, a3, &async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncAdd4(
		int a1,
		int a2,
		int a3,
		int a4,
		Rpc::AsyncTokenOf<int>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke4<
			int,
			int,
			int,
			int
		>(
			"Add4" "_4", method_id,
			a1, a2, a3, a4,
			token, callback, context, param, timeout
		);
	}

	int Add4(
		int a1,
		int a2,
		int a3,
		int a4,
		Rpc::Status_t* status = NULL, int timeout = 1000
	)
	{
		Rpc::AsyncTokenOf<int> async_token;
		return SyncInvokeReturn(
			GetClassName(), "Add4",
			AsyncAdd4(a1, a2, a3, a4, &async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncAdd5(
		int a1,
		int a2,
		int a3,
		int a4,
		int a5,
		Rpc::AsyncTokenOf<int>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke5<
			int,
			int,
			int,
			int,
			int
		>(
			"Add5" "_5", method_id,
			a1, a2, a3, a4, a5,
			token, callback, context, param,
			timeout
		);
	}

	int Add5(
		int a1,
		int a2,
		int a3,
		int a4,
		int a5,
		Rpc::Status_t* status = NULL,
		int timeout = 1000
	)
	{
		Rpc::AsyncTokenOf<int> async_token;
		return SyncInvokeReturn(
			GetClassName(), "Add5",
			AsyncAdd5(
				a1, a2, a3, a4, a5,
				&async_token,
				NULL, NULL, NULL,
				-1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncAdd6(
		int a1,
		int a2,
		int a3,
		int a4,
		int a5,
		int a6,
		Rpc::AsyncTokenOf<int>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke6<
			int,
			int,
			int,
			int,
			int,
			int
		>(
			"Add6" "_6", method_id,
			 a1, a2, a3, a4, a5, a6,
			 token, callback, context, param,
			 timeout
		);
	}

	int Add6(
		int a1,
		int a2,
		int a3,
		int a4,
		int a5,
		int a6,
		Rpc::Status_t* status = NULL,
		int timeout = 1000
	)
	{
		Rpc::AsyncTokenOf<int> async_token;
		return SyncInvokeReturn(
			GetClassName(), "Add6",
			AsyncAdd6(
				a1, a2, a3, a4, a5, a6,
				&async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncA_Nop(
		Rpc::AsyncTokenOf<void>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke0("A_Nop" "_0", method_id, token, callback, context, param, timeout);
	}

	void A_Nop(Rpc::Status_t* status = NULL, int timeout = 1000)
	{
		Rpc::AsyncTokenOf<void> async_token;
		return SyncInvokeReturn(
			GetClassName(), "A_Nop",
			AsyncA_Nop(&async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncA_Return0(
		Rpc::AsyncTokenOf<int>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke0("A_Return0" "_0", method_id, token, callback, context, param, timeout);
	}

	int A_Return0(Rpc::Status_t* status = NULL, int timeout = 1000)
	{
		Rpc::AsyncTokenOf<int> async_token;
		return SyncInvokeReturn(
			GetClassName(), "A_Return0",
			AsyncA_Return0(&async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncA_Next(
		Rpc::In<int>  a1,
		Rpc::AsyncTokenOf<int>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke1<Rpc::In<int> >("A_Next" "_1", method_id, a1, token, callback, context, param, timeout);
	}

	int A_Next(Rpc::In<int>  a1, Rpc::Status_t* status = NULL, int timeout = 1000)
	{
		Rpc::AsyncTokenOf<int> async_token;
		return SyncInvokeReturn(
			GetClassName(), "A_Next",
			AsyncA_Next(a1, &async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncA_Inc(
		int& a1,
		Rpc::AsyncTokenOf<void>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke1<int&>("A_Inc" "_1", method_id, a1, token, callback, context, param, timeout);
	}

	void A_Inc(int& a1, Rpc::Status_t* status = NULL, int timeout = 1000)
	{
		Rpc::AsyncTokenOf<void> async_token;
		return SyncInvokeReturn(
			GetClassName(), "A_Inc",
			AsyncA_Inc(a1, &async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncA_Add2(
		int a1,
		int a2,
		Rpc::AsyncTokenOf<int>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke2<
			int,
			int
		>(
			"A_Add2" "_2", method_id,
			a1, a2,
			token, callback, context, param,
			timeout
		);
	}

	int A_Add2(
		int a1,
		int a2,
		Rpc::Status_t* status = NULL,
		int timeout = 1000
	)
	{
		Rpc::AsyncTokenOf<int> async_token;
		return SyncInvokeReturn(
			GetClassName(), "A_Add2",
			AsyncA_Add2(a1, a2, &async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncA_Add3(
		int a1,
		int a2,
		int a3,
		Rpc::AsyncTokenOf<int>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke3<
			int,
			int,
			int
		>(
			"A_Add3" "_3", method_id,
			a1, a2, a3,
			token, callback, context, param,
			timeout
		);
	}

	int A_Add3(
		int a1,
		int a2,
		int a3,
		Rpc::Status_t* status = NULL,
		int timeout = 1000)
	{
		Rpc::AsyncTokenOf<int> async_token;
		return SyncInvokeReturn(
			GetClassName(), "A_Add3",
			AsyncA_Add3(a1, a2, a3, &async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncA_Add4(
		int a1,
		int a2,
		int a3,
		int a4,
		Rpc::AsyncTokenOf<int>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke4<
			int,
			int,
			int,
			int
		>(
			"A_Add4" "_4", method_id,
			a1, a2, a3, a4,
			token, callback, context, param, timeout
		);
	}

	int A_Add4(
		int a1,
		int a2,
		int a3,
		int a4,
		Rpc::Status_t* status = NULL, int timeout = 1000
	)
	{
		Rpc::AsyncTokenOf<int> async_token;
		return SyncInvokeReturn(
			GetClassName(), "A_Add4",
			AsyncA_Add4(a1, a2, a3, a4, &async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncA_Add5(
		int a1,
		int a2,
		int a3,
		int a4,
		int a5,
		Rpc::AsyncTokenOf<int>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke5<
			int,
			int,
			int,
			int,
			int
		>(
			"A_Add5" "_5", method_id,
			a1, a2, a3, a4, a5,
			token, callback, context, param,
			timeout
		);
	}

	int A_Add5(
		int a1,
		int a2,
		int a3,
		int a4,
		int a5,
		Rpc::Status_t* status = NULL,
		int timeout = 1000
	)
	{
		Rpc::AsyncTokenOf<int> async_token;
		return SyncInvokeReturn(
			GetClassName(), "A_Add5",
			AsyncA_Add5(
				a1, a2, a3, a4, a5,
				&async_token,
				NULL, NULL, NULL,
				-1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncA_Add6(
		int a1,
		int a2,
		int a3,
		int a4,
		int a5,
		int a6,
		Rpc::AsyncTokenOf<int>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke6<
			int,
			int,
			int,
			int,
			int,
			int
		>(
			"A_Add6" "_6", method_id,
			 a1, a2, a3, a4, a5, a6,
			 token, callback, context, param,
			 timeout
		);
	}

	int A_Add6(
		int a1,
		int a2,
		int a3,
		int a4,
		int a5,
		int a6,
		Rpc::Status_t* status = NULL,
		int timeout = 1000
	)
	{
		Rpc::AsyncTokenOf<int> async_token;
		return SyncInvokeReturn(
			GetClassName(), "A_Add6",
			AsyncA_Add6(
				a1, a2, a3, a4, a5, a6,
				&async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

	Rpc::Status_t AsyncDiv(
		int a1,
		int a2,
		Rpc::Out<int&>  a3,
		Rpc::Out<int&>  a4,
		Rpc::AsyncTokenOf<bool>* token = NULL,
		Rpc::AsyncCallback callback = NULL, void* context = NULL, void* param = NULL,
		int timeout = 1000
	)
	{
		static int method_id = -1;
		return ProxyAsyncInvoke4<
			int,
			int,
			Rpc::Out<int&> ,
			Rpc::Out<int&> 
		>(
			"Div" "_4", method_id,
			a1, a2, a3, a4,
			token, callback, context, param, timeout
		);
	}

	bool Div(
		int a1,
		int a2,
		Rpc::Out<int&>  a3,
		Rpc::Out<int&>  a4,
		Rpc::Status_t* status = NULL, int timeout = 1000
	)
	{
		Rpc::AsyncTokenOf<bool> async_token;
		return SyncInvokeReturn(
			GetClassName(), "Div",
			AsyncDiv(a1, a2, a3, a4, &async_token, NULL, NULL, NULL, -1),
			async_token, timeout, status
		);
	}

};
#endif// end of define TEST_PROXY_H
