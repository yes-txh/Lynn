// Copyright 2010, Tencent Inc.
//
// wookin
// aaronzou, 2011.01.10
//            Refactor to use polymorphism to forward to the right subclass.

#if !defined(COMMON_FILE_LOCALFILE_H_)
#define COMMON_FILE_LOCALFILE_H_

#include "common/file/file.h"

// Just for test.
#define LOCAL_FILE_PREFIX    "/local/"

// 本地异步读写的线程
// extern LocalFileThread*         g_local_thread;

class LocalFile : public File {
public:
    // Let public methods be virtual, so that the subclass could extends these when necessary.

    LocalFile();

    virtual ~LocalFile();

    virtual std::string GetFileImplName() {
        return LOCAL_FILE_PREFIX;
    }

    // @brief:      关闭一个文件
    // @param:      error_code  存放出错信息
    // @return:     0 成功; -1 失败
    //
    // @note:       关闭之前会主动调用Flush一次
    virtual int32_t Close(uint32_t* error_code=NULL);

    // @brief:      将本地数据刷新至网络服务器或磁盘
    // @param:      error_code  存放出错信息
    // @return:     0  成功
    //              <0 失败
    virtual int32_t Flush(uint32_t* error_code=NULL);

    // @brief:      同步方式向打开的文件写入数据，文件通常缓存在本地,
    //              调用Flush或Close时刷新到网络文件
    //
    // @param:      buffer      指向待写入的数据
    // @param:      size        待写入的文件长度
    // @param:      error_code  存放出错信息,
    //
    // @return:     >=0 成功(写入的长度);
    //              -1  失败
    virtual int64_t Write(const void* buf, int64_t buf_size,
                          uint32_t* error_code = NULL);

    // @brief:      同步从文件读取数据
    // @param:      buffer      表示存放数据的本地内存空间；
    // @param:      size        表示需要读取的最大数据长度；
    // @param:      error_code  存放错误信息；
    // @return:     -1 出错
    //              0  读到了文件结尾
    //              >0 实际读取到的文件长度
    virtual int64_t Read(void* buf, int64_t buf_size,
                         uint32_t* error_code=NULL);

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
    virtual int32_t AsyncWrite(const void* buf, int64_t buf_size,
                               Closure<void, int64_t, uint32_t>* callback,
                               uint32_t time_out = 60*60,
                               uint32_t* error_code = NULL);


    // @brief:       从文件的指定位置异步读取数据
    // @param:       buffer         表示存放数据的本地内存空间；
    // @param:       size           表示需要读取的最大数据长度；
    // @param:       start_position 表示从该位置开始读取数据；
    // @param:       callback       与当前数据相对应的回调函数
    //                              (void:表示回调函数的返回值,后两个是参数:
    //                              第一个表示成功传输的长度;
    //                              第二个为errorcode)
    //
    // @param:       timeout        超时时间,seconds
    // @param:       error_code     保存错误信息
    // @return:      =0 成功
    //               -1 失败
    virtual int32_t AsyncReadFrom(void* buffer,
                                  int64_t size,
                                  int64_t start_position,
                                  Closure<void, int64_t, uint32_t>* callback,
                                  uint32_t  timeout = 60*60,
                                  uint32_t* error_code = NULL);

    // @brief:      The file implementation support asynchronous operations?
    //              For file implementation doesn't support aync operaton,
    //                  the behavior of calling async operation is undefined.
    // @return:     return true if support async operations.
    virtual bool SupportAsync() {
        return true;
    }


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
    virtual int64_t Seek(int64_t offset,int32_t origin,uint32_t* error_code=NULL);



    // @brief:      返回当前文件的偏移量
    // @param:      error_code   保存出错信息
    // @return:     >=0 成功(返回当前文件的偏移量);-1 出错
    virtual int64_t Tell(uint32_t* error_code=NULL);


    // @brief:      设置文件有效大小（截短）
    // @param:      length      文件有效大小
    // @param:      error_code  保存出错信息
    // @return:     >=0 成功
    //              -1 出错
    virtual int32_t Truncate(uint64_t length, uint32_t* error_code=NULL);

    // @brief:      获取数据所在的NodeServe位置
    // @param:      start       数据块的开始位置
    // @param:      end         数据块的结束位置
    // @param:      buffer      获取到的位置信息
    // @param:      error_code  保存出错代码
    // @return:     0 成功
    //              -1 失败
    virtual int32_t LocateData( uint64_t start_pos, uint64_t end_pos,
                                std::vector<DataLocation>* buf,
                                uint32_t* error_code = NULL);

protected:
    // @brief:      初始化SDK模块,在进程中初始化调用一次,
    //              不需要每个File对象都调用
    //
    // @param:      user_name   创建文件的用户名，默认为当前用户
    //
    // @return:     true, 成功
    //              false,失败
    // @see:        Init
    virtual bool InitImpl(const char* user_name);

    // @brief:      清理SDK模块
    // @return:     void
    // @see:        Cleanup
    virtual bool CleanupImpl();

    //
    // @brief:      打开一个文件
    // @param:      file_path   要打开的文件全路径,可以是本地文件或/xfs/网络文件
    //                          可以下述形式附加选项.
    //                          file_path:k1=v1:k2=v2.
    //                          目前的LocalFile实现不支持额外的选项.
    // @param:      flags       要打开文件的模式,包括打开普通文件或者记录型文件,
    //                          读写权限，异步或同步读写
    // @param:      options     选项，包括是否使用客户端cache，副本数量等
    //                          目前的LocalFile实现忽略选项.
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
    virtual bool OpenImpl(const char* file_path, uint32_t flags,
                  const OpenFileOptions& options = OpenFileOptions(),
                  uint32_t *error_code = NULL);


