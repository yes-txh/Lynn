// Copyright 2010, Tencent Inc.
//
// ����: typherque
// ����: 2010.04.14
// �汾: 1.1
// ģ������: �ļ�ϵͳSDK�ӿ�,֧��XFS, LocalFile...
//
// �޸���ʷ:
// <author>    <time>    <version >   <desc>    <build number>    <last update>
// typherque  2010.04.14    0.1        ����     001
// typherque  2010.04.14    0.1        update   020               2010-07-21
// wookin     2010.08.05    0.1        update   021               2010-08-05
// typherque  2010.08.23    0.1        update   022               2010-08-23
// aaronzou   2011.01.05    0.1        Refactor 023               2011-01-10


#ifndef COMMON_FILE_FILE_H_
#define COMMON_FILE_FILE_H_

#include <string.h>

#include <map>
#include <string>
#include <vector>

#include "common/base/class_register.h"
#include "common/base/closure.h"
#include "common/base/platform_features.hpp"
#include "common/base/stdint.h"

// Define the options when open a file.
// Some options may NOT work for all file implementation.
struct OpenFileOptions {
    static const uint32_t   kDefaultCacheBufLen = 0;
    static const uint32_t   kDefaultCacheDataInterval = 1000; // microsends
    static const uint8_t    kDefaultFileBackupFactor = 3;

    explicit OpenFileOptions():
        cache_buf_len(kDefaultCacheBufLen),
        cache_data_interval(kDefaultCacheDataInterval),
        backup_factor(kDefaultFileBackupFactor) {
    }

    uint32_t    cache_buf_len; // Only use cache when buffer length > 0
    uint32_t    cache_data_interval; // cache invalidation time, microsends.
    uint8_t     backup_factor; // The number of file replica.
};

// A helper class to construct and parse key-value pairs infomation.
// The info is in the form of: key1=value1;key2=value2.
// This helper can be used in parse additional info.
// Such as additional info in  OpenFileOptions and AttrsInfo.
class KeyValueInfoHelper {
public:
    // parse the key-value info string to info_map.
    // Return true if all parse OK.
    static bool ParseKeyValueInfo(
        const std::string& info,
        std::map<std::string, std::string>* info_map);

    // construct the key-value info string from a map's key-value pairs.
    // The result is store in input info.
    // Return true if all parse OK.
    static bool CreateKeyValueInfo(
        const std::map<std::string, std::string>& info_map,
        std::string* info);

    // Append a key-value pair to the given info.
    // Return true if append OK.
    static bool AppendKeyValueInfo(const std::string& key, 
        const std::string& value, std::string* info);
};

// �����û����ļ���Ϣ�ṹ
struct AttrsMask {
    // should has mask
    unsigned char   file_type: 1;
    unsigned char   file_id: 1;

    unsigned char   file_owner: 1;
    unsigned char   file_group: 1;
    unsigned char   file_permission: 1;
    unsigned char   modify_user: 1;

    unsigned char   create_time: 1;
    unsigned char   modify_time: 1;
    unsigned char   access_time: 1;

    unsigned char   file_size: 1;
    unsigned char   backup_factor: 1;

    // addtional_info as a string.
    unsigned char   additional_info: 1;

    AttrsMask() {
        memset(this, 0, sizeof(*this));
    }
};

enum ENUM_FILE_TYPE {
    FILE_TYPE_UNKNOWNTYPE = 0,
    FILE_TYPE_NORMAL = 1,
    FILE_TYPE_DIR = 2,
    FILE_TYPE_RA = 3, // Record Append type, currently no implementation.
};

struct AttrsInfo {
    std::string         file_name;
    ENUM_FILE_TYPE      file_type;
    uint64_t            file_id;

    std::string         file_owner;
    std::string         file_group;
    uint16_t            file_permission;
    std::string         modify_user;

    // time in seconds since the epoch 1970.
    uint32_t            create_time; 
    uint32_t            modify_time;
    uint32_t            access_time;

    // signed, so that the -1 indicate un assigned value.
    int64_t             file_size;
    // unsigned, 0 means init value.
    uint8_t             backup_factor;
   
    // additional info should be interpreted by file implementations.
    // Usually in the form of key1=value1;key2=value2
    std::string         additional_info; 

