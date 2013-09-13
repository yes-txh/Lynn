// TestSvrpublib.cpp : Defines the entry point for the console application.
//

#include "common/baselib/svrpublib/twse_type_def.h"
#include "common/baselib/svrpublib/server_publib.h"

// using namespace google;
DECLARE_USING_LOG_LEVEL_NAMESPACE;

using namespace xfs::base;

int32_t get_test_val() {
    return 5;
}

void test(const char* program) {
    AutoBaseLib auto_baselib;
    LOG(INFO) << "hello.";
}

int main(int argc, char** argv) {
	InitGoogleDefaultLogParam(argv[0]);
    AutoBaseLib auto_baselib;
    
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, false);
    

    // 测试多次调用AutoBaseLib
    // test(argv[0]);

    // FLAGS_log_dir = "c:\\";
    // FLAGS_v = 0;
    // FLAGS_stderrthreshold = INFO;
    // FLAGS_logtostderr = false;
    // FLAGS_alsologtostderr = true;

    LOG(INFO) << ".";


    //
    // Set CHECK_xx ...
    //

    // CHECK_EQ(get_test_val(),6);


    uint32_t u = 0;
    // 测试XGUID
    XGUID uid;

    // for(u=0;u<50;u++)
    for (u = 0; u < 4; u++) {
        GetGUID(&uid);
        char sz[256];
        safe_snprintf(sz, sizeof(sz), "%x,%x,%x,%x-%x-%x-%x-%x-%x-%x-%x",
                     uid.data1, uid.data2, uid.data3,
                     uid.data4[0],uid.data4[1],uid.data4[2],uid.data4[3],
                     uid.data4[4],uid.data4[5],uid.data4[6],uid.data4[7]);
                     
        LOG(INFO) << "uuid:" << sz;
    }

    for (u = 0; u < 10; u++) {
		char temp[125];
		safe_snprintf(temp, sizeof(temp), "hi this \r\n \t brad \n test \r hi \r\n %u hi",u);
		temp[10] = -7;
		temp[17] = -52;
		Xprintf_MemDebug(temp, (int32_t)strlen(temp));
	}

    LOG(INFO) << " ";

    LOG(INFO) << "now online num:" << 4;

    XSleep(1);
    LOG(INFO) << "Hello World!";
    LOG(WARNING) << "test log level:warning.";
    LOG(ERROR) << "test log level:error.";
    LOG(INFO) << "test log level:info.";

    // 提前检测内存泄露
    // 如果不主动调用,进程退出会自动调用
    mempool_Clean();

    // char* tmp_ptr=NULL;
    // CHECK(tmp_ptr);
    // CHECK_NE(tmp_ptr,NULL_PTR);

    // RAW_LOG(FATAL,"FATAL...");

    return 0;
}

