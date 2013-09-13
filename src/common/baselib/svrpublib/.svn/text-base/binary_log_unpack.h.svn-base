#ifndef COMMON_BASELIB_SVRPUBLIB_BINARY_LOG_UNPACK_H_
#define COMMON_BASELIB_SVRPUBLIB_BINARY_LOG_UNPACK_H_

// 解析二进制日志，使用方法为
// BinaryLogUnpack unpack(binary_log_file_path)
// unpack.Serialize();
// 则在binary_log_file_path所在的目录生成一个文件名字为
// binary_log_file_path+"unpack.txt"的文本文件
//

#include <stdio.h>
#include <string>
#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/base_config.h"



_START_XFS_BASE_NAMESPACE_

class BinaryLogUnpack:public CBaseProtocolUnpack {
public:
    BinaryLogUnpack(std::string log_path):CBaseProtocolUnpack(),
        m_log_file(log_path),
        m_log_reader(NULL),
        m_log_writer(NULL) {}
    bool Serialize();
    ~BinaryLogUnpack();
private:
    std::string m_log_file;
    FILE*       m_log_reader;
    FILE*       m_log_writer;
};
_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_BINARY_LOG_UNPACK_H_