    AttrsInfo() : 
        file_type(FILE_TYPE_UNKNOWNTYPE),
        file_id(0),
        file_permission(0), 
        create_time(0), modify_time(0), access_time(0), 
        file_size(-1), backup_factor(0) {
    }
};

struct Attrs {
    AttrsMask       file_meta_mask;
    AttrsInfo       file_meta_info;
};

struct DataLocation {
    uint32_t        net_order_ip;
    uint16_t        net_order_port;
    uint64_t        start;
    uint64_t        end;
    uint32_t        chunk_index;
    uint32_t        total_bak_num;
    uint32_t        bak_sequence;

    // ����ռ���ļ��İٷֱ�
    float           size_percent;

    DataLocation() {
        memset(this, 0, sizeof(*this));
    }
};


const char* GetFileTypeDesc(ENUM_FILE_TYPE type);

//
// FILE Public Error Code
// File���������
//
#ifndef XFS_ERR_START_VALUE
#define XFS_ERR_START_VALUE 0x5F5E100
#endif//

enum FILE_ERROR_CODE {
    ERR_FILE_OK     = 1,
    ERR_FILE_FAIL   = XFS_ERR_START_VALUE + 1,
    ERR_FILE_REOPEN_FOR_WRITE,
    ERR_FILE_RETRY, 
    ERR_FILE_ENTRY_EXIST,
    ERR_FILE_ENTRY_NOT_EXIST,
    ERR_FILE_NOT_INIT,
    ERR_FILE_CLOSED,
    ERR_FILE_OPENMODE,
    ERR_FILE_PARAMETER,
    ERR_FILE_PERMISSION_DENIED,
    ERR_FILE_NOT_EMPTY_DIRECTORY,
};

const char* GetFileErrorCodeStr(unsigned int error_code);

class File {
public:
    static const char kPathSeparator = '/';

    // @brief:       ����Open�е�flag����
    enum FILE_FLAG {
        ENUM_FILE_OPEN_MODE_R     = 0x01,
        ENUM_FILE_OPEN_MODE_W     = 0x02,
        ENUM_FILE_OPEN_MODE_A     = 0x04,
        ENUM_FILE_TYPE_RA_FILE    = 0x200,    ///< ������������ͨ�ļ�
        ENUM_FILE_IO_NON_BLOCKING = 0X1000,   ///< ����������ͬ��,���������첽
    };

    // @brief:      ��ʼ��SDKģ��,�ڽ����г�ʼ������һ��,
    //              ����Ҫÿ��File���󶼵���
    //
    // @param:      identity,��֤�û������,�����ļ����ʵ�ʱ���ʹ���û���ĳһ��role,
    //              ,Ĭ��Ϊ���û���ͬ����һ��role. 
    //              ʹ�ò�ͬ��role? �μ�class Certifier��ʹ��.
    //
    // @return:     true, �ɹ�
    //              false,ʧ��
    static bool     Init(const char* identity = NULL);

    // @brief:      ��һ���ļ�
    // @param:      file_path   Ҫ�򿪵��ļ�ȫ·��,�����Ǳ����ļ���/xfs/�����ļ�
    //                          ����������ʽ����ѡ��.
    //                          file_path:k1=v1:k2=v2
    //                          ѡ��������ļ�ʵ�ֽ���.
    //                          �����ӵ�ѡ���봫���options�ظ�ʱ���Դ˴�Ϊ׼.
    // @param:      flags       Ҫ���ļ���ģʽ,��������ͨ�ļ����߼�¼���ļ�,
    //                          ��дȨ�ޣ��첽��ͬ����д
    // @param:      options     ѡ������Ƿ�ʹ�ÿͻ���cache������������
    // @param:      error_code  ���ڷ��س������
    // @return:     ��ʾ������ļ�ָ��; NULL ��ʾ����
    //
    // @note:       flags����ģʽ�����壺
    // W:           ��������ڻὨ���ļ�;����ļ����ڣ������֮ǰ�ļ�ɾ����
    //              ����һ���µ��ļ�;�ļ�ָ�����ļ�ͷ;
    //
    // A:           ����ļ�����,���ļ�β׷��;��������,����һ���ļ�,
    //              �ļ���дָ�������;
    //
    // R:           ֻ�ܴ��Ѵ��ڵ��ļ�,�ļ�ָ�����ļ�ͷ;
    // R|W:         �ļ�������ʱ,���½��ļ�,�ļ�ָ����ͷ��;�ļ�����ʱ��
    //              ��д�ļ�������Զ��ƶ����ļ�β;
    //
    // W|A:         ������;
    // R|A:         ��RW��ͬ;
    // R|W|A:       ��RW��ͬ;
    static File*    Open(const char*   file_path,
                         uint32_t      flags,
                         const OpenFileOptions& options = OpenFileOptions(),
                         uint32_t* error_code = NULL); 

