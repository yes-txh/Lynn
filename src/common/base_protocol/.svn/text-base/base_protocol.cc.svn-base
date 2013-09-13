#include "common/base_protocol/base_protocol.h"

// ============================================================================
// class BaseProtocolPack
// ============================================================================
void BaseProtocolPack::CommonConstruct()
{
    // 初始化变量
    m_last_key           = 0;
    m_last_key_type      = 0;
    m_last_key_pos       = 0;

    // 默认使用网络字节序列
    SetOption(kBaseProtocolOptNetOrder, true);

#ifndef NDEBUG
    // _DEBUG状态启用CRC检查
    SetOption(kBaseProtocolOptCrcCheck, true);
#endif

    // 自动维护序列号,用户调用ResetContent()以后自动设置
    // 如果用户调用SetSeq(),则使用用户设定的SeqNum
    srand(time(0));
    m_auto_seq_num = 200;
}

BaseProtocolPack::BaseProtocolPack()
{
    m_use_inside_buffer  = 1;
    m_pack_buffer        = 0;
    m_pack_buffer_len    = 0;

    CommonConstruct();
}

BaseProtocolPack::BaseProtocolPack(void *data_buff, uint32_t buffer_len)
{
    m_use_inside_buffer  = 0;
    m_pack_buffer        = reinterpret_cast<uint8_t *>(data_buff);
    m_pack_buffer_len    = buffer_len;

    CommonConstruct();
}

BaseProtocolPack::BaseProtocolPack(void *data_buff, uint32_t buffer_len,
                                     uint32_t valid_pack_len)
{
    m_use_inside_buffer  = 0;
    m_pack_buffer        = reinterpret_cast<uint8_t *>(data_buff);
    m_pack_buffer_len    = buffer_len;

    CommonConstruct();

    // 设置已经打包好的一段内存
    if (valid_pack_len >= sizeof(m_pack_head))
        memcpy(&m_pack_head, data_buff, sizeof(m_pack_head));
}

BaseProtocolPack::~BaseProtocolPack()
{
    Uninit();
}

bool BaseProtocolPack::Init()
{
    if (m_use_inside_buffer)
    {
        if (NULL == m_pack_buffer)
        {
            m_pack_buffer     = new uint8_t[1024 * 1024];
            m_pack_buffer_len = 1024 * 1024;
        }
    }

    return true;
}

void BaseProtocolPack::Uninit()
{
    if (m_use_inside_buffer)
    {
        delete [] m_pack_buffer;
        m_pack_buffer     = 0;
        m_pack_buffer_len = 0;
    }
}

bool BaseProtocolPack::CheckAndAddBuffer(uint32_t new_data_len)
{
    bool b = false;

    if (m_use_inside_buffer)
    {
        if (m_pack_head.length + new_data_len > m_pack_buffer_len)
        {
            uint32_t  new_len  = m_pack_head.length + new_data_len + 256;
            uint8_t *data_buff = new uint8_t[new_len];

            if (m_pack_buffer)
            {
                memcpy(data_buff, m_pack_buffer, m_pack_head.length);
                delete [] m_pack_buffer;
            }

            m_pack_buffer     = data_buff;
            m_pack_buffer_len = new_len;

            b = true;
        }
        else
            b = true;
    }
    else
    {
        if (m_pack_head.length + new_data_len <= m_pack_buffer_len)
            b = true;
    }

    return b;
}

bool BaseProtocolPack::SetReservedData(uint8_t *data, uint32_t data_len)
{
    bool b = false;

    if (data && data_len && CheckAndAddBuffer(data_len))
    {
        m_pack_head.length = sizeof(m_pack_head);

        memcpy(m_pack_buffer + m_pack_head.length, data, data_len);

        m_pack_head.length += data_len;
        m_pack_head.first_seg_pos = static_cast<uint16_t>(m_pack_head.length);

        b = true;
    }

    return b;
}

