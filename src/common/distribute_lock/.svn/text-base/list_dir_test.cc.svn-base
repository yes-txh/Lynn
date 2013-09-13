#include <iostream>
#include <common/distribute_lock/distribute_lock.h>


int main(int argc, char* argv[])
{
    using namespace distribute_lock;

    if( argc < 3 ){
        std::cout<<"input ip:port, node-path"<<std::endl;
        return 0;
    }
    std::vector<std::string> files;
    std::string dir = std::string(argv[2]);
    EventListener listener;

    DistributeLock dist_lock;
    dist_lock.Init(argv[1], &listener, 1000);

    dist_lock.ListNode(dir, files);
    for( size_t i = 0 ; i < files.size() ; ++ i ){
        std::cout<<"files: "<<files[i]<<std::endl;
    }
    return 0;
}