    // @brief:      ��һ���ļ���һ�ӿ�, ���������Ҫ���ɹ�
    //              Ҫ�������������ɸ�����Ҫѡ�ã�
    // @param:      file_path   Ҫ�򿪵��ļ�ȫ·��,�����Ǳ����ļ���/xfs/�����ļ�
    // @param:      flags       Ҫ���ļ���ģʽ,��������ͨ�ļ����߼�¼���ļ�,
    //                          ��дȨ�ޣ��첽��ͬ����д
    // @param:      options     ��д�ļ���ѡ��
    // @return:     ��ʾ������ļ�ָ��; NULL ��ʾ����
    //
    // @note:       flags����ģʽ�����壺
    // W:           ��������ڻὨ���ļ�;����ļ����ڣ������֮ǰ�ļ�ɾ����
    //              ����һ���µ��ļ�;�ļ�ָ�����ļ�ͷ;
    //
    // A:           ����ļ�����,���ļ�β׷��;��������,����һ���ļ�,
    //              �ļ���дָ�������;
    //
    // R:           ֻ�ܴ��Ѵ��ڵ��ļ�,�ļ�ָ�����ļ�ͷ;
    // R|W:         �ļ�������ʱ,���½��ļ�,�ļ�ָ����ͷ��;�ļ�����ʱ��
    //              ��д�ļ�������Զ��ƶ����ļ�β;
    //
    // W|A:         ������;
    // R|A:         ��RW��ͬ;
    // R|W|A:       ��RW��ͬ;
    static File*    OpenOrDie(const char* file_path, uint32_t flags, 
        const OpenFileOptions& options = OpenFileOptions());

    // @brief:      ͬ�����ļ���ȡ����
    // @param:      buffer      ��ʾ������ݵı����ڴ�ռ䣻
    // @param:      size        ��ʾ��Ҫ��ȡ��������ݳ��ȣ�
    // @param:      error_code  ��Ŵ�����Ϣ��
    // @return:     -1 ����
    //              0  �������ļ���β
    //              >0 ʵ�ʶ�ȡ�����ļ�����
    virtual int64_t Read(void*     buffer,
                         int64_t   size,
                         uint32_t* error_code = NULL) = 0;

    // @brief:      ͬ����ʽ��򿪵��ļ�д�����ݣ��ļ�ͨ�������ڱ���,
    //              ����Flush��Closeʱˢ�µ������ļ�
    //
    // @param:      buffer      ָ���д�������
    // @param:      size        ��д����ļ�����
    // @param:      error_code  ��ų�����Ϣ,
    //
    // @return:     >=0 �ɹ�(д��ĳ���);
    //              -1  ʧ��
    virtual int64_t Write(const void* buffer,
                          int64_t     size,
                          uint32_t*   error_code = NULL) = 0;

    // @brief:      ���ļ���ָ��λ���첽��ȡ����
    // @param:      buffer          ��ʾ������ݵı����ڴ�ռ䣻
    // @param:      size            ��ʾ��Ҫ��ȡ��������ݳ��ȣ�
    // @param:      start_position  ��ʾ�Ӹ�λ�ÿ�ʼ��ȡ���ݣ�
    // @param:      callback        �뵱ǰ�������Ӧ�Ļص�����
    //                              (void:��ʾ�ص������ķ���ֵ,�������ǲ���:
    //                              ��һ����ʾ�ɹ�����ĳ���;
    //                              �ڶ���Ϊerrorcode)
    //
    // @param:      timeout         ��ʱʱ��,seconds
    // @param:      error_code      ���������Ϣ
    // @return:     =0 �ɹ�
    //              -1 ʧ��
    virtual int32_t AsyncReadFrom(void* buffer,
                                  int64_t size,
                                  int64_t start_position,
                                  Closure<void, int64_t, uint32_t>* callback,
                                  uint32_t  timeout = 60*60,
                                  uint32_t* error_code = NULL)=0;

