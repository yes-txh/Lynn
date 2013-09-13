//////////////////////////////////////////////////////////////////////////
// key_value_parser_test.cc
// @brief:     Test class CParseProc
// @author:  fatliu@tencent
// @time:    2010-09-30
// @version: 1.0
//////////////////////////////////////////////////////////////////////////
#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/general_head.h"
#include "common/baselib/svrpublib/general_type_def.h"
#include "common/baselib/svrpublib/thread_mutex.h"
#include "common/baselib/svrpublib/general_util.h"
#include "common/baselib/svrpublib/parse_proc.h"

#include "common/baselib/svrpublib/log.h"
#include "thirdparty/gtest/gtest.h"

#define CPU_NUM 128


DECLARE_USING_LOG_LEVEL_NAMESPACE;
using namespace xfs::base;


#ifdef WIN32
int32_t TestParseProc(int32_t argc, char** argv)
#else
int32_t main(int32_t argc, char** argv)
#endif
{
#ifndef WIN32
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, true);

    AutoBaseLib auto_baselib;
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
#else
    return 0;
#endif
}

#ifndef WIN32

TEST(CParseProc, CalcCpu) {
    CParseProc parser;
    std::vector<CpuStatus> cpu1, cpu2;

    ASSERT_TRUE(parser.ParseCpu(&cpu1));
    ASSERT_TRUE(parser.ParseCpu(&cpu2));
    ASSERT_EQ(cpu1.size(), cpu2.size());
    ASSERT_LT(0u, cpu1.size());

    CpuRate cpu_rate;

    for( size_t i = 0; i < cpu1.size(); ++i) {
        ASSERT_TRUE(parser.CalcCpu(&cpu_rate, &cpu1[i], &cpu2[i]));
    }
}

TEST(CParseProc, ParseSystemMemoryStatus) {
    CParseProc parser;
    SystemMemoryStatus sys_mem_status;
    ASSERT_TRUE(parser.ParseSystemMemoryStatus(&sys_mem_status));
}

TEST(CParseProc, ParseProcessMemoryStatus) {
    CParseProc parser;
    std::vector<ProcessMemStatus> process_mem_status;
    ASSERT_TRUE(parser.ParseProcessMemoryStatus(&process_mem_status, false));
    CHECK_EQ(static_cast<size_t>(1), process_mem_status.size());
    CHECK_EQ(static_cast<uint32_t>(getpid()), process_mem_status[0].pid);
    ASSERT_TRUE(parser.ParseProcessMemoryStatus(&process_mem_status));
    CHECK_EQ(static_cast<uint32_t>(getpid()), process_mem_status[0].pid);
}

TEST(CParseProc, ParseDiskStatus) {
    CParseProc parser;
    const char* disk_status = parser.ParseDiskStatus();
    ASSERT_TRUE(disk_status != NULL);
}

TEST(TestParseSocketPair, PROTOCOL_TCP) {
    CParseProc parser;
    std::vector<NetSocketPairInfo> socket_pair;
    ASSERT_TRUE(parser.ParseSocketPair(ENUM_PROTOCOL_TCP, &socket_pair));
}

TEST(TestParseSocketPair, PROTOCOL_UDP) {
    CParseProc parser;
    std::vector<NetSocketPairInfo> socket_pair;
    ASSERT_TRUE(parser.ParseSocketPair(ENUM_PROTOCOL_UDP, &socket_pair));
}

TEST(TestParseSocketPair, PROTOCOL_TCP_LISTEN) {
    // tcp_listen在有些机器没有
    /*CParseProc parser;
    std::vector<SocketPair> socket_pair;
    ASSERT_TRUE(parser.ParseSocketPair(ENUM_PROTOCOL_TCP_LISTEN, &socket_pair));*/
}

TEST(CParseProc, ParseProcessInfo) {
    CParseProc parser;
    std::vector<ProcessInfo> process_info;
    ASSERT_TRUE(parser.ParseProcessInfo(&process_info));
}

TEST(CParseProc, GetNetDesc) {
    CParseProc parser;
    const char* states[] = {
        "ESTBLSH",
        "SYNSENT",
        "SYNRECV",
        "FWAIT1",
        "FWAIT2",
        "TMEWAIT",
        "CLOSED",
        "CLSWAIT",
        "LASTACK",
        "LISTEN",
        "CLOSING",
        "UNKNOWN",
        ""
    };
    char* str_states = const_cast<char*>("");

    for (int32_t i = 0; i < 13; i++) {
        str_states = const_cast<char*>(parser.GetNetDesc(i));
        EXPECT_EQ(0, strcmp(str_states, states[i]));
    }

    str_states = const_cast<char*>(parser.GetNetDesc(14));
    EXPECT_EQ(0, strcmp(str_states, ""));
}

TEST(CParseProc, GetProtocol) {
    CParseProc parser;
    char* str_protocol = const_cast<char*>(parser.GetProtocol(0));
    EXPECT_EQ(0, strcmp("tcp", str_protocol));
    str_protocol = const_cast<char*>(parser.GetProtocol(1));
    EXPECT_EQ(0, strcmp("udp", str_protocol));
    str_protocol = const_cast<char*>(parser.GetProtocol(2));
    EXPECT_EQ(0, strcmp("", str_protocol));
}

TEST(CParseProc, IOStat) {
    std::vector<IOStat> io_stat;
    CParseProc parser;
    bool b_ret = parser.ParseDiskStat(&io_stat);
    ASSERT_TRUE(b_ret);

    EXPECT_NE(0u, io_stat.size());

    for(size_t i = 0; i < io_stat.size(); ++i) {
        ASSERT_FALSE(io_stat[i].dev.empty());
        LOG(INFO) << "dev=" << io_stat[i].dev
                  << " rio=" << io_stat[i].rio
                  << " wio=" << io_stat[i].wio
                  << " rkb=" << io_stat[i].rkb
                  << " wkb=" << io_stat[i].wkb
                  << " await=" << io_stat[i].await
                  << " svctm=" << io_stat[i].svctm
                  << " util=" << io_stat[i].util;
    }


}

TEST(CParseProc, NetStat) {
    std::vector<NetStat> net_stat;
    CParseProc parser;
    bool b_ret = parser.ParseNetStat(&net_stat);
    ASSERT_TRUE(b_ret);

    EXPECT_NE(0u, net_stat.size());

    for(size_t i = 0; i < net_stat.size(); ++i) {
        ASSERT_FALSE(net_stat[i].dev.empty());
        LOG(INFO) << "dev=" << net_stat[i].dev
                  << " rbytes=" << net_stat[i].rbytes
                  << " wbytes=" << net_stat[i].wbytes
                  << " rpackets=" << net_stat[i].rpackets
                  << " wpackets=" << net_stat[i].wpackets;
    }
}








#endif
