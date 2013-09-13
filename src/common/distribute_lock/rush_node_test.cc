#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <zookeeper/zookeeper.h>
#include <common/base/string/string_number.hpp>

void string_comp(int rc, const char* value, const void *data)
{
    printf("%s\n", value);
}

int main(int argc, char *argv[])
{
    int cnt;
    if( argc < 2 ){
        cnt = 100;
    }
    else
    {
        cnt = atoi(optarg);
    }
	std::string node = "/node";
	std::string ip_port_group = "172.24.18.209:2181/zk/idc-xaec/soso/xcube/master_test";
    zhandle_t* handle = zookeeper_init(ip_port_group.c_str(), NULL, 2000, 0, NULL, 0 );
    sleep(1);

    for( int i = 0 ; i <  cnt; ++ i ){
        std::string name  = node + "/" + IntegerToString(i);
        zoo_acreate(handle, name.c_str(),NULL, 0, &ZOO_OPEN_ACL_UNSAFE, 0, string_comp, NULL );
    }
    while( true ){
        usleep(600);
    }

}