    // @brief:      �첽��ʽ��򿪵��ļ�д�����ݣ��ļ�ͨ�������ڱ���,
    //              ����Flush��Closeʱˢ�µ������ļ�
    //
    // @param:      buffer      ָ���д�������
    // @param:      size        ��д����ļ�����
    // @param:      callback    ����д��ɹ���ûص�������������
    //                          (void:��ʾ�ص������ķ���ֵ,�������ǲ���:
    //                          ��һ����ʾ�ɹ�����ĳ���;
    //                          �ڶ���Ϊerrorcode)
    //
    // @param:      timeout     ��ʱʱ��,seconds
    // @param:      error_code  ��ų�����Ϣ,
    //
    // @return:     =0 �ɹ�(д��ĳ���)
    //              -1 ʧ��
    virtual int32_t AsyncWrite(const void* buffer,
                               int64_t     size,
                               Closure<void, int64_t, uint32_t>* callback,
                               uint32_t    timeout = 60 * 60,
                               uint32_t*   error_code = NULL) = 0;

    // @brief:      The file implementation support asynchronous operations?
    //              For file implementation doesn't support aync operaton,
    //                  the behavior of calling async operation is undefined.
    // @return:     return true if support async operations.
    virtual bool SupportAsync() = 0;

    // @brief:      ����������ˢ������������������
    // @param:      error_code  ��ų�����Ϣ
    // @return:     0  �ɹ�
    //              <0 ʧ��
    virtual int32_t Flush(uint32_t* error_code = NULL) = 0;

    // @brief:      �ر�һ���ļ�
    // @param:      error_code  ��ų�����Ϣ
    // @return:     0 �ɹ�; -1 ʧ��
    //
    // @note:       �ر�֮ǰ����������Flushһ��
    virtual int32_t Close(uint32_t* error_code = NULL) = 0;

    // @brief:      �����ļ�����,�ݲ�֧�ֺ�ͨ����Ĵ�����������
    // @param:      src_file_path   Ŀ��Դ�ļ�ȫ·��
    // @param:      dest_file_path  Ŀ���ļ�ȫ·��
    // @param:      error_code      ���������Ϣ
    // @return:     0 �ɹ�
    //              -1 ʧ��
    static int32_t  Copy(const char*    src_file_path,
                         const char*    dest_file_path,
                         uint32_t*      error_code = NULL);

    // @brief:      �ƶ��ļ�����Ŀ¼
    // @param:      src_name    Դ�ļ���Ŀ¼ȫ·����Ŀ¼����/��β
    // @param:      dest_name   Ŀ��Ŀ¼ȫ·����Ŀ¼����/��β
    // @param:      error_code  ���������Ϣ
    // @return:     0 �ɹ�
    //              -1 ʧ��
    static int32_t  Move(const char* src_name,
                         const char* dest_name,
                         uint32_t*   error_code = NULL);

    // @brief:      �޸��ļ�����Ŀ¼����
    // @param:      old_path_name   �ɵ��ļ���Ŀ¼ȫ·����,Ŀ¼����/��β
    // @param:      new_path_name   �µ��ļ���Ŀ¼����Ŀ¼����/��β
    // @param:      error_code      ���������Ϣ
    // @return:     0 �ɹ�
    //              -1 ʧ��
    static int32_t  Rename(const char* old_path_name,
                           const char* new_path_name,
                           uint32_t*   error_code = NULL);

    // @brief:      ɾ���ļ�����Ŀ¼
    // @param:      path_name       ��ɾ�����ļ���Ŀ¼ȫ·����,Ŀ¼����/��β
    // @param:      is_recursive    �Ƿ�ݹ�ɾ�������ļ�����Ŀ¼, 
    //                              �����ļ�ʵ�ֲ�֧��.
    // @param:      error_code      ���������Ϣ
    // @return:     0 �ɹ�
    //              -1 ʧ��
    static int32_t  Remove(const char* path_name,
                           bool        is_recursive = false,
                           uint32_t*   error_code = NULL);