void BaseProtocolPack::ResetContent()
{
    m_pack_head.length        = sizeof(m_pack_head);
    m_pack_head.first_seg_pos = static_cast<uint16_t>(m_pack_head.length);

    // auto seq num
    m_auto_seq_num++;
    m_pack_head.sequence_num = m_auto_seq_num;
}

int BaseProtocolPack::CalcKeyValueLen(ProtocolDataType type, uint16_t key,
                                       uint8_t *data, uint32_t data_len)
{
    int net_block_len = 0;

    switch (type)
    {
    case kDataTypeUChar:
    case kDataTypeUShort:
    case kDataTypeUInt:
    case kDataTypeULong:
        net_block_len = sizeof(key) + 1/*data type*/ + data_len;
        break;
    case kDataTypeUByte:
        data_len = (data == NULL) ? 0 : data_len;
        net_block_len = sizeof(key) + 1/*data type*/ +
            sizeof(int)/*data len*/ + data_len; // NOLINT
        break;
    default:
        break;
    }

    return net_block_len;
}

bool BaseProtocolPack::SetKeyValue(ProtocolDataType type, uint16_t key,
                                  uint8_t *data, uint32_t data_len)
{
    bool b = false;

    // 计算存储的数据长度
    int key_value_len = CalcKeyValueLen(type, key, data, data_len);
    if (key_value_len <= 0)
    {
        return false;
    }

    uint32_t net_block_len = static_cast<uint32_t>(key_value_len);

    // 检查内存并打包
    // 长度为0的字符流也打包
    if (CheckAndAddBuffer(net_block_len))
    {
        bool use_net_order=((m_pack_head.option & kBaseProtocolOptNetOrder) ==
                            kBaseProtocolOptNetOrder) ? true : false;

        // save last key info
        m_last_key      = key;
        m_last_key_type = type;
        m_last_key_pos  = m_pack_head.length;

        // seg name
        uint16_t seg_name = key;
        if (use_net_order)
            seg_name = htons(seg_name);
        memcpy(m_pack_buffer+m_pack_head.length, &seg_name, sizeof(seg_name));
        m_pack_head.length += sizeof(seg_name);

        // data type
        memcpy(m_pack_buffer + m_pack_head.length, &type, sizeof(type));
        m_pack_head.length += sizeof(type);

        // data length
        if (type == kDataTypeUByte)
        {
            uint32_t ul_data_len = data_len;
            if (use_net_order)
                ul_data_len = htonl(ul_data_len);
            memcpy(m_pack_buffer + m_pack_head.length, &ul_data_len,
                    sizeof(ul_data_len));
            m_pack_head.length += sizeof(ul_data_len);
        }

        // real data
        uint16_t us_real_data  = 0;
        uint32_t ul_real_data  = 0;
        uint64_t ull_real_data = 0;
        switch (type)
        {
        case kDataTypeUShort:
            memcpy(&us_real_data, data, sizeof(uint16_t));
            if (use_net_order)
                us_real_data = htons(us_real_data);
            memcpy(m_pack_buffer + m_pack_head.length, &us_real_data,
                    sizeof(us_real_data));
            m_pack_head.length += sizeof(us_real_data);
            break;
        case kDataTypeUInt:
            memcpy(&ul_real_data, data, sizeof(uint32_t));
            if (use_net_order)
                ul_real_data = htonl(ul_real_data);
            memcpy(m_pack_buffer + m_pack_head.length, &ul_real_data,
                    sizeof(ul_real_data));
            m_pack_head.length += sizeof(ul_real_data);
            break;
        case kDataTypeULong:
            memcpy(&ull_real_data, data, sizeof(uint64_t));
            if (use_net_order)
                ull_real_data = BaseProtocolH2N64(ull_real_data);
            memcpy(m_pack_buffer + m_pack_head.length, &ull_real_data,
                    sizeof(uint64_t));
            m_pack_head.length += sizeof(uint64_t);
            break;
        default:
            memcpy(m_pack_buffer + m_pack_head.length, data, data_len);
            m_pack_head.length += data_len;
        }

        b = true;
    }

    return b;
}

