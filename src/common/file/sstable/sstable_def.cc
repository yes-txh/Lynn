#include "common/file/sstable/sstable_def.h"

namespace sstable
{
/// @brief   ȡ���ȼ��ߵ�ѹ���㷨 ѹ���㷨�����ȼ�, LZO > BMZ > QUICKLZ > GZIP > NONE;
/// @retval  ���ȼ��ߵ�ѹ���㷨;
CompressType CompareCompressType(CompressType type1, CompressType type2)
{
    CompressType ret_type = type1;

    switch (type1)
    {
    case intern::BlockCompressionCodec::BMZ:

        if (type2 == intern::BlockCompressionCodec::LZO)
        {
            ret_type = type2;
        }

        break;
    case intern::BlockCompressionCodec::LZO:
        break;
    case intern::BlockCompressionCodec::QUICKLZ:

        if ((type2 == intern::BlockCompressionCodec::BMZ)
            || (type2 == intern::BlockCompressionCodec::LZO))
        {
            ret_type = type2;
        }

        break;
    case intern::BlockCompressionCodec::ZLIB:
    case intern::BlockCompressionCodec::NONE:

        if (type2 != intern::BlockCompressionCodec::NONE)
        {
            ret_type = type2;
        }

        break;
    default:
        assert(0);
    }

    return ret_type;
}

/// @brief   ȡ���ȼ��ߵ�KV Type, �䳤���ȼ� > �������ȼ�
/// @retval  ���ȼ��ߵ�KV Type,
RecordKVType CompareKVType(RecordKVType type1, RecordKVType type2)
{
    assert((type1 != kTypeUnknown) && (type2 != kTypeUnknown));
    RecordKVType ret_type = kTypeUnknown;

    // val type;
    if ((((type1 & kValTypeMask) >> 4) == kTypeVariableLen)
        || (((type2 & kValTypeMask) >> 4) == kTypeVariableLen))
    {
        ret_type = static_cast<RecordKVType>(ret_type | (kTypeVariableLen << 4));
    }
    else
    {
        ret_type = static_cast<RecordKVType>(ret_type | (kTypeFixedLen << 4));
    }

    // key type
    if (((type1 & kKeyTypeMask) == kTypeVariableLen)
        || ((type2 & kKeyTypeMask) == kTypeVariableLen))
    {
        ret_type = static_cast<RecordKVType>(ret_type | kTypeVariableLen);
    }
    else
    {
        ret_type = static_cast<RecordKVType>(ret_type | kTypeFixedLen);
    }
    return ret_type;
}

/// @breif  �Ƚ�����prefixѹ��key�Ĵ�С;
/// @param  key1,
/// @param  key2,
/// @retval <0: ��ʾkey1<key2;
///         0:  ��ʾkey1==key2;
///         >0: ��ʾkey1>key2;
int16_t ComparePrefixCompressedKey(const PrefixCompressedKey& key1,
                                   const PrefixCompressedKey& key2)
{
    int ret = 0;

    bool is_inclusive = false;

    if (key1.prefix_key_len == 0)
    {
        assert((key1.remainder_key != NULL) && (key1.remainder_key_len != 0));

        if (key2.prefix_key_len == 0)
        {
            assert((key2.remainder_key != NULL) && (key2.remainder_key_len != 0));
            ret = CompareByteString(key1.remainder_key, key1.remainder_key_len,
                                key2.remainder_key, key2.remainder_key_len);
            return ret;
        }

        assert(key2.prefix_key != NULL);
        ret = CompareByteString(key1.remainder_key, key1.remainder_key_len,
                            key2.prefix_key, key2.prefix_key_len, &is_inclusive);

        if (ret == 0)   // �����Ӵ����;
        {
            ret = key2.remainder_key_len > 0 ? -1 : 0;
        }
        else if ((ret > 0) && (is_inclusive))
        {
            // key1��remainder����key2��prefix;
            ret = CompareByteString(key1.remainder_key + key2.prefix_key_len,
                                key1.remainder_key_len - key2.prefix_key_len,
                                key2.remainder_key, key2.remainder_key_len);
        }

        return ret;
    }

    assert(key1.prefix_key != NULL);

    if (key2.prefix_key_len == 0)
    {
        assert((key2.remainder_key != NULL) && (key2.remainder_key_len != 0));
        ret = CompareByteString(key1.prefix_key, key1.prefix_key_len,
                            key2.remainder_key, key2.remainder_key_len, &is_inclusive);

        if (ret == 0)   // �����Ӵ����;
        {
            ret = key1.remainder_key_len > 0 ? 1 : 0;
        }
        else if ((ret < 0) && (is_inclusive))
        {
            // key2��remainder����key1��prefix;
            ret = CompareByteString(key1.remainder_key, key1.remainder_key_len,
                key2.remainder_key + key1.prefix_key_len,
                key2.remainder_key_len - key1.prefix_key_len);
        }
        return ret;
    }

    assert(key2.prefix_key != NULL);

    ret = CompareByteString(key1.prefix_key, key1.prefix_key_len,
                        key2.prefix_key, key2.prefix_key_len, &is_inclusive);

    if (ret == 0)   // �����Ӵ����;
    {
        ret = CompareByteString(key1.remainder_key, key1.remainder_key_len,
                            key2.remainder_key, key2.remainder_key_len);
        return ret;
    }

    if ((ret > 0) && (is_inclusive))
    {
        ret = CompareByteString(key1.prefix_key + key2.prefix_key_len,
                            key1.prefix_key_len - key2.prefix_key_len,
                            key2.remainder_key, key2.remainder_key_len, &is_inclusive);

        if (ret == 0)
        {
            ret = key1.remainder_key_len > 0 ? 1 : 0;
            return ret;
        }

        if ((ret < 0) && (is_inclusive))
        {
            ret = CompareByteString(key1.remainder_key,
                                key1.remainder_key_len,
                                key2.remainder_key + key1.prefix_key_len - key2.prefix_key_len,
                                key2.remainder_key_len - key1.prefix_key_len + key2.prefix_key_len);
        }
    }
    else if ((ret < 0) && (is_inclusive))
    {
        ret = CompareByteString(key1.remainder_key, key1.remainder_key_len,
            key2.prefix_key + key1.prefix_key_len,
            key2.prefix_key_len - key1.prefix_key_len, &is_inclusive);

        if (ret == 0)
        {
            ret = key2.remainder_key_len > 0 ? -1 : 0;
            return ret;
        }

        if ((ret > 0) && (is_inclusive))
        {
            ret = CompareByteString(key1.remainder_key + key2.prefix_key_len - key1.prefix_key_len,
                key1.remainder_key_len + key1.prefix_key_len - key2.prefix_key_len,
                key2.remainder_key,
                key2.remainder_key_len);
        }
    }
    return ret;
}

} // namespace sstable

