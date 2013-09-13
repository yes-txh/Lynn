
#include "common/baselib/svrpublib/binary_log_unpack.h"


_START_XFS_BASE_NAMESPACE_

bool BinaryLogUnpack::Serialize() {
    bool b = true;
    if (!m_log_reader)
        m_log_reader = fopen(m_log_file.c_str(),"rb");

    if (!m_log_writer)
        m_log_writer = fopen((m_log_file+".unpack.txt").c_str(),"w");

    Init();
    BinaryLogHead head;
    int32_t ret;
    time_t t;
    struct tm* time_ptr;
    char time_str[128];
    while (true) {
        ret = fread(&head, sizeof(head), 1, m_log_reader);
        if(ret == 0)
            break; // 已经达到文件末尾
        if (!head.IsValid()) {
            b = false;
            break;
        }

        // 头部含有reserve字段
        if (head.reserve_len != 0)
            fseek(m_log_reader, head.reserve_len, SEEK_CUR);

        unsigned char* data = new unsigned char[head.len];
        ret = fread(data, head.len, 1, m_log_reader);

        AttachPackage(data,head.len);
        Unpack();
        char tmp[128];
        t = head.tv_sec;
        time_ptr = localtime(&t);
        strftime(time_str, sizeof(time_str), "time: %Y-%m-%d %H:%M:%S",time_ptr);
        sprintf(tmp, "\r\n%s.%d \r\n",time_str,head.tv_usec);
        fwrite(tmp, strlen(tmp)+1, 1, m_log_writer);

        sprintf(tmp,"service type: %d\r\n", GetServiceType());
        fwrite(tmp, strlen(tmp)+1, 1, m_log_writer);

        sprintf(tmp,"seq: %d\r\n", GetSeq());
        fwrite(tmp, strlen(tmp)+1, 1, m_log_writer);

        for (int32_t key = 0; key != 65536; ++key) {
            if (m_seg_vals[key].seq == m_seq) {
                switch (m_seg_vals[key].type) {
                case DTYPE_UCHAR:
                    char c1;
                    unsigned char c2;
                    GetVal(key,&c1);
                    GetVal(key,&c2);
                    sprintf(tmp,"key = %d---data = %d(char) or %d(unsigned char)\r\n",
                            key,c1,c2);
                    fwrite(tmp, strlen(tmp)+1, 1 ,m_log_writer);
                    break;

                case DTYPE_USHORT16:
                    int16_t s1;
                    uint16_t  s2;
                    GetVal(key,&s1);
                    GetVal(key,&s2);
                    sprintf(tmp,"key = %d---data = %d(int16_t) or %u(uint16_t)\r\n",key,s1,s2);
                    fwrite(tmp, strlen(tmp)+1, 1 ,m_log_writer);
                    break;

                case DTYPE_UINT32:
                    int32_t  i1;
                    uint32_t i2;
                    GetVal(key,&i1);
                    GetVal(key,&i2);

                    sprintf(tmp,"key = %d---data = %d(int32_t) or %u(uint32_t)\r\n",key,i1,i2);
                    fwrite(tmp, strlen(tmp)+1, 1 ,m_log_writer);
                    break;

                case DTYPE_UINT64:
                    int64_t  l1;
                    uint64_t l2;
                    GetVal(key,&l1);
                    GetVal(key,&l2);

                    sprintf(tmp,"key = %d---data = %"FMTd64"(int64_t) or %"FMTu64"(uint64_t)\r\n",key,l1,l2);
                    fwrite(tmp, strlen(tmp)+1, 1 ,m_log_writer);
                    break;

                case DTYPE_BYTES:
                    sprintf(tmp,"key = %d---data len= %d(bytes or struct)\r\n",key,m_seg_vals[key].len);
                    fwrite(tmp, strlen(tmp)+1, 1 ,m_log_writer);
                    break;

                default:
                    break;
                }
            }
        }
        delete data;
    }
    return b;
}

BinaryLogUnpack::~BinaryLogUnpack() {
    if (m_log_reader) {
        fclose(m_log_reader);
        m_log_reader = NULL;
    }
    if (m_log_writer) {
        fclose(m_log_writer);
        m_log_writer = NULL;
    }
}
_END_XFS_BASE_NAMESPACE_