    // @brief:      �����Ŀ¼
    // @param:      path_name   Ŀ¼ȫ·����,Ŀ¼����/��β
    // @param:      error_code  ���������Ϣ
    // @return:     0 �ɹ�
    //              -1 ʧ��
    static int32_t  AddDir(const char* path_name, uint32_t* error_code = NULL);



    // @brief:      �鿴ָ��Ŀ¼���ļ���������Ϣ
    // @param:      pattern     Ҫ�鿴��Ŀ¼���ļ���֧��* ? [].
    // @param:      mask        ��Ҫ��ȡ��������Ϣ����. 
    //                          ���к����ó��ļ�ʵ��ʵ��֧�ֵ�����.
    // @param:      buffer      �ŷ��ص�������Ϣ,Ӧ�ò����Ļ�����.
    //                          ĳЩ�ֶο������ڻ�ȡ��Ϣʧ�ܶ�����Ϊ��Ĭ��ֵ.
    // @param:      error_code  ���������Ϣ
    // @return:     0 �ɹ�; -1 ʧ��
    static int32_t List(const char*    pattern,
                        AttrsMask* mask,
                        std::vector<AttrsInfo>* buffer,
                        uint32_t* error_code = NULL);

    // @brief:      �ı��ļ��ĵ�ǰƫ����
    // @param:      offset      �����origin��ƫ����
    // @param:      whence      ƫ���������λ��
    //              ��ѡ����:
    //                  SEEK_SET    �ļ���ʼ
    //                  SEEK_CUR    �ļ��ĵ�ǰλ��
    //                  SEEK_END    �ļ���β
    // @param:      error_code  ���س�����Ϣ
    // @return:     >=0 �ɹ�(���ص�ǰ�ļ���ƫ����)
    //              1 ����
    virtual int64_t Seek(int64_t   offset,
                         int32_t   whence,
                         uint32_t* error_code = NULL) = 0;

    // @brief:      �鿴�ļ�����Ŀ¼ռ�õĿռ�
    // @param:      path_name   �鿴���ļ���Ŀ¼ȫ·������Ŀ¼����/��β
    // @param:      error_code  ���������Ϣ
    // @return:     >=0 �ɹ�(�����ļ�����Ŀ¼��С)
    //              -1 ʧ��
    static int64_t  Du(const char* path_name, uint32_t* error_code = NULL);

    // @brief:      ���ص�ǰ�ļ���ƫ����
    // @param:      error_code   ���������Ϣ
    // @return:     >=0 �ɹ�(���ص�ǰ�ļ���ƫ����);-1 ����
    virtual int64_t Tell(uint32_t* error_code = NULL) = 0;

    // @brief:      �����ļ���Ч��С���ض̣�
    // @param:      length      �ļ���Ч��С
    // @param:      error_code  ���������Ϣ
    // @return:     >=0 �ɹ�
    //              -1 ����
    virtual int32_t Truncate(uint64_t length, uint32_t* error_code = NULL) = 0;


    // @brief:      ��ȡ�������ڵ�NodeServeλ��
    // @param:      start       ���ݿ�Ŀ�ʼλ��
    // @param:      end         ���ݿ�Ľ���λ��
    // @param:      buffer      ��ȡ����λ����Ϣ
    // @param:      error_code  ����������
    // @return:     0 �ɹ�
    //              -1 ʧ��
    virtual int32_t LocateData(uint64_t      start,
                               uint64_t      end,
                               std::vector<DataLocation>* buffer,
                               uint32_t*     error_code = NULL) = 0;

    // @brief:      ����ļ���С
    // @param:      file_name   Ŀ¼ȫ·����
    // @param:      error_code  ���������Ϣ
    // @return:     >=0 �ɹ�
    //              -1 ʧ��
    static int64_t  GetSize(const char* file_name,
                            uint32_t*   error_code = NULL);