bool BaseProtocolPack::AppendKeyData(uint16_t key, uint8_t *data,
                                      uint32_t data_len)
{
    bool b = false;
    uint32_t new_buff_len = data_len;

    if (data && data_len && m_last_key == key &&
        m_last_key_type == kDataTypeUByte &&
        CheckAndAddBuffer(new_buff_len) &&
        m_pack_head.length < m_pack_buffer_len)
    {
        bool use_net_order = ((m_pack_head.option & kBaseProtocolOptNetOrder)
                                == kBaseProtocolOptNetOrder) ? true : false;

        // get data length
        uint32_t ul_data_len = 0;
        memcpy(&ul_data_len, m_pack_buffer + m_last_key_pos + 2 + 1, 4);

        if (use_net_order)
            ul_data_len = ntohl(ul_data_len);

        // append data and update m_pack_head.length
        memcpy(m_pack_buffer + m_last_key_pos + 2 + 1 + 4 + ul_data_len,
                data, data_len);
        ul_data_len += data_len;
        m_pack_head.length += data_len;

        // update data length
        if (use_net_order)
            ul_data_len = htonl(ul_data_len);

        memcpy(m_pack_buffer + m_last_key_pos + 2 + 1, &ul_data_len,
                sizeof(ul_data_len));

        b = true;
    }

    return b;
}

void BaseProtocolPack::GetPackedHeadOfBlock(uint16_t service_type, uint16_t key,
                    uint32_t block_len, uint8_t **pptr_head, uint32_t *head_len)
{
    BaseProtocolHeader *proto_head =
        reinterpret_cast<BaseProtocolHeader *>(single_block_packed_head_);
    memcpy(proto_head, &m_pack_head, sizeof(m_pack_head));

    proto_head->option        &= (~kBaseProtocolOptCrcCheck);
    proto_head->first_seg_pos = sizeof(m_pack_head);
    proto_head->service_type  = service_type;

    uint8_t *ptr = reinterpret_cast<uint8_t *>(single_block_packed_head_) +
                    sizeof(m_pack_head);

    // set key
    uint16_t net_order_key = htons(key);
    memcpy(ptr, &net_order_key, sizeof(uint16_t));
    ptr += sizeof(net_order_key);

    // set data type
    ProtocolDataType data_type = kDataTypeUByte;
    memcpy(ptr, &data_type, sizeof(data_type));
    ptr += sizeof(data_type);

    // set block length
    uint32_t net_order_len = htonl(block_len);
    memcpy(ptr, &net_order_len, sizeof(net_order_len));

    // update length
    proto_head->length = sizeof(m_pack_head) + 7 + block_len;
    proto_head->ToNetOrder();

    // head+part of first seg. length
    *head_len  = sizeof(m_pack_head) + 7;
    *pptr_head = reinterpret_cast<uint8_t *>(proto_head);
}

bool BaseProtocolPack::SetKey(uint16_t key, int8_t val)
{
    uint8_t *value = reinterpret_cast<uint8_t *>(&val);
    return SetKeyValue(kDataTypeUChar, key, value, sizeof(val));
}

bool BaseProtocolPack::SetKey(uint16_t key, uint8_t val)
{
    return SetKeyValue(kDataTypeUChar, key, &val, sizeof(val));
}

bool BaseProtocolPack::SetKey(uint16_t key, int16_t val)
{
    uint8_t *value = reinterpret_cast<uint8_t *>(&val);
    return SetKeyValue(kDataTypeUShort, key, value, sizeof(val));
}

bool BaseProtocolPack::SetKey(uint16_t key, uint16_t val)
{
    uint8_t *value = reinterpret_cast<uint8_t *>(&val);
    return SetKeyValue(kDataTypeUShort, key, value, sizeof(val));
}

