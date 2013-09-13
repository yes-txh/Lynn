#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#include <algorithm>

#include "common/rpc/rpc.hpp"
#include "common/config/cflags.hpp"
#include "common/system/concurrency/atomic/atomic.hpp"

#include "common/rpc/examples/multilisten_proxy.h"
#include "common/rpc/examples/multilisten_stub.h"

/// 实现对象
class MultiListen: public MultiListenStub<MultiListen>
{
public:
	MultiListen(Rpc::ObjectId_t id) : MultiListenStub<MultiListen>(id)
	{
	}

	/// 同步方式实现 Echo0 方法
	void Foo()
	{
	}
};

volatile bool g_quit;

void sighandler(int)
{
	g_quit = true;
}

std::string g_endpoint1 = "127.0.0.1:10001";
std::string g_endpoint2 = "127.0.0.1:10002";

void TestClient()
{
	MultiListenProxy proxy1, proxy2;
	Rpc::GetRemoteObject(g_endpoint1, 1, proxy1);
	Rpc::GetRemoteObject(g_endpoint2, 1, proxy2);
	proxy1.Foo();
	printf("proxy1.Foo() OK\n");
	proxy2.Foo();
	printf("proxy2.Foo() OK\n");
}

int main(int argc, char** argv)
{
	Rpc::Initialize();
	MultiListen object(1);

	Rpc::Status_t status = Rpc::Listen(g_endpoint1);
	if (status != Rpc::Status_Success)
	{
		fprintf(stderr, "Listen error: %s, %s\n", Rpc::StatusString(status), strerror(errno));
		return EXIT_FAILURE;
	}

	status = Rpc::Listen(g_endpoint2);
	if (status != Rpc::Status_Success)
	{
		fprintf(stderr, "Listen error: %s, %s\n", Rpc::StatusString(status), strerror(errno));
		return EXIT_FAILURE;
	}

	TestClient();

	signal(SIGINT, sighandler);

	while (!g_quit)
	{
		ThisThread::Sleep(1000);
	}

	return 0;
}
