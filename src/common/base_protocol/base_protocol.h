// BaseProtocol.h: interface for the BaseProtocol class.
// wookin@tencent.com
// 2007-05-11
//
// Modify by lonely 2010-04-08: Add DataSource Check
// 说明:通用协议封包,接解包
//        1.使用key-value对模型
//        2.支持的数据类型1bytes(int8_t/uint8_t),2bytes(SHORT16/USHORT16),
//                                  4bytes(INT32/UINT32),n bytes(BYTES)
//        3.字节序转换，如果设定kBaseProtocolOptNetOrder，自动做字节序转换，
//          应用层不用转换,BYTES类型字节序需要用户自行处理.
//        4.用户数据类型，可以自己定义结构按BYTES类型数据处理
//
//        package:
//        +----------+
//        | head     |
//        | reserved |
//        |----------|
//        | seg      |
//        | seg      |
//        | ...      |
//        +__________+
//
//        seg.
//        +----------------------------------+
//        | seg name(Key) 2B                 |
//        | data type     1B                 |
//        | *data len*    4B (type=bytes存在)|
//        | data...[len=data len]            |
//        +----------------------------------+
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// modified by ivanhuang at 20101104
// 1.修改代码风格
// 2.增加UnitTest
//////////////////////////////////////////////////////////////////////////

#ifndef COMMON_BASE_PROTOCOL_BASE_PROTOCOL_H_ // NOLINT
#define COMMON_BASE_PROTOCOL_BASE_PROTOCOL_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <BaseTsd.h>
#define int8_t      INT8
#define uint8_t     UINT8
#define int16_t     INT16
#define uint16_t    UINT16
#define int32_t     INT32
#define uint32_t    UINT32
#define int64_t     INT64
#define uint64_t    UINT64
#else
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdint.h>
#endif

#include "common/crypto/hash/crc.hpp"
#include "common/base_protocol/encryption.h"

inline uint64_t BaseProtocolH2N64(uint64_t x)
{
    return (((uint64_t)(htonl((uint32_t)((x) & 0xffffffff))) << 32) |
        htonl((uint32_t)(((x) >> 32) & 0xffffffff)));
}

inline uint64_t BaseProtocolN2H64(uint64_t x)
{
    return (((uint64_t)(ntohl((uint32_t)((x) & 0xffffffff))) << 32) |
        ntohl((uint32_t)(((x) >> 32) & 0xffffffff)));
}

inline size_t BaseProtocolStrLen(const char *string)
{
    return string ? strlen(string) : 0;
}

// 打包解包选项
// 如果需要CRC check,则Option高位一个Byte保存CRC8结果,计算CRC8不取Head部分
enum BaseProtocolOpt {
    kBaseProtocolOptNetOrder = 1,
    kBaseProtocolOptCrcCheck = 2,
};

// 打包解包版本信息
enum BaseProtocolVer {
    kProtocolVer1 = 1,
};

#pragma pack(push, 1)

// 协议头
struct BaseProtocolHeader {
    uint32_t            length;      // total length
    PseudoProtoHead     pseudo_head; // 伪协议头，数据来源校验
    uint8_t             version;
    uint8_t             time_to_live;
    uint32_t            option;

    uint32_t            sequence_num;
    uint16_t            service_type;
    uint16_t            first_seg_pos;

    BaseProtocolHeader()
    {
        version         = kProtocolVer1;
        time_to_live    = 10;
        option          = 0;
        sequence_num    = 0;
        first_seg_pos   = sizeof(BaseProtocolHeader);
        length          = sizeof(BaseProtocolHeader);
    }
    void ToNetOrder()
    {
        length          = htonl(length);
        option          = htonl(option);
        sequence_num    = htonl(sequence_num);
        service_type    = htons(service_type);
        first_seg_pos   = htons(first_seg_pos);
    }
    void ToHostOrder()
    {
        length          = ntohl(length);
        option          = ntohl(option);
        sequence_num    = ntohl(sequence_num);
        service_type    = ntohs(service_type);
        first_seg_pos   = ntohs(first_seg_pos);
    }
};

#pragma pack(pop)

// 该枚举类型用于标识用户提交的数据类型
// 并非表示该数据类型的长度^-^
enum {
    kDataTypeUChar   = 0,
    kDataTypeUShort  = 1,
    kDataTypeUInt    = 2,
    kDataTypeULong   = 3,
    kDataTypeUByte   = 4,
};

typedef uint8_t ProtocolDataType;

// 打包类 v1.0
class BaseProtocolPack
{
public:
    // 设置buffer,并标明已经打好包的长度
    BaseProtocolPack(void *buffer, uint32_t buffer_len,
                     uint32_t valid_packet_len);
    BaseProtocolPack(void *buffer, uint32_t buffer_len);

    BaseProtocolPack();
    ~BaseProtocolPack();

    bool Init();    // added by lonely: 如果使用内部buffer，预留1M内存
    void Uninit();

    void SetOption(BaseProtocolOpt opt, bool enable);
    void SetTTL(uint8_t time_to_live);
    void SetSeq(uint32_t seq);
    bool SetReservedData(uint8_t *data, uint32_t data_len);
    void SetForceSkipCRC(bool skip);

