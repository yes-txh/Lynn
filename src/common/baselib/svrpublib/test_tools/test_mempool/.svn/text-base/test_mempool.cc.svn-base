// ReportServer.cpp : Defines the entry point for the console application.
//
#include <list>
#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/binary_log_unpack.h"
#include "common/baselib/svrpublib/test_tools/test_mempool/parse_inl.h"
using namespace xfs::base;

int32_t flag = 0;
CXThreadMutex mutex;
int32_t random_num[200];

struct ABCD
{
    int32_t i;
    int32_t j;
    ABCD():i(47), j(74){}
};

class MempoolTestThread:public CXThreadBase
{
public:
    void Routine() {
        uint64_t ul;
        for(int i = 0; i < 20000; i++)
        {
            int len=0;
            len=random_num[rand()%(200)];

            unsigned char*  mptr=mempool_NEW_BYTES(len);

            ul=0;
            memcpy(&ul,&mptr,sizeof(void*));
            if((ul%8) !=0 ) {
                LOG(ERROR) << "***ERROR,Level 3***,invalid addrs.";
            }
            mem_list.push_back(mptr);
            if(mem_list.size() < 100)
                continue;
            int pos = rand()%100;
            unsigned char* to_delete;
            std::list<unsigned char*>::iterator iter = mem_list.begin();
            for (int32_t j = 0; j < pos; ++j) {
                ++iter;
            }
            to_delete = *iter;
            mem_list.erase(iter);
            mempool_DELETE(to_delete);        
        }
        while (mem_list.size() > 1) {
            unsigned char* tmp = mem_list.front();
            mem_list.pop_front();
            mempool_DELETE(tmp);
        }
        CXThreadAutoLock lock(&mutex);
        StopRoutine();
        ++flag;
    }
private:
    std::list<unsigned char*> mem_list;
};
DECLARE_bool(binary_log);
DECLARE_int32(binary_log_size);

DECLARE_USING_LOG_LEVEL_NAMESPACE;
int main(int argc, char* argv[])
{
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, false);
    AutoBaseLib auto_baselib;

    time_t now = time(0);
    for (int32_t i = 0; i != 200; i++) {
        srand((uint32_t)now + i);
        random_num[i] = rand()% 1024 * 32;
    }
    MempoolTestThread test_thread[10];
    time_t begin = time(0);
    for (int i = 0; i != 5; ++i) {
        test_thread[i].StartThread();
    }
    while (1) {
        mutex.Lock();
        if (flag == 5) {
            mutex.UnLock();
            break;
        }
        mutex.UnLock();
        XSleep(10);
    }
    time_t end = time(0);
    LOG(INFO) << "last time is " << end - begin;

    /*FLAGS_binary_log = true;
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
        packet.SetKey(i, i);
        abc.j++;
        packet.SetKey(i+1, &abc);
        packet.SetKey(i+2, c1++);
        packet.SetKey(i+3, c2++);
        packet.SetKey(i+4, s1++);
        packet.SetKey(i+5, s2++);
        packet.SetKey(i+6, i1++);
        packet.SetKey(i+7, i2++);
        packet.SetKey(i+8, l1++);
        packet.SetKey(i+9, l2++);
        unsigned char* ptr;
        uint32_t len;
        packet.GetPackage(&ptr, &len);

        CBaseProtocolUnpack unpacket;
        unpacket.AttachPackage(ptr, len);

        int32_t file_id2;
        unsigned char* tmp;
        unpacket.GetReservedData(&tmp, &len);
        memcpy(static_cast<void*>(&file_id2), tmp, len);
        if (file_id1 != file_id2)
            LOG(ERROR) << "put file id =" << file_id1
            <<", get file id =" << file_id2;
    }
    FLAGS_binary_log = false;
    BinaryLogUnpack unpack("E:\\vcoutput\\debug\\Test_MemPool_D.exe.BRADZHANG-PC0.bradzhang.binary_log00.dat");
    unpack.Serialize();*/
    return 0;
}

#if 0
#define ENUM3(Name,V1,V2,V3) enum Name {V1,V2,V3}; \
                             const char* Name##_Strings [3] = {#V1,#V2,#V3};\
                             int Name##_Index[3] = {V1,V2,V3};
#define GET_ENUM_STRING(Name, val) Name##_Strings[val]



ENUM3(
      Enum_Test, 
      val1,//hi
      val2,
      val3
      );
#define MY_LIST \
    X(foo),    \
    X(bar),    \
    E_V(zz,300), \
    X(baz)


#define X(x) x
#define E_V(x,v) x=v
enum eMyEnum{	MY_LIST};

#undef E_V
#define E_V(x,v) x

int val_arry[] = {MY_LIST};
#undef X


#undef E_V
#define E_V(x,v) #x##"="#v
#define X(x) #x
const char *szMyEnum[] ={	MY_LIST};
#undef X 

int main()
{
    Enum_Test enum_test;
    enum_test = val1;

    printf("get enum val: %d, desc:%s\r\n", enum_test, GET_ENUM_STRING(Enum_Test, val1));

    ParseProtocolInl parse_inl;
    parse_inl.Parse("E:\\SVN-DIR\\setech_infrastructure_rep\\Infra_proj\\trunk\\src\\xfs\\share\\xfs_internal_include\\node_server_2_master.protocol.inl");
    parse_inl.Debug();

    //eMyEnum emy;
    const char* sz= szMyEnum[1];
    int i=0;
    int count = sizeof(val_arry)/sizeof(int);
    for(i=0; i<count;i++)
        printf("val_arry[%d]=%d %s\r\n",i, val_arry[i],szMyEnum[i]);

    return 0;
};
#endif