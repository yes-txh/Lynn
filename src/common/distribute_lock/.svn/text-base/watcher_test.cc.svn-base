#include <iostream>
#include <vector>
#include <string>
#include <common/distribute_lock/distribute_lock.h>
#include <common/distribute_lock/event_listener.h>
#include <common/distribute_lock/event_mask.h>


void usage()
{
     std::cout << std::endl;
     std::cout << "USAGE: List [options]" << std::endl << std::endl;
     std::cout << "options:" << std::endl << std::endl;
     std::cout << "-h \t for help." << std::endl << std::endl;
     std::cout << "-i \t ip address of zookeeper (default 172.24.18.209:2181)" << std::endl << std::endl;
     std::cout << "-m \t method to test" << std::endl << std::endl;
     std::cout << "-n \t node name to deal with (default /lock)" << std::endl << std::endl;
}

int main(int argc, char *argv[])
{
    using namespace distribute_lock;
    google::InitGoogleLogging(argv[0]);
    FLAGS_log_dir = "./";
    FLAGS_per_log_size = 1000;
    FLAGS_max_log_size = 20000;
    FLAGS_stderrthreshold = 3;
    std::string zookeeper_ip = "xaec.zk.oa.com:2181/zk/xaec/xcube/master_test";
    std::string method = "watch-on-child";
    std::string node   = "/node";
    DistributeLock dist_lock;


    int ch = 0;
    while( (ch = getopt(argc, argv, "hi:m:n:")) != -1 ){
        switch ( ch )
        {
        case 'h' :
            usage();
            exit(0);
            break;
        case 'i' :
            zookeeper_ip = std::string(optarg);
            break;
        case 'm':
            method = std::string(optarg);
        case 'n':
            node = std::string(optarg);
        default :
            break;
        }
    }
    EventListener listener;
    int ret = dist_lock.Init(zookeeper_ip.c_str(), &listener, 20000);
    assert(ret == 0);

    if(method == std::string("watch-on-lock"))
    {
        /// 设置了监听lock事件
        if( dist_lock.Exists(node) ){
            dist_lock.SetWatcher(node, EVENT_MASK_LOCK_ACQUIRED);
        }
        else dist_lock.CreateNode(node, EVENT_MASK_LOCK_ACQUIRED );
    }
    if( method == std::string("watch-on-child") )
    {
        ret = dist_lock.MkDir(node, EVENT_MASK_CHILD_CHANGED);
    }

    while( true ){
        usleep(1000);
    }

    return 0;
}

