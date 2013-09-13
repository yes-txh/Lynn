// expandable_array.h: 打包/解包各元素长度不定的数组.
// bayou@tencent.com
// 2010-09-04
// ////////////////////////////////////////////////////////////////////
#ifndef COMMON_BASELIB_SVRPUBLIB_EXPANDABLE_ARRAY_H_
#define COMMON_BASELIB_SVRPUBLIB_EXPANDABLE_ARRAY_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

// -count(4B)-, |--len(4B)--|--data(len)--|--len(4B)--|--data--|....


//
// class:       VarDataSerialize
// description: package expandable array
//

class VarDataSerialize {
public:
    VarDataSerialize();

    ~VarDataSerialize();

    //  清空该对象
    bool ResetContent();

    //  设置内存的增加步长
    void SetExtendStep(uint32_t extend_step);

    //  增加一段数据到变长数组中
    //  输入参数 data :   该段数据的地址
    //  输入参数  len :   该段数据的长度
    //  返回值： 增加是否成功
    bool AddVarData(const char* data, uint32_t len);

    //  得到变长数组的序列化结构
    //  输出参数 data :           该段数据的地址
    //  输出参数  len :           该段数据的长度
    //  输出参数  item_count :    该段数据的元素个数
    //  返回值： 获取是否成功
    bool GetPackage(const char** data,
                    uint32_t* len,
                    uint32_t* item_count = 0);

private:
    BufferV m_buff;

    //  是否已初始化m_buff.buff的头4字节为item_count
    bool m_is_inited;
};

class VarDataUnSerialize {
public:

    VarDataUnSerialize();

    ~VarDataUnSerialize();

    //  将一段数据设为需要反序列化的输入源
    //  输入参数 data :   该段数据的地址
    //  输入参数  len :   该段数据的长度
    //  返回值： 设置是否成功
    bool AttachPackage(const char* data, uint32_t len);

    //  获取下一个变长元素的内容
    //  输出参数 data :           该元素的地址
    //  输出参数  len :           该元素的内容长度
    //  输出参数  pos :           该元素所属序号
    //  返回值： 获取是否成功
    bool GetNextVal(const char** data, uint32_t* len, uint32_t* pos = 0);

    //  获取该段数据所含元素的数目
    //  返回值： 该段数据所含元素的数目
    uint32_t GetValidItemsCount() const;

private:
    const char*    m_data;              // 包的起始地址
    uint32_t       m_valid_len;         // 变长序列的有效元素个数
    const char*    m_cursor;            // 当前指向的这条数据的地址
    uint32_t       m_current_pos;       // 当前指向的这条数据的序号
    uint32_t       m_package_len;       // 记录Attach的数据包长度。
    // 防止GetNextVal()越界
};

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_EXPANDABLE_ARRAY_H_