    // @brief:      ��ȡָ��Ŀ¼�µ��ļ�����
    // @param:      pattern      Ҫ�鿴��Ŀ¼���ļ���Ŀ¼��/��β
    // @param:      files        �ŷ��ص������ļ����У�����ҳ��. 
    // @return:     0 �ɹ�; -1 ʧ��
    static int32_t GetMatchingFiles(const char*     pattern,
                                    std::vector<std::string>* files);

    // @brief:      �鿴ָ���ļ�����Ŀ¼�Ƿ����
    // @param:      path         Ҫ�鿴���ļ���ȫ·������Ŀ¼��/��β
    // @param:      error_code   ���������Ϣ
    // @return:     true �ɹ�(����)
    //              false ʧ��
    static bool IsExist(const char* path_name, uint32_t* error_code = NULL);

    // @brief:      �鿴ָ���ļ�����Ŀ¼�Ƿ����
    // @param:      path         Ҫ�鿴���ļ���ȫ·������Ŀ¼��/��β
    // @param:      error_code   ���������Ϣ
    // @return:     true �ɹ�(����)
    //              false ʧ��
    // DEPRECATED_BY(IsExist)
    static bool CheckExist(const char* path_name, uint32_t* error_code = NULL);

    // @brief:      �޸��ļ���Ŀ¼Ȩ��
    // @param:      path_name    Ҫ�޸ĵ��ļ���ȫ·����
    // @param:      permission   Ŀ��Ȩ��
    // @param:      error_code   ���������Ϣ
    // @return:     true �ɹ�(����)
    //              false ʧ��
    static bool Chmod(const char* path_name,
                      const uint32_t permission,
                      uint32_t* error_code = NULL);

    // @brief:      �޸��ļ���Ŀ¼��role,ע��ֻ��xfs_admin�����޸�xfs���ļ���Role
    // @param:      path_name     Ҫ�޸ĵ��ļ���ȫ·����
    // @param:      role_name     xfs�ļ�,�˲���Ȩ�޸����role;
    //                            local�ļ�, �˲���Ϊuser_name:group_name
    // @param:      error_code    ���������Ϣ
    // @return:     true �ɹ�(����)
    //              false ʧ��
    static bool ChangeRole(const char* path_name,
                           const char* role_name,
                           uint32_t* error_code = NULL);

    // @brief:      ����SDKģ��
    // @return:     void
    static void CleanUp();


    virtual std::string GetFileImplName() = 0;

    // @brief:      Get the prefix of a file name. The prefix must start with
    //              a segement char '/'.
    //              The prefix is the registered file system name.
    // @param:      file_path   the test file path
    // @return:     the prefix of a file name, or an empty string if fails.
    static std::string GetFilePrefix(const char* file_path);
    
    virtual ~File() {}

protected:
    // @brief��     Create a File object for the file implementation.
    // @param:      prefix      the file prefix, 
    //                          which is also the file implementation name.
    // @return:     the pointer to the newly created subclass File object. 
    //              For unknown prefix, return default file implementation.
    //              Return NULL for failures.
    static File* CreateFileImpl(const std::string& prefix);
    
    // @brief:      Get the singleton File object for the file implementation.
    // @param:      prefix      the file prefix, 
    //                          which is also the file implementation name.
    // @return:     the pointer to the singleton subclass File object.
    //              For unknown prefix, return the default file implementation.
    //              Return NULL for failures.
    static File* GetFileImplSingleton(const std::string& prefix);

protected:

    // File system level operation implementation.
    // Each operation is the virtual one for the corresponding static method.
    // Name conversion: has the posfix impl for implementation.
    
    // @brief:      ��ʼ��SDKģ��,�ڽ����г�ʼ������һ��,
    //              ����Ҫÿ��File���󶼵���
    //
    // @param:      identity   �����ļ����û���(rtx name)��Ĭ��Ϊ��ǰ�û�
    // @return:     true, �ɹ�
    //              false,ʧ��
    // @see:        Init
    virtual bool InitImpl(const char* identity = NULL) = 0;

    // @brief:      ����SDKģ��
    // @return:     void
    // @see:        Cleanup 
    virtual bool CleanupImpl() = 0;