bool BaseProtocolPack::SetKey(uint16_t key, int32_t val)
{
    uint8_t *value = reinterpret_cast<uint8_t *>(&val);
    return SetKeyValue(kDataTypeUInt, key, value, sizeof(val));
}

bool BaseProtocolPack::SetKey(uint16_t key, uint32_t val)
{
    uint8_t *value = reinterpret_cast<uint8_t *>(&val);
    return SetKeyValue(kDataTypeUInt, key, value, sizeof(val));
}

bool BaseProtocolPack::SetKey(uint16_t key, float val)
{
    uint8_t *value = reinterpret_cast<uint8_t *>(&val);
    return SetKeyValue(kDataTypeUInt, key, value, sizeof(val));
}

bool BaseProtocolPack::SetKey(uint16_t key, double val)
{
    uint8_t *value = reinterpret_cast<uint8_t *>(&val);
    return SetKeyValue(kDataTypeULong, key, value, sizeof(val));
}

bool BaseProtocolPack::SetKey(uint16_t key, int64_t val)
{
    uint8_t *value = reinterpret_cast<uint8_t *>(&val);
    return SetKeyValue(kDataTypeULong, key, value, sizeof(val));
}

bool BaseProtocolPack::SetKey(uint16_t key, uint64_t val)
{
    uint8_t *value = reinterpret_cast<uint8_t *>(&val);
    return SetKeyValue(kDataTypeULong, key, value, sizeof(val));
}

bool BaseProtocolPack::SetKey(uint16_t key, uint8_t *data, uint32_t data_len)
{
    return SetKeyValue(kDataTypeUByte, key, data, data_len);
}

bool BaseProtocolPack::SetKey(uint16_t key, uint8_t *data)
{
    const char *string = reinterpret_cast<const char *>(data);

    return SetKeyValue(kDataTypeUByte, key, data, BaseProtocolStrLen(string)+1);
}

void BaseProtocolPack::GetPackage(uint8_t **pptr, uint32_t *data_len)
{
    if (pptr && m_pack_buffer)
    {
        // ?cal CRC8
        if ((m_pack_head.option & kBaseProtocolOptCrcCheck) ==
                kBaseProtocolOptCrcCheck)
        {
            uint32_t crc_result = CRC16Hash8(m_pack_buffer +
                sizeof(m_pack_head),
                m_pack_head.length - sizeof(m_pack_head));

            // uCRC<<=24; 如果需要CRC check,Option高位1byte存放CRC8
            crc_result *= 16777216;

            m_pack_head.option &= 0xFFFFFF;
            m_pack_head.option |= crc_result;
        }

        BaseEncryption::SetPseudoProHead(&m_pack_head.pseudo_head);

        m_pack_head.ToNetOrder();
        memcpy(m_pack_buffer, &m_pack_head, sizeof(m_pack_head));
        m_pack_head.ToHostOrder();

        *pptr = m_pack_buffer;

        if (data_len)
            *data_len = m_pack_head.length;
    }
}

// ============================================================================
// class BaseProtocolUnpack
// ============================================================================
BaseProtocolUnpack::BaseProtocolUnpack()
{
    m_seq           = 1;
    m_seg_val       = 0;
    m_ref_data      = 0;
    m_data_len      = 0;
    m_force_non_crc = false;
}

BaseProtocolUnpack::~BaseProtocolUnpack()
{
    // nothing to do
}

bool BaseProtocolUnpack::Init()
{
    bool b = false;

    if (!m_seg_val)
    {
        m_seg_val = new SegVal[65536]; // max uint16_t
        if (m_seg_val)
        {
            memset(m_seg_val, 0, sizeof(SegVal) * 65536);
            b = true;
        }
    }

    m_seq = 1;
    UntachPackage();

    return b;
}

void BaseProtocolUnpack::Uninit()
{
    if (m_seg_val)
    {
        delete [] m_seg_val;
        m_seg_val = NULL;
    }
}

