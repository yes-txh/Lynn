// ReportServer.cpp : Defines the entry point for the console application.
//
#include <list>
#include "common/baselib/svrpublib/server_publib.h"
using namespace xfs::base;


struct ABCD
{
    int32_t i;
    int32_t j;
    ABCD():i(47),j(74){}
};

DECLARE_USING_LOG_LEVEL_NAMESPACE;

DECLARE_bool(binary_log);
DECLARE_int32(binary_log_size);


int main(int argc, char* argv[])
{
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, true);

    AutoBaseLib auto_baselib;
    FLAGS_binary_log = true;

    FLAGS_binary_log_size = 1024 * 200;
    ABCD abc;
    char c1 = 0;
    unsigned char c2 = 0;
    int16_t s1 = 0;
    uint16_t s2 = 0;
    int32_t i1 = 0;
    uint32_t i2 = 0;
    int64_t l1 = 0;
    uint64_t l2 = 0;
    for (int32_t i = 0; i != 5000; ++i) {
        CBaseProtocolPack packet;
        int32_t file_id1 = rand()%(1024*1024*1024);
        packet.SetServiceType(i);
        packet.SetReservedData((unsigned char*)&file_id1, sizeof(file_id1));
        packet.SetKey(i,i);
        abc.j++;
        packet.SetKey(i+1,&abc);
        packet.SetKey(i+2,c1++);
        packet.SetKey(i+3,c2++);
        packet.SetKey(i+4,s1++);
        packet.SetKey(i+5,s2++);
        packet.SetKey(i+6,i1++);
        packet.SetKey(i+7,i2++);
        packet.SetKey(i+8,l1++);
        packet.SetKey(i+9,l2++);
        unsigned char* ptr;
        uint32_t len;
        packet.GetPackage(&ptr,&len);

        CBaseProtocolUnpack unpacket;
        unpacket.AttachPackage(ptr,len);

        int32_t file_id2;
        unsigned char* tmp = NULL;
        unpacket.GetReservedData(&tmp,&len);
        memcpy((void*)&file_id2,tmp,len);
        if(file_id1 != file_id2)
            LOG(ERROR) << "put file id = " << file_id1
                       << ", get file id =" << file_id2;
    }
    return 0;
}