    // @brief:      查看指定目录或文件的属性信息
    // @param:      pattern     要查看的目录或文件，目录以/结尾，支持* ? []
    // @param:      mask        想要获取的属性信息掩码.
    //                          运行后被设置成文件实现实际支持的掩码.
    // @param:      buffer      放返回的属性信息,应用层分配的缓冲区
    //                          某些字段可能由于获取信息失败而保留为其默认值.
    // @param:      error_code  保存出错信息
    // @return:     0 成功; -1 失败
    // @see:        List
    virtual int32_t ListImpl(const char* pattern,
                        AttrsMask* mask, std::vector<AttrsInfo>* buffer,
                        uint32_t* error_code=NULL);

    // @brief:      查看指定文件或者目录是否存在
    // @param:      file_name   要查看的文件名全路径名，目录以/结尾
    // @param:      error_code  保存出错信息
    // @return:     true 成功(存在)
    //              false 失败
    // @see:        CheckExist
    virtual bool CheckExistImpl(const char* file_name,
                           uint32_t* error_code = NULL);

    // @brief:      查看文件或子目录占用的空间
    // @param:      path        查看的文件或目录全路径名，目录须以/结尾
    // @param:      error_code  保存出错信息
    // @return:     >=0 成功(返回文件或子目录大小)
    //              -1 失败
    // @see:        Du
    virtual int64_t  DuImpl(const char* path_name, uint32_t* error_code = NULL);

    // @brief:      移动文件或子目录
    // @param:      src_name    源文件或目录全路径，目录名以/结尾
    // @param:      dest_name   目标目录全路径，目录名以/结尾
    // @param:      error_code  保存出错信息
    // @return:     0 成功
    //              -1 失败
    // @see:        Move
    virtual int32_t MoveImpl(const char* src_name, const char* dst_name, uint32_t* error_code);


    // @brief:      修改文件或子目录名字
    // @param:      old_path_name   旧的文件或目录全路径名,目录名以/结尾
    // @param:      new_path_name   新的文件或目录名，目录名以/结尾
    // @param:      error_code      保存出错信息
    // @return:     0 成功
    //              -1 失败
    // @see:        Rename
    virtual int32_t  RenameImpl(const char* old_path_name,
                           const char* new_path_name,
                           uint32_t*   error_code = NULL);

    // @brief:      删除文件或子目录
    // @param:      path_name       待删除的文件或目录全路径名,目录名以/结尾
    // @param:      is_recursive    是否递归删除其子文件和子目录, 部分文件实现不支持.
    // @param:      error_code      保存出错信息
    // @return:     0 成功
    //              -1 失败
    // @see:        Remove
    virtual int32_t RemoveImpl(const char* file_name,bool is_recursive=false,
                          uint32_t* error_code=NULL);


    // @brief:      添加子目录
    // @param:      path_name   目录全路径名,目录名以/结尾
    // @param:      error_code  保存出错信息
    // @return:     0 成功
    //              -1 失败
    // @see:        AddDir
    virtual int32_t AddDirImpl(const char* file_name, uint32_t* error_code=NULL);

    // @brief:      获得文件大小
    // @param:      file_name   目录全路径名
    // @param:      error_code  保存出错信息
    // @return:     >=0 成功
    //              -1 失败
    virtual int64_t GetSizeImpl(const char* file_name, uint32_t* error_code=NULL);

    // @brief:      修改文件或目录权限
    // @param:      path_name    要查看的文件名全路径名
    // @param:      permission   目标权限
    // @param:      error_code   保存出错信息
    // @return:     true 成功(存在)
    //              false 失败
    virtual bool ChmodImpl(const char* path_name,
                           const uint32_t permission,
                           uint32_t* error_code = NULL);

    // @brief:      修改文件或目录的owner
    // @param:      path_name     要修改的文件全路径名
    // @param:      role_name     user_name:group_name的组合
    // @param:      error_code    保存出错信息
    // @return:     true 成功(存在)
    //              false 失败
    virtual bool ChangeRoleImpl(const char* path_name,
                                const char* role_name,
                                uint32_t* error_code = NULL);

private:
    int32_t                 m_fd;

    bool CheckOpenModeValid(const uint32_t flags);

    // Build the full path for file_name in file_path.
    static std::string ConnectPathComponent(const char* file_path, const char* file_name);

    // Normalize a given file path and append seperator when necessary.
    static void NormalizePath(std::string& file_path);

    // actually write file in sync way to help AsyncWrite.
    void DoAsyncWrite(
        const void* buf,
        int64_t buf_size,
        Closure<void, int64_t, uint32_t>* callback,
        uint32_t time_out = 60*60);

    // actually read file in sync way to help AsyncReadFrom.
    void DoAsyncReadFrom(
        void* buffer,
        int64_t size,
        int64_t start_position,
        Closure<void, int64_t, uint32_t>* callback,
        uint32_t  timeout = 60*60);

    void AsyncWriteAIOCallback(
        Closure<void, int64_t, uint32_t>* out_callback,
        int64_t size,
        uint32_t status_code);

    void AsyncReadFromAIOCallback(
        Closure<void, int64_t, uint32_t>* out_callback,
        int64_t size,
        uint32_t status_code);

    // return 0 for valid path without wildcard
    // return 1 for valid path with wildcard in last path component
    // return -1 for invalide file path has wildcard in non-last path component
    int32_t ValidatePathWildcard(const char* file_path);
};

#endif//COMMON_FILE_LOCALFILE_H_