bool BaseProtocolUnpack::GetReservedData(uint8_t **pptr, uint32_t *data_len)
{
    bool b = false;

    if (m_ref_data && m_data_len && m_data_len >= sizeof(BaseProtocolHeader))
    {
        BaseProtocolHeader head;
        memcpy(&head, m_ref_data, sizeof(BaseProtocolHeader));
        head.ToHostOrder();

        if (pptr)
            *pptr = m_ref_data + sizeof(BaseProtocolHeader);
        if (data_len)
            *data_len = head.first_seg_pos - sizeof(BaseProtocolHeader);

        b = true;
    }

    return b;
}

bool BaseProtocolUnpack::Unpack()
{
    bool b = false;

    if (m_ref_data && m_data_len >= sizeof(BaseProtocolHeader) && m_seg_val)
    {
        BaseProtocolHeader head;
        memcpy(&head, m_ref_data, sizeof(head));
        head.ToHostOrder();

        // check version
        if (head.version != kProtocolVer1)
            return false;

        // ?need CRC check
        // ?cal CRC8
        if (!m_force_non_crc && (head.option & kBaseProtocolOptCrcCheck ) ==
                kBaseProtocolOptCrcCheck)
        {
            // m_pack_head.Option>>=24;如果需要CRC check,Option高位1byte存放CRC8
            uint8_t crc = CRC16Hash8(m_ref_data +
                sizeof(BaseProtocolHeader),
                head.length - sizeof(BaseProtocolHeader));

            uint8_t crc_in_head = uint8_t(head.option / 16777216);

            if (crc != crc_in_head)
                return false;
        }

        uint32_t position = head.first_seg_pos;
        m_net_order = ((head.option & kBaseProtocolOptNetOrder) ==
            kBaseProtocolOptNetOrder);
        m_seq++;

        while (head.length > position)
        {
            // seg name
            uint16_t seg_name = 0;
            memcpy(&seg_name, m_ref_data + position, sizeof(uint16_t));

            if (m_net_order)
                seg_name = ntohs(seg_name);
            m_seg_val[seg_name].seq = m_seq;

            position += sizeof(uint16_t);

            // data type
            uint8_t type = 0;
            memcpy(&type, m_ref_data + position, sizeof(uint8_t));
            m_seg_val[seg_name].type = type;

            position += sizeof(uint8_t);

            // data length
            // real data
            switch (type)
            {
            case kDataTypeUChar:
                m_seg_val[seg_name].len = 1;
                m_seg_val[seg_name].data_offset = position;
                position += 1;
                break;
            case kDataTypeUShort:
                m_seg_val[seg_name].len = 2;
                m_seg_val[seg_name].data_offset = position;
                position += 2;
                break;
            case kDataTypeUInt:
                m_seg_val[seg_name].len = 4;
                m_seg_val[seg_name].data_offset = position;
                position += 4;
                break;
            case kDataTypeULong:
                m_seg_val[seg_name].len = 8;
                m_seg_val[seg_name].data_offset = position;
                position += 8;
                break;
            case kDataTypeUByte:
                {
                    uint32_t real_len = 0;
                    memcpy(&real_len, m_ref_data + position, sizeof(uint32_t));
                    if (m_net_order)
                        real_len = ntohl(real_len);

                    position += sizeof(uint32_t);

                    m_seg_val[seg_name].len            = real_len;
                    m_seg_val[seg_name].data_offset    = position;

                    position += real_len;
                }
                break;
            default:
                break;
            }
        }

        b = true;
    }
    else
    {
#ifdef _DEBUG
        fprintf(stderr, "Unpack: invalid parameter or not Init()");
#endif // _DEBUG
    }

    return b;
}