    void ResetContent();
    void SetServiceType(uint16_t type);

    bool SetKey(uint16_t key, int8_t val);
    bool SetKey(uint16_t key, uint8_t val);
    bool SetKey(uint16_t key, int16_t val);
    bool SetKey(uint16_t key, uint16_t val);
    bool SetKey(uint16_t key, int32_t val);
    bool SetKey(uint16_t key, uint32_t val);
    bool SetKey(uint16_t key, int64_t val);
    bool SetKey(uint16_t key, uint64_t val);

    bool SetKey(uint16_t key, float val);
    bool SetKey(uint16_t key, double val);

    bool SetKey(uint16_t key, uint8_t *data);
    bool SetKey(uint16_t key, uint8_t *data, uint32_t data_len);

    // 在最后一个Key后面追加数据(key必须和上次SetKey的key相同)
    bool AppendKeyData(uint16_t key, uint8_t *data, uint32_t data_len);

    // 获取打包后结果
    void GetPackage(uint8_t **pptr, uint32_t *len);

    // 获取只对单个大的数据块打包后的头部数据
    void GetPackedHeadOfBlock(uint16_t service_type, uint16_t key,
            uint32_t block_len, uint8_t **head, uint32_t *head_len);

private:
    int  CalcKeyValueLen(ProtocolDataType type, uint16_t key,
                         uint8_t *data, uint32_t data_len);
    void CommonConstruct();
    bool CheckAndAddBuffer(uint32_t new_data_len);
    bool SetKeyValue(ProtocolDataType type, uint16_t key,
                    uint8_t *data, uint32_t data_len);

private:
    uint8_t             m_use_inside_buffer;
    uint8_t             *m_pack_buffer;
    uint32_t            m_pack_buffer_len;
    BaseProtocolHeader    m_pack_head;
    uint32_t            m_auto_seq_num;

    // 最后一次设置数据的key和数据类型,用于AppendKeyData
    uint16_t            m_last_key;
    ProtocolDataType    m_last_key_type;
    uint32_t            m_last_key_pos;

    // 打包后的头部数据块,用于打包单个大数据块
    // BaseProtocolHeader+key(ushort)+data type(uchar)+data len(uint32)
    uint8_t single_block_packed_head_[sizeof(BaseProtocolHeader) + 7];
};

// 解包类 v1.0
class BaseProtocolUnpack
{
public:
    BaseProtocolUnpack();
    virtual ~BaseProtocolUnpack();

    bool                Init();
    void                Uninit();
    void                AttachPackage(void *buffer, uint32_t len);
    void                SetForceSkipCRC(bool skip);
    bool                Unpack();
    uint8_t             GetTTL();
    uint32_t            GetSeq();
    uint16_t            GetServiceType();
    bool                GetReservedData(uint8_t **pptr, uint32_t *data_len);
    bool                GetBodyData(uint8_t * &data_ptr, size_t &data_len);
    ProtocolDataType    GetDataType(uint16_t key, bool *in_use);
    bool                GetVal(uint16_t key, uint8_t *pval);
    bool                GetVal(uint16_t key, int8_t *pval);
    bool                GetVal(uint16_t key, int16_t *pval);
    bool                GetVal(uint16_t key, uint16_t *pval);
    bool                GetVal(uint16_t key, int32_t *pval);
    bool                GetVal(uint16_t key, uint32_t *pval);
    bool                GetVal(uint16_t key, float *pval);
    bool                GetVal(uint16_t key, double *pval);
    bool                GetVal(uint16_t key, int64_t *pval);
    bool                GetVal(uint16_t key, uint64_t *pval);
    bool                GetVal(uint16_t key, uint8_t **pptr, uint32_t *len);
    void                UntachPackage();

private:
    bool                GetKeyVal(uint16_t key, ProtocolDataType type,
                                void **pptr, uint32_t *len);

    struct SegVal
    {
        uint32_t    seq;            // 校验数据是否过时
        uint8_t     type;           // 数据类型
        uint32_t    len;            // 数据长度
        uint32_t    data_offset;    // 数据起始偏移
    };

    uint32_t    m_seq;
    SegVal      *m_seg_val;
    bool        m_net_order;
    uint8_t     *m_ref_data;
    uint32_t    m_data_len;
    bool        m_force_non_crc;
};

//////////////////////////////////////////////////////////////////////////
inline void BaseProtocolPack::SetOption(BaseProtocolOpt opt, bool enable)
{
    if (enable)
        m_pack_head.option |= opt;
    else
        m_pack_head.option &= ~opt;
}

inline void BaseProtocolPack::SetTTL(uint8_t ttl)
{
    m_pack_head.time_to_live = ttl;
}

inline void BaseProtocolPack::SetSeq(uint32_t seq)
{
    m_pack_head.sequence_num = seq;
}

inline void BaseProtocolPack::SetForceSkipCRC(bool skip)
{
    SetOption(kBaseProtocolOptCrcCheck, !skip);
}

inline void BaseProtocolPack::SetServiceType(uint16_t service_type)
{
    m_pack_head.service_type = service_type;
}

