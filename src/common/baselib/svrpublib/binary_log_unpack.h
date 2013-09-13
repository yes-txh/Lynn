#ifndef COMMON_BASELIB_SVRPUBLIB_BINARY_LOG_UNPACK_H_
#define COMMON_BASELIB_SVRPUBLIB_BINARY_LOG_UNPACK_H_

// ������������־��ʹ�÷���Ϊ
// BinaryLogUnpack unpack(binary_log_file_path)
// unpack.Serialize();
// ����binary_log_file_path���ڵ�Ŀ¼����һ���ļ�����Ϊ
// binary_log_file_path+"unpack.txt"���ı��ļ�
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
