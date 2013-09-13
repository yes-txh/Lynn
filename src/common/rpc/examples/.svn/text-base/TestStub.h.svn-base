
// auto generated code, don't edit, edit corresponding .idl please

#ifndef TEST_STUB_H
#define TEST_STUB_H

template <typename T>
class TestStub: public Rpc::StubImpl
{
	typedef T ImplType;
	typedef TestStub ThisType;
	typedef Rpc::StubImpl BaseType;
public:
	TestStub() {}
	TestStub(Rpc::ObjectId_t id) : Rpc::StubImpl(id) {}
	virtual const typename Rpc::StubImpl::DispatchTable& GetDispatchTable() const;

	Rpc::Status_t StubNop(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		return this->InvokeStub0<void, ImplType>(
			endpoint, id, method_id, buffer,
			&ImplType::Nop,
			typename Rpc::ReturnTypeTag<void>::Type()
		);
	}

	Rpc::Status_t StubReturn0(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		return this->InvokeStub0<int, ImplType>(
			endpoint, id, method_id, buffer,
			&ImplType::Return0,
			typename Rpc::ReturnTypeTag<int>::Type()
		);
	}

	Rpc::Status_t StubNext(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		return this->InvokeStub1<
			int, ImplType,
			Rpc::In<int> 
		>(
			endpoint, id, method_id, buffer,
			&ImplType::Next,
			typename Rpc::ReturnTypeTag<int>::Type()
		);
	}

	Rpc::Status_t StubInc(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		return this->InvokeStub1<
			void, ImplType,
			int&
		>(
			endpoint, id, method_id, buffer,
			&ImplType::Inc,
			typename Rpc::ReturnTypeTag<void>::Type()
		);
	}

	Rpc::Status_t StubAdd2(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		return this->InvokeStub2<
			int, ImplType,
			int,
			int
		>(
			endpoint, id, method_id, buffer,
			&ImplType::Add2,
			typename Rpc::ReturnTypeTag<int>::Type()
		);
	}

	Rpc::Status_t StubAdd3(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		return this->InvokeStub3<
			int, ImplType,
			int,
			int,
			int
		>(
			endpoint, id, method_id, buffer,
			&ImplType::Add3,
			typename Rpc::ReturnTypeTag<int>::Type()
		);
	}

	Rpc::Status_t StubAdd4(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		return this->InvokeStub4<
			int, ImplType,
			int,
			int,
			int,
			int
		>(
			endpoint, id, method_id, buffer,
			&ImplType::Add4,
			typename Rpc::ReturnTypeTag<int>::Type()
		);
	}

	Rpc::Status_t StubAdd5(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		return this->InvokeStub5<
			int, ImplType,
			int,
			int,
			int,
			int,
			int
		>(
			endpoint, id, method_id, buffer,
			&ImplType::Add5,
			typename Rpc::ReturnTypeTag<int>::Type()
		);
	}

	Rpc::Status_t StubAdd6(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		return this->InvokeStub6<
			int, ImplType,
			int,
			int,
			int,
			int,
			int,
			int
		>(
			endpoint, id, method_id, buffer,
			&ImplType::Add6,
			typename Rpc::ReturnTypeTag<int>::Type()
		);
	}

	Rpc::Status_t StubA_Nop(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		Rpc::InvokeContext context(endpoint, id, method_id);
		context.SetReturnHandler(static_cast<void*>(NULL));
		return (static_cast<ImplType*>(this))->A_Nop(context, buffer);
	}

	Rpc::Status_t StubA_Return0(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		Rpc::InvokeContext context(endpoint, id, method_id);
		context.SetReturnHandler(static_cast<int*>(NULL));
		return (static_cast<ImplType*>(this))->A_Return0(context, buffer);
	}

	Rpc::Status_t StubA_Next(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer)
	{
		Rpc::InvokeContext context(endpoint, id, method_id);
		context.SetParameterHandlers<Rpc::In<int> >();
		context.SetReturnHandler(static_cast<int*>(NULL));
		return (static_cast<ImplType*>(this))->A_Next(context, buffer);
	}