//////////////////////////////////////////////////////////////////////////
inline void BaseProtocolUnpack::SetForceSkipCRC(bool skip)
{
    m_force_non_crc = skip;
}

inline void BaseProtocolUnpack::AttachPackage(void *packet,
                                               uint32_t packet_len)
{
    m_ref_data = reinterpret_cast<uint8_t *>(packet);
    m_data_len = packet_len;
}

inline bool BaseProtocolUnpack::GetBodyData(uint8_t * &data_ptr, size_t &data_len)
{
    if (m_ref_data && m_data_len >= sizeof(BaseProtocolHeader))
    {
        BaseProtocolHeader head;
        memcpy(&head, m_ref_data, sizeof(head));
        head.ToHostOrder();

        data_ptr = m_ref_data + head.first_seg_pos;
        data_len = head.length - head.first_seg_pos;

        return true;
    }

    return false;
}

inline uint16_t BaseProtocolUnpack::GetServiceType()
{
    uint16_t service_type = 0;

    if (m_ref_data && m_data_len && m_data_len >= sizeof(BaseProtocolHeader))
    {
        BaseProtocolHeader *phead =
            reinterpret_cast<BaseProtocolHeader *>(m_ref_data);
        service_type = ntohs(phead->service_type);
    }

    return service_type;
}

inline uint8_t BaseProtocolUnpack::GetTTL()
{
    uint8_t time_to_live = 0;

    if (m_ref_data && m_data_len && m_data_len >= sizeof(BaseProtocolHeader))
    {
        BaseProtocolHeader *phead =
            reinterpret_cast<BaseProtocolHeader *>(m_ref_data);
        time_to_live = phead->time_to_live;
    }

    return time_to_live;
}

inline uint32_t BaseProtocolUnpack::GetSeq()
{
    uint32_t sequence = 0;
    if (m_ref_data && m_data_len && m_data_len >= sizeof(BaseProtocolHeader))
    {
        BaseProtocolHeader *phead =
            reinterpret_cast<BaseProtocolHeader *>(m_ref_data);
        sequence = ntohl(phead->sequence_num);
    }
    return sequence;
}

inline ProtocolDataType BaseProtocolUnpack::GetDataType(uint16_t key,
                                                         bool *in_use)
{
    ProtocolDataType type;

    if (m_seg_val)
    {
        if (m_seg_val[key].seq == m_seq && in_use)
            *in_use = true;

        type = (ProtocolDataType)m_seg_val[key].type;
    }

    return type;
}

inline void BaseProtocolUnpack::UntachPackage()
{
    m_ref_data = 0;
    m_data_len = 0;
}

//////////////////////////////////////////////////////////////////////////
// 1:强行修改已经打好包包头中序列号
// 2:也可以通过打包前设置BaseProtocolHeader.sequence_num来完成
inline void ModifySeqNum(uint8_t *data, uint32_t data_len, uint32_t seq_num)
{
    if (data && data_len >= sizeof(BaseProtocolHeader))
    {
        BaseProtocolHeader *proto_head =
            reinterpret_cast<BaseProtocolHeader *>(data);

        if (proto_head->version == kProtocolVer1)
            proto_head->sequence_num = htonl(seq_num);
    }
}

// 1:从已经打包的数据块中获取序列号
// 2:也可以从BaseProtocolHeader.sequence_num获取
inline bool GetSeqNumInPack(uint8_t *data, uint32_t data_len, uint32_t *seq_num)
{
    bool b = false;

    if (seq_num && data && data_len >= sizeof(BaseProtocolHeader))
    {
        BaseProtocolHeader *proto_head =
            reinterpret_cast<BaseProtocolHeader *>(data);

        if (proto_head->version == kProtocolVer1)
        {
            *seq_num = ntohl(proto_head->sequence_num);
            b = true;
        }
    }

    return b;
}

// add by lonelyjia
// 1.强行修改已经打包好的包头中的servicetype
// 2.也可以通过打包前设置BaseProtocolHeader.service_type来完成
inline void ModifyServicType(uint8_t *data, uint32_t data_len, uint16_t service_type)
{
    if (data && data_len >= sizeof(BaseProtocolHeader))
    {
        BaseProtocolHeader *proto_head =
            reinterpret_cast<BaseProtocolHeader *>(data);

        if (proto_head->version == kProtocolVer1)
            proto_head->service_type = htons(service_type);
    }
}

// 1.从已经打包的数据中获取serviceType
// 2.也可以从BaseProtocolHeader.service_type 来完成
inline bool GetServicTypeInPack(uint8_t *data, uint32_t data_len, uint16_t *svc_type)
{
    bool b = false;
    *svc_type = 0;

    if (svc_type && data && data_len >= sizeof(BaseProtocolHeader))
    {
        BaseProtocolHeader *proto_head =
            reinterpret_cast<BaseProtocolHeader *>(data);

        if (proto_head->version == kProtocolVer1)
        {
            *svc_type = ntohs(proto_head->service_type);
            b = true;
        }
    }

    return b;
}

#endif // COMMON_BASE_PROTOCOL_BASE_PROTOCOL_H_ // NOLINT