    //
    // @brief:      ��һ���ļ�
    // @param:      file_path   Ҫ�򿪵��ļ�ȫ·��,�����Ǳ����ļ���/xfs/�����ļ�
    //                          ����������ʽ����ѡ��.
    //                          file_path:k1=v1:k2=v2
    //                          ѡ��������ļ�ʵ�ֽ���.
    //                          �����ӵ�ѡ���봫���options�ظ�ʱ���Դ˴�Ϊ׼.
    // @param:      flags       Ҫ���ļ���ģʽ,��������ͨ�ļ����߼�¼���ļ�,
    //                          ��дȨ�ޣ��첽��ͬ����д
    // @param:      options     ѡ������Ƿ�ʹ�ÿͻ���cache������������
    // @param:      error_code  ���ڷ��س������
    // @return:     ��ʾ������ļ�ָ��; NULL ��ʾ����
    //
    // @note:       flags����ģʽ�����壺
    // W:           ��������ڻὨ���ļ�;����ļ����ڣ������֮ǰ�ļ�ɾ����
    //              ����һ���µ��ļ�;�ļ�ָ�����ļ�ͷ;
    //
    // A:           ����ļ�����,���ļ�β׷��;��������,����һ���ļ�,
    //              �ļ���дָ�������;
    //
    // R:           ֻ�ܴ��Ѵ��ڵ��ļ�,�ļ�ָ�����ļ�ͷ;
    // R|W:         �ļ�������ʱ,���½��ļ�,�ļ�ָ����ͷ��;�ļ�����ʱ��
    //              ��д�ļ�������Զ��ƶ����ļ�β;
    //
    // W|A:         ������;
    // R|A:         ��RW��ͬ;
    // R|W|A:       ��RW��ͬ;
    // @see:        Open
    virtual bool OpenImpl(const char *file_path, uint32_t flags,
                        const OpenFileOptions& options = OpenFileOptions(),
                        uint32_t *error_code = NULL) = 0;    

    // @brief:      �ƶ��ļ�����Ŀ¼
    // @param:      src_name    Դ�ļ���Ŀ¼ȫ·����Ŀ¼����/��β
    // @param:      dest_name   Ŀ��Ŀ¼ȫ·����Ŀ¼����/��β
    // @param:      error_code  ���������Ϣ
    // @return:     0 �ɹ�
    //              -1 ʧ��
    // @see:        Move
    virtual int32_t MoveImpl(const char* src_name, 
                        const char* dst_name, uint32_t* error_code) = 0;


    // @brief:      �޸��ļ�����Ŀ¼����
    // @param:      old_path_name   �ɵ��ļ���Ŀ¼ȫ·����,Ŀ¼����/��β
    // @param:      new_path_name   �µ��ļ���Ŀ¼����Ŀ¼����/��β
    // @param:      error_code      ���������Ϣ
    // @return:     0 �ɹ�
    //              -1 ʧ��    
    // @see:        Rename
    virtual int32_t  RenameImpl(const char* old_path_name,
                           const char* new_path_name,
                           uint32_t*   error_code = NULL) = 0;

    // @brief:      ɾ���ļ�����Ŀ¼
    // @param:      path_name       ��ɾ�����ļ���Ŀ¼ȫ·����,Ŀ¼����/��β
    // @param:      is_recursive    �Ƿ�ݹ�ɾ�������ļ�����Ŀ¼.
    //                              �����ļ�ʵ�ֲ�֧��.
    // @param:      error_code      ���������Ϣ
    // @return:     0 �ɹ�
    //              -1 ʧ��    
    // @see:        Remove
    virtual int32_t  RemoveImpl(const char* path_name,
                           bool        is_recursive = false,
                           uint32_t*   error_code = NULL) = 0;

    // @brief:      �����Ŀ¼
    // @param:      path_name   Ŀ¼ȫ·����,Ŀ¼����/��β
    // @param:      error_code  ���������Ϣ
    // @return:     0 �ɹ�
    //              -1 ʧ��    
    // @see:        AddDir
    virtual int32_t  AddDirImpl(const char* path_name, 
                            uint32_t* error_code = NULL) = 0;


