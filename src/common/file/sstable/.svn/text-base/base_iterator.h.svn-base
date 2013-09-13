#ifndef SSTABLE_BASE_ITERATOR_H_
#define SSTABLE_BASE_ITERATOR_H_

#include <string>

namespace sstable
{
class BaseIterator
{
public:

    // 析构
    virtual ~BaseIterator() {}

    /// @brief   重新定位到起始位置;
    virtual void Reset() = 0;

    /// @brief   同步顺序读取下一条Record;
    /// @param   should_prefetch, true, 表示需要读文件时需要文件进行预取操作;
    //           false 表示不需要对文件进行预取操作;
    virtual void Next(bool should_prefetch = true) = 0;

    /// @brief   定位到某个Key的起始位置;
    /// @param   key, 输入key的指针;
    /// @param   key_len, key的长度;
    virtual void Seek(const void* key, uint32_t key_len) = 0;

    /// @brief   low_bound定位到某个Key的起始位置;
    /// @param   key, 输入key的指针;
    /// @param   key_len, key的长度;
    /// @param   is_accurate_hit, 输出参数, 表示是否完全命中, 为空时不取该值;
    virtual void LowBoundSeek(const void* key,
        uint32_t key_len, bool* is_accurate_hit = NULL) = 0;

    /// @brief 判断当前的Iterator是否为Eof;
    /// @retval 结束;
    virtual bool Done() const = 0;

    /// @brief   获取当前记录的解压缩完毕之后的key
    /// @param   key, 输出记录的key的Buffer, 当传入的参数为NULL时,
    ///          key_len指向的值必须预付为0, 相当于取Key的长度;
    /// @param   key_len, 输出记录的key len，输入时为存储key的Buffer长度
    /// @retval  kRetOK：       获取成功
    ///          kRetBufferOverflow：   失败，长度不够时，key_len 返回需要的长度;
    virtual RetCode GetKey(char* key, uint16_t* key_len) const = 0;

    /// @brief   获取当前记录的解压缩完毕之后的key
    /// @param   key, 出参数，当前记录key的string类型, 输入不能为NULl,
    /// @retval  无
    virtual void  GetKey(std::string* key) const = 0;

    /// @brief   获取当前记录的val的指针, 使用该函数时需要注意：
    ///           1: 当传入的val=NULL时, 相当于只取值的长度;
    ///           2: 获取出去的val的指针的有效生命周期和Iterator的一致;
    ///              当Iterator发生改变时(例如++操作之后), val的指针的内容可能会无效;
    /// @param   val, 出参数，值的指针的指针;
    /// @param   val_len, 输入输出参数, 该参数不能为NULL;
    /// @retval  无;
    virtual void GetValuePtr(char** val, uint32_t* val_len) const = 0;
    virtual void GetValuePtr(StringPiece* val) const = 0;

    /// @brief   获取当前记录的out_val;
    /// @param   val, 输出数据Buffer, 当传入的参数为NULL时,
    ///          val_len指向的值必须预付为0, 相当于取val的长度;
    /// @param   val_len,输出数据的长度， 输入时为存储数据的Buffer长度;
    /// @retval  kRetOK：       成功;
    ///          kRetBufferOverflow：   失败，长度不够时，val_len 返回需要的长度;
    virtual RetCode GetValue(char* val, uint32_t* val_len) const = 0;

    /// @brief   获取当前记录的data;
    /// @param   val, 出参数，当前数据的string类型
    /// @retval  无;
    virtual void  GetValue(std::string* val) const = 0;

    /// @brief   获取当前记录的Key和Value Pair;
    /// @param   key, 输出Key，
    ///          key_len, 输出Key的长度,输入时为存储key的Buffer长度
    ///          val, 输出Data,
    ///          val_len, data的长度, 输入时为存储数据的Buffer长度;
    /// @retval  kRetOK：    成功;
    ///          kRetBufferOverflow：失败，Buffer长度不够; 长度不够时,
    ///          key_len, val_len返回需要的长度;
    virtual RetCode GetKVPair(char* key, uint16_t* key_len,
        char* val, uint32_t* val_len) const = 0;

    /// @brief   获取当前记录的Key和Value Pair;
    /// @param   key, 出参数，当前记录key的string类型的指针
    ///          val, 出参数，当前数据的string类型的指针
    /// @retval  无;
    virtual void  GetKVPair(std::string* key, std::string* val) const = 0;

protected:

    BaseIterator() {}
};
} // sstable

#endif // SSTABLE_BASE_ITERATOR_H_
