// Copyright 2010, Tencent Inc.
//
// 作者: typherque
// 日期: 2010.04.14
// 版本: 1.1
// 模块描述: 文件系统SDK接口,支持XFS, LocalFile...
//
// 修改历史:
// <author>    <time>    <version >   <desc>    <build number>    <last update>
// typherque  2010.04.14    0.1        创建     001
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

// 面向用户的文件信息结构
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

    // 数据占总文件的百分比
    float           size_percent;

    DataLocation() {
        memset(this, 0, sizeof(*this));
    }
};


const char* GetFileTypeDesc(ENUM_FILE_TYPE type);

//
// FILE Public Error Code
// File对外错误码
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

    // @brief:       传给Open中的flag参数
    enum FILE_FLAG {
        ENUM_FILE_OPEN_MODE_R     = 0x01,
        ENUM_FILE_OPEN_MODE_W     = 0x02,
        ENUM_FILE_OPEN_MODE_A     = 0x04,
        ENUM_FILE_TYPE_RA_FILE    = 0x200,    ///< 不设置则是普通文件
        ENUM_FILE_IO_NON_BLOCKING = 0X1000,   ///< 不设置则是同步,设置则是异步
    };

    // @brief:      初始化SDK模块,在进程中初始化调用一次,
    //              不需要每个File对象都调用
    //
    // @param:      identity,验证用户的身份,进行文件访问的时候会使用用户的某一个role,
    //              ,默认为和用户名同名的一个role. 
    //              使用不同的role? 参见class Certifier的使用.
    //
    // @return:     true, 成功
    //              false,失败
    static bool     Init(const char* identity = NULL);

    // @brief:      打开一个文件
    // @param:      file_path   要打开的文件全路径,可以是本地文件或/xfs/网络文件
    //                          可以下述形式附加选项.
    //                          file_path:k1=v1:k2=v2
    //                          选项具体由文件实现解释.
    //                          当附加的选项与传入的options重复时，以此处为准.
    // @param:      flags       要打开文件的模式,包括打开普通文件或者记录型文件,
    //                          读写权限，异步或同步读写
    // @param:      options     选项，包括是否使用客户端cache，副本数量等
    // @param:      error_code  用于返回出错代码
    // @return:     表示分配的文件指针; NULL 表示出错
    //
    // @note:       flags各个模式的意义：
    // W:           如果不存在会建立文件;如果文件存在，都会把之前文件删除，
    //              建立一个新的文件;文件指针在文件头;
    //
    // A:           如果文件存在,在文件尾追加;若不存在,则建立一个文件,
    //              文件读写指针在最后;
    //
    // R:           只能打开已存在的文件,文件指针在文件头;
    // R|W:         文件不存在时,会新建文件,文件指针在头部;文件存在时，
    //              若写文件，则会自动移动到文件尾;
    //
    // W|A:         不允许;
    // R|A:         与RW相同;
    // R|W|A:       与RW相同;
    static File*    Open(const char*   file_path,
                         uint32_t      flags,
                         const OpenFileOptions& options = OpenFileOptions(),
                         uint32_t* error_code = NULL); 

    // @brief:      打开一个文件另一接口, 不处理错误，要不成功
    //              要不程序死掉（可根据需要选用）
    // @param:      file_path   要打开的文件全路径,可以是本地文件或/xfs/网络文件
    // @param:      flags       要打开文件的模式,包括打开普通文件或者记录型文件,
    //                          读写权限，异步或同步读写
    // @param:      options     读写文件的选项
    // @return:     表示分配的文件指针; NULL 表示出错
    //
    // @note:       flags各个模式的意义：
    // W:           如果不存在会建立文件;如果文件存在，都会把之前文件删除，
    //              建立一个新的文件;文件指针在文件头;
    //
    // A:           如果文件存在,在文件尾追加;若不存在,则建立一个文件,
    //              文件读写指针在最后;
    //
    // R:           只能打开已存在的文件,文件指针在文件头;
    // R|W:         文件不存在时,会新建文件,文件指针在头部;文件存在时，
    //              若写文件，则会自动移动到文件尾;
    //
    // W|A:         不允许;
    // R|A:         与RW相同;
    // R|W|A:       与RW相同;
    static File*    OpenOrDie(const char* file_path, uint32_t flags, 
        const OpenFileOptions& options = OpenFileOptions());

    // @brief:      同步从文件读取数据
    // @param:      buffer      表示存放数据的本地内存空间；
    // @param:      size        表示需要读取的最大数据长度；
    // @param:      error_code  存放错误信息；
    // @return:     -1 出错
    //              0  读到了文件结尾
    //              >0 实际读取到的文件长度
    virtual int64_t Read(void*     buffer,
                         int64_t   size,
                         uint32_t* error_code = NULL) = 0;

    // @brief:      同步方式向打开的文件写入数据，文件通常缓存在本地,
    //              调用Flush或Close时刷新到网络文件
    //
    // @param:      buffer      指向待写入的数据
    // @param:      size        待写入的文件长度
    // @param:      error_code  存放出错信息,
    //
    // @return:     >=0 成功(写入的长度);
    //              -1  失败
    virtual int64_t Write(const void* buffer,
                          int64_t     size,
                          uint32_t*   error_code = NULL) = 0;

    // @brief:      从文件的指定位置异步读取数据
    // @param:      buffer          表示存放数据的本地内存空间；
    // @param:      size            表示需要读取的最大数据长度；
    // @param:      start_position  表示从该位置开始读取数据；
    // @param:      callback        与当前数据相对应的回调函数
    //                              (void:表示回调函数的返回值,后两个是参数:
    //                              第一个表示成功传输的长度;
    //                              第二个为errorcode)
    //
    // @param:      timeout         超时时间,seconds
    // @param:      error_code      保存错误信息
    // @return:     =0 成功
    //              -1 失败
    virtual int32_t AsyncReadFrom(void* buffer,
                                  int64_t size,
                                  int64_t start_position,
                                  Closure<void, int64_t, uint32_t>* callback,
                                  uint32_t  timeout = 60*60,
                                  uint32_t* error_code = NULL)=0;

    // @brief:      异步方式向打开的文件写入数据，文件通常缓存在本地,
    //              调用Flush或Close时刷新到网络文件
    //
    // @param:      buffer      指向待写入的数据
    // @param:      size        待写入的文件长度
    // @param:      callback    数据写入成功后该回调函数将被调用
    //                          (void:表示回调函数的返回值,后两个是参数:
    //                          第一个表示成功传输的长度;
    //                          第二个为errorcode)
    //
    // @param:      timeout     超时时间,seconds
    // @param:      error_code  存放出错信息,
    //
    // @return:     =0 成功(写入的长度)
    //              -1 失败
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

    // @brief:      将本地数据刷新至网络服务器或磁盘
    // @param:      error_code  存放出错信息
    // @return:     0  成功
    //              <0 失败
    virtual int32_t Flush(uint32_t* error_code = NULL) = 0;

    // @brief:      关闭一个文件
    // @param:      error_code  存放出错信息
    // @return:     0 成功; -1 失败
    //
    // @note:       关闭之前会主动调用Flush一次
    virtual int32_t Close(uint32_t* error_code = NULL) = 0;

    // @brief:      拷贝文件数据,暂不支持含通配符的大量拷贝操作
    // @param:      src_file_path   目标源文件全路径
    // @param:      dest_file_path  目标文件全路径
    // @param:      error_code      保存出错信息
    // @return:     0 成功
    //              -1 失败
    static int32_t  Copy(const char*    src_file_path,
                         const char*    dest_file_path,
                         uint32_t*      error_code = NULL);

    // @brief:      移动文件或子目录
    // @param:      src_name    源文件或目录全路径，目录名以/结尾
    // @param:      dest_name   目标目录全路径，目录名以/结尾
    // @param:      error_code  保存出错信息
    // @return:     0 成功
    //              -1 失败
    static int32_t  Move(const char* src_name,
                         const char* dest_name,
                         uint32_t*   error_code = NULL);

    // @brief:      修改文件或子目录名字
    // @param:      old_path_name   旧的文件或目录全路径名,目录名以/结尾
    // @param:      new_path_name   新的文件或目录名，目录名以/结尾
    // @param:      error_code      保存出错信息
    // @return:     0 成功
    //              -1 失败
    static int32_t  Rename(const char* old_path_name,
                           const char* new_path_name,
                           uint32_t*   error_code = NULL);

    // @brief:      删除文件或子目录
    // @param:      path_name       待删除的文件或目录全路径名,目录名以/结尾
    // @param:      is_recursive    是否递归删除其子文件和子目录, 
    //                              部分文件实现不支持.
    // @param:      error_code      保存出错信息
    // @return:     0 成功
    //              -1 失败
    static int32_t  Remove(const char* path_name,
                           bool        is_recursive = false,
                           uint32_t*   error_code = NULL);

    // @brief:      添加子目录
    // @param:      path_name   目录全路径名,目录名以/结尾
    // @param:      error_code  保存出错信息
    // @return:     0 成功
    //              -1 失败
    static int32_t  AddDir(const char* path_name, uint32_t* error_code = NULL);



    // @brief:      查看指定目录或文件的属性信息
    // @param:      pattern     要查看的目录或文件，支持* ? [].
    // @param:      mask        想要获取的属性信息掩码. 
    //                          运行后被设置成文件实现实际支持的掩码.
    // @param:      buffer      放返回的属性信息,应用层分配的缓冲区.
    //                          某些字段可能由于获取信息失败而保留为其默认值.
    // @param:      error_code  保存出错信息
    // @return:     0 成功; -1 失败
    static int32_t List(const char*    pattern,
                        AttrsMask* mask,
                        std::vector<AttrsInfo>* buffer,
                        uint32_t* error_code = NULL);

    // @brief:      改变文件的当前偏移量
    // @param:      offset      相对于origin的偏移量
    // @param:      whence      偏移量的相对位置
    //              可选参数:
    //                  SEEK_SET    文件开始
    //                  SEEK_CUR    文件的当前位置
    //                  SEEK_END    文件结尾
    // @param:      error_code  返回出错信息
    // @return:     >=0 成功(返回当前文件的偏移量)
    //              1 出错
    virtual int64_t Seek(int64_t   offset,
                         int32_t   whence,
                         uint32_t* error_code = NULL) = 0;

    // @brief:      查看文件或子目录占用的空间
    // @param:      path_name   查看的文件或目录全路径名，目录须以/结尾
    // @param:      error_code  保存出错信息
    // @return:     >=0 成功(返回文件或子目录大小)
    //              -1 失败
    static int64_t  Du(const char* path_name, uint32_t* error_code = NULL);

    // @brief:      返回当前文件的偏移量
    // @param:      error_code   保存出错信息
    // @return:     >=0 成功(返回当前文件的偏移量);-1 出错
    virtual int64_t Tell(uint32_t* error_code = NULL) = 0;

    // @brief:      设置文件有效大小（截短）
    // @param:      length      文件有效大小
    // @param:      error_code  保存出错信息
    // @return:     >=0 成功
    //              -1 出错
    virtual int32_t Truncate(uint64_t length, uint32_t* error_code = NULL) = 0;


    // @brief:      获取数据所在的NodeServe位置
    // @param:      start       数据块的开始位置
    // @param:      end         数据块的结束位置
    // @param:      buffer      获取到的位置信息
    // @param:      error_code  保存出错代码
    // @return:     0 成功
    //              -1 失败
    virtual int32_t LocateData(uint64_t      start,
                               uint64_t      end,
                               std::vector<DataLocation>* buffer,
                               uint32_t*     error_code = NULL) = 0;

    // @brief:      获得文件大小
    // @param:      file_name   目录全路径名
    // @param:      error_code  保存出错信息
    // @return:     >=0 成功
    //              -1 失败
    static int64_t  GetSize(const char* file_name,
                            uint32_t*   error_code = NULL);

    // @brief:      获取指定目录下的文件序列
    // @param:      pattern      要查看的目录或文件，目录以/结尾
    // @param:      files        放返回的所有文件序列（不分页）. 
    // @return:     0 成功; -1 失败
    static int32_t GetMatchingFiles(const char*     pattern,
                                    std::vector<std::string>* files);

    // @brief:      查看指定文件或者目录是否存在
    // @param:      path         要查看的文件名全路径名，目录以/结尾
    // @param:      error_code   保存出错信息
    // @return:     true 成功(存在)
    //              false 失败
    static bool IsExist(const char* path_name, uint32_t* error_code = NULL);

    // @brief:      查看指定文件或者目录是否存在
    // @param:      path         要查看的文件名全路径名，目录以/结尾
    // @param:      error_code   保存出错信息
    // @return:     true 成功(存在)
    //              false 失败
    // DEPRECATED_BY(IsExist)
    static bool CheckExist(const char* path_name, uint32_t* error_code = NULL);

    // @brief:      修改文件或目录权限
    // @param:      path_name    要修改的文件名全路径名
    // @param:      permission   目标权限
    // @param:      error_code   保存出错信息
    // @return:     true 成功(存在)
    //              false 失败
    static bool Chmod(const char* path_name,
                      const uint32_t permission,
                      uint32_t* error_code = NULL);

    // @brief:      修改文件或目录的role,注意只有xfs_admin才能修改xfs上文件的Role
    // @param:      path_name     要修改的文件名全路径名
    // @param:      role_name     xfs文件,此参数权限赋予的role;
    //                            local文件, 此参数为user_name:group_name
    // @param:      error_code    保存出错信息
    // @return:     true 成功(存在)
    //              false 失败
    static bool ChangeRole(const char* path_name,
                           const char* role_name,
                           uint32_t* error_code = NULL);

    // @brief:      清理SDK模块
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
    // @brief：     Create a File object for the file implementation.
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
    
    // @brief:      初始化SDK模块,在进程中初始化调用一次,
    //              不需要每个File对象都调用
    //
    // @param:      identity   创建文件的用户名(rtx name)，默认为当前用户
    // @return:     true, 成功
    //              false,失败
    // @see:        Init
    virtual bool InitImpl(const char* identity = NULL) = 0;

    // @brief:      清理SDK模块
    // @return:     void
    // @see:        Cleanup 
    virtual bool CleanupImpl() = 0;

    //
    // @brief:      打开一个文件
    // @param:      file_path   要打开的文件全路径,可以是本地文件或/xfs/网络文件
    //                          可以下述形式附加选项.
    //                          file_path:k1=v1:k2=v2
    //                          选项具体由文件实现解释.
    //                          当附加的选项与传入的options重复时，以此处为准.
    // @param:      flags       要打开文件的模式,包括打开普通文件或者记录型文件,
    //                          读写权限，异步或同步读写
    // @param:      options     选项，包括是否使用客户端cache，副本数量等
    // @param:      error_code  用于返回出错代码
    // @return:     表示分配的文件指针; NULL 表示出错
    //
    // @note:       flags各个模式的意义：
    // W:           如果不存在会建立文件;如果文件存在，都会把之前文件删除，
    //              建立一个新的文件;文件指针在文件头;
    //
    // A:           如果文件存在,在文件尾追加;若不存在,则建立一个文件,
    //              文件读写指针在最后;
    //
    // R:           只能打开已存在的文件,文件指针在文件头;
    // R|W:         文件不存在时,会新建文件,文件指针在头部;文件存在时，
    //              若写文件，则会自动移动到文件尾;
    //
    // W|A:         不允许;
    // R|A:         与RW相同;
    // R|W|A:       与RW相同;
    // @see:        Open
    virtual bool OpenImpl(const char *file_path, uint32_t flags,
                        const OpenFileOptions& options = OpenFileOptions(),
                        uint32_t *error_code = NULL) = 0;    

    // @brief:      移动文件或子目录
    // @param:      src_name    源文件或目录全路径，目录名以/结尾
    // @param:      dest_name   目标目录全路径，目录名以/结尾
    // @param:      error_code  保存出错信息
    // @return:     0 成功
    //              -1 失败
    // @see:        Move
    virtual int32_t MoveImpl(const char* src_name, 
                        const char* dst_name, uint32_t* error_code) = 0;


    // @brief:      修改文件或子目录名字
    // @param:      old_path_name   旧的文件或目录全路径名,目录名以/结尾
    // @param:      new_path_name   新的文件或目录名，目录名以/结尾
    // @param:      error_code      保存出错信息
    // @return:     0 成功
    //              -1 失败    
    // @see:        Rename
    virtual int32_t  RenameImpl(const char* old_path_name,
                           const char* new_path_name,
                           uint32_t*   error_code = NULL) = 0;

    // @brief:      删除文件或子目录
    // @param:      path_name       待删除的文件或目录全路径名,目录名以/结尾
    // @param:      is_recursive    是否递归删除其子文件和子目录.
    //                              部分文件实现不支持.
    // @param:      error_code      保存出错信息
    // @return:     0 成功
    //              -1 失败    
    // @see:        Remove
    virtual int32_t  RemoveImpl(const char* path_name,
                           bool        is_recursive = false,
                           uint32_t*   error_code = NULL) = 0;

    // @brief:      添加子目录
    // @param:      path_name   目录全路径名,目录名以/结尾
    // @param:      error_code  保存出错信息
    // @return:     0 成功
    //              -1 失败    
    // @see:        AddDir
    virtual int32_t  AddDirImpl(const char* path_name, 
                            uint32_t* error_code = NULL) = 0;


    // @brief:      查看指定目录或文件的属性信息
    // @param:      pattern     要查看的目录或文件，目录以/结尾，支持* ? []
    // @param:      mask        想要获取的属性信息掩码. 
    // @param:      buffer      放返回的属性信息,应用层分配的缓冲区
    // @param:      error_code  保存出错信息
    // @return:     0 成功; -1 失败
    // @see:        List
    virtual int32_t ListImpl(const char* pattern,
                        AttrsMask* mask,
                        std::vector<AttrsInfo>* buffer,
                        uint32_t* error_code = NULL) = 0;

    // @brief:      查看文件或子目录占用的空间
    // @param:      path_name   查看的文件或目录全路径名，目录须以/结尾
    // @param:      error_code  保存出错信息
    // @return:     >=0 成功(返回文件或子目录大小)
    //              -1 失败
    // @see:        Du
    virtual int64_t  DuImpl(const char* path_name,
                        uint32_t* error_code = NULL) = 0;


    // @brief:      获得文件大小
    // @param:      file_name   目录全路径名
    // @param:      error_code  保存出错信息
    // @return:     >=0 成功
    //              -1 失败
    // @see:        GetSize
    virtual int64_t  GetSizeImpl(const char* file_name,
                        uint32_t*   error_code = NULL) = 0;

    // @brief:      查看指定文件或者目录是否存在
    // @param:      path_name    要查看的文件名全路径名，目录以/结尾
    // @param:      error_code   保存出错信息
    // @return:     true 成功(存在)
    //              false 失败
    // @see:        CheckExist
    virtual bool CheckExistImpl(const char* path_name, 
                        uint32_t* error_code = NULL) = 0;

    // @brief:      修改文件或目录权限
    // @param:      path_name    要修改的文件名全路径名
    // @param:      permission   目标权限
    // @param:      error_code   保存出错信息
    // @return:     true 成功(存在)
    //              false 失败
    virtual bool ChmodImpl(const char* path_name,
                           const uint32_t permission,
                           uint32_t* error_code = NULL) = 0;

    // @brief:      修改文件或目录的role,注意只有xfs_admin才能修改xfs上文件的Role
    // @param:      path_name     要修改的文件名全路径名
    // @param:      role_name     对xfs文件,此参数为一个role名称;
    //                            对local文件, 此参数为user_name:group_name
    // @param:      error_code    保存出错信息
    // @return:     true 成功(存在)
    //              false 失败
    virtual bool ChangeRoleImpl(const char* path_name,
                                const char* role_name,
                                uint32_t* error_code = NULL) = 0;

    // A helper functions to set error code when necessary.
    static void SetErrorCode(uint32_t* error_code, FILE_ERROR_CODE code) {
        if(error_code) {
            *error_code = code;
        }
    }

    // 构造函数 & 析构函数
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
