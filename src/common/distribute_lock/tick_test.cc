#include "gflags/gflags.h"
#include "common/system/time/time_utils.hpp"
#include "common/distribute_lock/distribute_lock.h"

DEFINE_string(addr, "127.0.0.1", "location");
int main(int argc, char *argv[])
{
    if ( argc < 2 )
    {
        exit(0);
    }

    // 解析配置
    if (!google::ParseCommandLineFlags(&argc, &argv, false))
    {
        fprintf(stderr, "CFlags::ParseCommandLine error\n");
        return EXIT_FAILURE;
    }//end if
    fprintf(stderr, "%s\n", FLAGS_addr.data());
    FLAGS_log_dir = "./";
    FLAGS_per_log_size = 1000;
    FLAGS_max_log_size = 20000;
    FLAGS_stderrthreshold = 3;
    FLAGS_logbuflevel = -1;
    std::string timestamp = TimeUtils::GetCurMilliTime(); ///< 当前的时间
    std::string ip_tm = FLAGS_addr + "_" + timestamp;
    std::string zk_ip = "xaec.zk.oa.com:2181/zk/xaec/xcube/dist_lock/ts";
    distribute_lock::DistributeLock dist_lock;
    int ret = dist_lock.Init(zk_ip, NULL, 2000);
    std::string node = "/" + ip_tm;
    ret = dist_lock.CreateNode(node);
    ret = dist_lock.Lock(node);
    LOG(INFO) << "lock ok";
    while (true)
    {
        sleep(1);
    }
    return 0;
}