bool BaseProtocolUnpack::GetKeyVal(uint16_t key, ProtocolDataType type,
                                    void **pptr, uint32_t *val_len)
{
    bool b = false;

    if (pptr && m_ref_data && m_seg_val)
    {
        if (m_seg_val[key].seq == m_seq /*valid item?*/ && \
            m_seg_val[key].type == type /*check data type*/)
        {
            switch (type)
            {
            case kDataTypeUChar:
                memcpy(pptr, m_ref_data+m_seg_val[key].data_offset,
                        sizeof(uint8_t));
                break;
            case kDataTypeUShort:
                memcpy(pptr, m_ref_data+m_seg_val[key].data_offset,
                        sizeof(uint16_t));
                if (m_net_order)
                    *(uint16_t*)pptr = ntohs(*(uint16_t*)pptr); // NOLINT
                break;
            case kDataTypeUInt:
                memcpy(pptr, m_ref_data+m_seg_val[key].data_offset,
                        sizeof(uint32_t));
                if (m_net_order)
                    *(uint32_t*)pptr = ntohl(*(uint32_t*)pptr); // NOLINT
                break;
            case kDataTypeULong:
                memcpy(pptr, m_ref_data+m_seg_val[key].data_offset,
                        sizeof(uint64_t));
                if (m_net_order)
                    *(uint64_t*)pptr =
                        BaseProtocolN2H64(*(uint64_t*)pptr); // NOLINT
                break;
            case kDataTypeUByte:
                *pptr = m_ref_data + m_seg_val[key].data_offset;
                break;
            default:
                break;
            }

            if (val_len)
                *val_len = m_seg_val[key].len;

            b = true;
        }
    }

    return b;
}

bool BaseProtocolUnpack::GetVal(uint16_t key, uint8_t *pval)
{
    void **temple = reinterpret_cast<void **>(pval);
    return GetKeyVal(key, kDataTypeUChar, temple, 0);
}

bool BaseProtocolUnpack::GetVal(uint16_t key, int8_t *pval)
{
    void **temple = reinterpret_cast<void **>(pval);
    return GetKeyVal(key, kDataTypeUChar, temple, 0);
}

bool BaseProtocolUnpack::GetVal(uint16_t key, int16_t *pval)
{
    void **temple = reinterpret_cast<void **>(pval);
    return GetKeyVal(key, kDataTypeUShort, temple, 0);
}

bool BaseProtocolUnpack::GetVal(uint16_t key, uint16_t *pval)
{
    void **temple = reinterpret_cast<void **>(pval);
    return GetKeyVal(key, kDataTypeUShort, temple, 0);
}

bool BaseProtocolUnpack::GetVal(uint16_t key, int32_t *pval)
{
    void **temple = reinterpret_cast<void **>(pval);
    return GetKeyVal(key, kDataTypeUInt, temple, 0);
}

bool BaseProtocolUnpack::GetVal(uint16_t key, uint32_t *pval)
{
    void **temple = reinterpret_cast<void **>(pval);
    return GetKeyVal(key, kDataTypeUInt, temple, 0);
}

bool BaseProtocolUnpack::GetVal(uint16_t key, float *pval)
{
    void **temple = reinterpret_cast<void **>(pval);
    return GetKeyVal(key, kDataTypeUInt, temple, 0);
}

bool BaseProtocolUnpack::GetVal(uint16_t key, double *pval)
{
    void **temple = reinterpret_cast<void **>(pval);
    return GetKeyVal(key, kDataTypeULong, temple, 0);
}

bool BaseProtocolUnpack::GetVal(uint16_t key, int64_t *pval)
{
    void **temple = reinterpret_cast<void **>(pval);
    return GetKeyVal(key, kDataTypeULong, temple, 0);
}

bool BaseProtocolUnpack::GetVal(uint16_t key, uint64_t *pval)
{
    void **temple = reinterpret_cast<void **>(pval);
    return GetKeyVal(key, kDataTypeULong, temple, 0);
}

bool BaseProtocolUnpack::GetVal(uint16_t key, uint8_t **pptr,
                                uint32_t *len)
{
    void **temple = reinterpret_cast<void **>(pptr);
    return GetKeyVal(key, kDataTypeUByte, temple, len);
}
