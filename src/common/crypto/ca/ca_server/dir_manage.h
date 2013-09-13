//////////////////////////////////////////////////////////////////////////
// @file:   dir_manage
// @brief:  һЩ��Ŀ¼���ļ��Ľ�����ɾ���Ĺ���
// @author: joeytian@tencent.com
// @time:   2010-11-24
// �޸���ʷ:
//          <author>    <time>
//////////////////////////////////////////////////////////////////////////
#ifndef COMMON_CRYPTO_CA_CA_SERVER_DIR_MANAGE_H_
#define COMMON_CRYPTO_CA_CA_SERVER_DIR_MANAGE_H_

#ifdef WIN32
#include <stdlib.h>
#include <direct.h>
#include <io.h>
#include <sys/stat.h>

// linux��windows���ļ�API��ͬ
#define stat _stat
#define access _access
#define R_OK 04
#define W_OK 02
#define S_IFREG _S_IFREG
#define S_IFDIR _S_IFDIR
#else
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/resource.h>
#endif
#include <stdio.h>
#include <string>
#include <fcntl.h>
#include "common/crypto/ca/ca_public/ca_struct.h"

namespace ca {

class DirManage {
public:
    DirManage();
    ~DirManage();

    bool MkDir(const char* dir_name);
    bool IsDir(const char* dir_name);
    bool IsFile(const char* file_name);
    bool IsDirExist(const char* dir_name);
    bool IsFileExist(const char* file_name);
    bool IsFileReadable(const char *file_name);
    bool RecursivelyRmDir(const char* path);

    // ���������common������ȡ�������޸ģ���ȡ�ļ�ʱ�������˰�ָ����delim���ָ�ÿһ��
    //std::string ReadTextFile(const char *text_file_name, const char* delim = "\r\n");

    // ���ܣ�׷�ӷ�ʽд�ļ���֧�ֿɱ������ʹ������printf
    // ���������common����file_utility.h��ȡ��������ͷ�ļ�������ɾ��
    bool WriteFile(const char *file_name, const char* fmt, ...);

    // ���ܣ����ı��ļ�����Ҫ������2�����ļ���
    // ���������common����file_utility.h��ȡ��������ͷ�ļ�������ɾ��
    std::string ReadTextFile(const char *file_name);

    // ���ܣ����ı��ļ�����Ҫ������2�����ļ���
    // ��ָ����ȡ�ļ��ĵڼ��е��ڼ���, �����Է����ļ����������ͳɹ���ȡ������
    // got_line_num: �ɹ���ȡ������
    // total_line_num: �ļ���������
    std::string ReadTextFile(const char *file_name, int32_t start_line_num, int32_t end_line_num,
                             int32_t* got_line_num, int32_t* total_line_num);
};

} // namespace ca
#endif // COMMON_CRYPTO_CA_CA_SERVER_DIR_MANAGE_H_