    // @brief:      �鿴ָ��Ŀ¼���ļ���������Ϣ
    // @param:      pattern     Ҫ�鿴��Ŀ¼���ļ���Ŀ¼��/��β��֧��* ? []
    // @param:      mask        ��Ҫ��ȡ��������Ϣ����. 
    // @param:      buffer      �ŷ��ص�������Ϣ,Ӧ�ò����Ļ�����
    // @param:      error_code  ���������Ϣ
    // @return:     0 �ɹ�; -1 ʧ��
    // @see:        List
    virtual int32_t ListImpl(const char* pattern,
                        AttrsMask* mask,
                        std::vector<AttrsInfo>* buffer,
                        uint32_t* error_code = NULL) = 0;

    // @brief:      �鿴�ļ�����Ŀ¼ռ�õĿռ�
    // @param:      path_name   �鿴���ļ���Ŀ¼ȫ·������Ŀ¼����/��β
    // @param:      error_code  ���������Ϣ
    // @return:     >=0 �ɹ�(�����ļ�����Ŀ¼��С)
    //              -1 ʧ��
    // @see:        Du
    virtual int64_t  DuImpl(const char* path_name,
                        uint32_t* error_code = NULL) = 0;


    // @brief:      ����ļ���С
    // @param:      file_name   Ŀ¼ȫ·����
    // @param:      error_code  ���������Ϣ
    // @return:     >=0 �ɹ�
    //              -1 ʧ��
    // @see:        GetSize
    virtual int64_t  GetSizeImpl(const char* file_name,
                        uint32_t*   error_code = NULL) = 0;

    // @brief:      �鿴ָ���ļ�����Ŀ¼�Ƿ����
    // @param:      path_name    Ҫ�鿴���ļ���ȫ·������Ŀ¼��/��β
    // @param:      error_code   ���������Ϣ
    // @return:     true �ɹ�(����)
    //              false ʧ��
    // @see:        CheckExist
    virtual bool CheckExistImpl(const char* path_name, 
                        uint32_t* error_code = NULL) = 0;

    // @brief:      �޸��ļ���Ŀ¼Ȩ��
    // @param:      path_name    Ҫ�޸ĵ��ļ���ȫ·����
    // @param:      permission   Ŀ��Ȩ��
    // @param:      error_code   ���������Ϣ
    // @return:     true �ɹ�(����)
    //              false ʧ��
    virtual bool ChmodImpl(const char* path_name,
                           const uint32_t permission,
                           uint32_t* error_code = NULL) = 0;

    // @brief:      �޸��ļ���Ŀ¼��role,ע��ֻ��xfs_admin�����޸�xfs���ļ���Role
    // @param:      path_name     Ҫ�޸ĵ��ļ���ȫ·����
    // @param:      role_name     ��xfs�ļ�,�˲���Ϊһ��role����;
    //                            ��local�ļ�, �˲���Ϊuser_name:group_name
    // @param:      error_code    ���������Ϣ
    // @return:     true �ɹ�(����)
    //              false ʧ��
    virtual bool ChangeRoleImpl(const char* path_name,
                                const char* role_name,
                                uint32_t* error_code = NULL) = 0;

    // A helper functions to set error code when necessary.
    static void SetErrorCode(uint32_t* error_code, FILE_ERROR_CODE code) {
        if(error_code) {
            *error_code = code;
        }
    }

    // ���캯�� & ��������
    File() {}
};

// Following Macros are for registering and creating file implementations.

CLASS_REGISTER_DEFINE_REGISTRY(file_impl_register, File);

#define REGISTER_FILE_IMPL(path_prefix_as_string, file_impl_name) \
    CLASS_REGISTER_OBJECT_CREATOR_WITH_SINGLETON( \
        file_impl_register, File, path_prefix_as_string, file_impl_name) 

#define CREATE_FILE_IMPL(path_prefix_as_string) \
    CLASS_REGISTER_CREATE_OBJECT(file_impl_register, path_prefix_as_string)

#define GET_FILE_IMPL_SINGLETON(path_prefix_as_string) \
    CLASS_REGISTER_GET_SINGLETON(file_impl_register, path_prefix_as_string)

#define FILE_IMPL_COUNT() \
    CLASS_REGISTER_CREATOR_COUNT(file_impl_register)

#define FILE_IMPL_NAME(i) \
    CLASS_REGISTER_CREATOR_NAME(file_impl_register, i)

#endif//COMMON_FILE_FILE_H_