	Rpc::Status_t StubA_Inc(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer)
	{
		Rpc::InvokeContext context(endpoint, id, method_id);
		context.SetParameterHandlers<int&>();
		context.SetReturnHandler(static_cast<void*>(NULL));
		return (static_cast<ImplType*>(this))->A_Inc(context, buffer);
	}

	Rpc::Status_t StubA_Add2(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		Rpc::InvokeContext context(endpoint, id, method_id);
		context.SetParameterHandlers<
			int,
			int
		>();
		context.SetReturnHandler(static_cast<int*>(NULL));
		return (static_cast<ImplType*>(this))->A_Add2(context, buffer);
	}

	Rpc::Status_t StubA_Add3(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		Rpc::InvokeContext context(endpoint, id, method_id);
		context.SetParameterHandlers<
			int,
			int,
			int
		>();
		context.SetReturnHandler(static_cast<int*>(NULL));
		return (static_cast<ImplType*>(this))->A_Add3(context, buffer);
	}

	Rpc::Status_t StubA_Add4(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		Rpc::InvokeContext context(endpoint, id, method_id);
		context.SetParameterHandlers<
			int,
			int,
			int,
			int
		>();
		
		context.SetReturnHandler(static_cast<int*>(NULL));
		return (static_cast<ImplType*>(this))->A_Add4(context, buffer);
	}

	Rpc::Status_t StubA_Add5(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		Rpc::InvokeContext context(endpoint, id, method_id);
		context.SetParameterHandlers<
			int,
			int,
			int,
			int,
			int
		>();
		context.SetReturnHandler(static_cast<int*>(NULL));
		return (static_cast<ImplType*>(this))->A_Add5(context, buffer);
	}

	Rpc::Status_t StubA_Add6(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
		)
	{
		Rpc::InvokeContext context(endpoint, id, method_id);
		context.SetParameterHandlers<
			int,
			int,
			int,
			int,
			int,
			int
		>();
		context.SetReturnHandler(static_cast<int*>(NULL));
		return (static_cast<ImplType*>(this))->A_Add6(context, buffer);
	}

	Rpc::Status_t StubDiv(
		const Rpc::EndPoint_t& endpoint,
		Rpc::InvokeId_t id,
		int method_id,
		const Rpc::Buffer& buffer
	)
	{
		Rpc::InvokeContext context(endpoint, id, method_id);
		context.SetParameterHandlers<
			int,
			int,
			Rpc::Out<int&> ,
			Rpc::Out<int&> 
		>();
		
		context.SetReturnHandler(static_cast<bool*>(NULL));
		return (static_cast<ImplType*>(this))->Div(context, buffer);
	}

};

template <typename T>
const typename Rpc::StubImpl::DispatchTable& TestStub<T>::GetDispatchTable() const
{
	static const typename Rpc::StubImpl::DispatchEntry entries[] = 
	{
		{ "Nop" "_ArgCount", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubNop) },
		{ "Return0" "_ArgCount", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubReturn0) },
		{ "Next" "_1", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubNext) },
		{ "Inc" "_1", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubInc) },
		{ "Add2" "_2", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubAdd2) },
		{ "Add3" "_3", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubAdd3) },
		{ "Add4" "_4", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubAdd4) },
		{ "Add5" "_5", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubAdd5) },
		{ "Add6" "_6", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubAdd6) },
		{ "A_Nop" "_ArgCount", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubA_Nop) },
		{ "A_Return0" "_ArgCount", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubA_Return0) },
		{ "A_Next" "_1", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubA_Next) },
		{ "A_Inc" "_1", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubA_Inc) },
		{ "A_Add2" "_2", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubA_Add2) },
		{ "A_Add3" "_3", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubA_Add3) },
		{ "A_Add4" "_4", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubA_Add4) },
		{ "A_Add5" "_5", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubA_Add5) },
		{ "A_Add6" "_6", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubA_Add6) },
		{ "Div" "_4", static_cast<Rpc::StubImpl::StubMethod>(&ThisType::StubDiv) },
		{ NULL, NULL }
	};

	static const typename Rpc::StubImpl::DispatchTable table(entries, sizeof(entries)/sizeof(entries[0]) - 1);
	return table;
};
#endif// end of define TEST_STUB_H
