//
// Created by wookin.
// Modified by typherque.
// Refactored by aaronzou.
//

// LocalFile.cc
// author: aaronzou, 2011.01.05 Refactor to use polymorphism to forward to the right subclass.

#ifdef WIN32
#include <io.h>
#include <windows.h>
#else

#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include <dirent.h>
#include <libaio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#endif

#include <fcntl.h>
#include <stdio.h>

#include "common/base/closure.h"
#include "common/file/local_file.h"
#include "common/file/aioframe/aioframe.h"
#include "common/base/string/string_algorithm.hpp"
#include "common/system/concurrency/thread_pool.hpp"
#include "common/system/concurrency/mutex.hpp"
#include "common/text/wildcard.hpp"

#include "thirdparty/glog/logging.h"

//Register LocalFile when this module is load and all global varibles are initialed.
REGISTER_FILE_IMPL(LOCAL_FILE_PREFIX, LocalFile);


#define CHECK_HANDLE(h) if(h == -1) { SetErrorCode(error_code, ERR_FILE_FAIL); return -1; }

// Define common functions for local file.
#ifdef WIN32

#define STRNCASECMP _strnicmp
#define WRITE       _write
#define OPEN        _open
#define READ        _read
#define CLOSE       _close
#define O_BINARY    _O_BINARY
#define ACCESS      _access
#define FTELL64(x)  _telli64((x))
#define LSEEKI64    _lseeki64

#else

#define STRNCASECMP strncasecmp
#define WRITE       write
#define OPEN        open
#define READ        read
#define CLOSE       close
#define O_BINARY    0
#define ACCESS      access
#define FTELL64(fhn) lseek(fhn, (off_t)0, SEEK_CUR)
#define LSEEKI64    lseek

#endif

namespace {
// local thread pool for async operations.
ThreadPool g_local_thread_pool;
// mutex to protect local file operations.
SimpleMutex g_file_mutex;

// linux only
#ifndef WIN32
aioframe::AIOFrame g_aioframe;
#endif
}

LocalFile::LocalFile() {
    m_fd=-1;
}

LocalFile::~LocalFile() {
    if(m_fd!=-1)
        CLOSE(m_fd);
}

std::string LocalFile::ConnectPathComponent(const char* file_path, const char* file_name) {
    std::string full_path(file_path);

    if (full_path.length() > 0
            && File::kPathSeparator != full_path[full_path.length() - 1]) {
        full_path += File::kPathSeparator;
    }

    full_path += file_name;

    return full_path;
}

void LocalFile::NormalizePath(std::string& full_path) {
    // TODO check '\\'
    if (full_path.length() > 0
            && File::kPathSeparator != full_path[full_path.length() - 1]) {
        full_path += File::kPathSeparator;
    }
}

bool LocalFile::InitImpl(const char* user_name) {
    //TODO(aaronzou): Not consider user_name and default_group_name now.
    return true;
}


bool LocalFile::CleanupImpl() {
    // terminate threads and wait to complete.
    g_local_thread_pool.Terminate(true);

    return true;
}

bool LocalFile::OpenImpl(const char* input_file_path, uint32_t flags,
    const OpenFileOptions& options,
    uint32_t* error_code) {
    // ignore options for local files.
    bool b=false;
    if(!input_file_path || !CheckOpenModeValid(flags)) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return b;
    }

    const char* file_path = input_file_path;

    int32_t open_flag=0;
    open_flag|=O_BINARY;
    bool is_exist = false;

    FILE* fp=fopen(file_path,"rb");
    if(fp) {
        // file exist
        fclose(fp);
        is_exist = true;
        fp=0;
    } else {
        is_exist = false;
        // file not exist
        open_flag|=((flags&ENUM_FILE_OPEN_MODE_W)==ENUM_FILE_OPEN_MODE_W
            || (flags&ENUM_FILE_OPEN_MODE_A)==ENUM_FILE_OPEN_MODE_A)?(O_CREAT):0;
    }

    if ((flags & ENUM_FILE_OPEN_MODE_A) == ENUM_FILE_OPEN_MODE_A) { // append, no write
        open_flag |= O_APPEND;
        if ((flags & ENUM_FILE_OPEN_MODE_R) == ENUM_FILE_OPEN_MODE_R) {
            open_flag |= O_RDWR; // which not equal to O_WRONLY | O_RDONLY
        } else {
            open_flag |= O_WRONLY;
        }
    } else if((flags&ENUM_FILE_OPEN_MODE_R)==ENUM_FILE_OPEN_MODE_R
        && (flags&ENUM_FILE_OPEN_MODE_W)==ENUM_FILE_OPEN_MODE_W) { // read, write
        open_flag|=O_RDWR;
    } else if((flags&ENUM_FILE_OPEN_MODE_R)==ENUM_FILE_OPEN_MODE_R
        && (flags&ENUM_FILE_OPEN_MODE_W)!=ENUM_FILE_OPEN_MODE_W) { // read
        open_flag|=O_RDONLY;
    } else if((flags&ENUM_FILE_OPEN_MODE_R)!=ENUM_FILE_OPEN_MODE_R
        && (flags&ENUM_FILE_OPEN_MODE_W)==ENUM_FILE_OPEN_MODE_W) { // write
        if((flags&ENUM_FILE_OPEN_MODE_A)!=ENUM_FILE_OPEN_MODE_A)
            open_flag|=(O_WRONLY|O_TRUNC);
        else
            open_flag|=O_WRONLY;
    }

    m_fd=OPEN(file_path,open_flag);
    if(m_fd==-1) {
        LOG(ERROR) << "Fail to open local file " << file_path << ". Reason: " << strerror(errno);
        SetErrorCode(error_code, ERR_FILE_FAIL);
    } else {
#ifdef WIN32
        SetFileAttributes(file_path, FILE_ATTRIBUTE_NORMAL);
#else
        if (!is_exist) {
            chmod(file_path, S_IRUSR | S_IWUSR);
        }
#endif
        b=true;
    }

    if (b) {
        SetErrorCode(error_code, ERR_FILE_OK);
    } else {
        SetErrorCode(error_code, ERR_FILE_FAIL);
    }
    return b;
}

int32_t LocalFile::Close(uint32_t* error_code) {
    CHECK_HANDLE(m_fd);

    if(CLOSE(m_fd)!=0) {
        SetErrorCode(error_code,ERR_FILE_FAIL);
        return -1;
    } else {
        SetErrorCode(error_code,ERR_FILE_OK);
        m_fd = -1;
        return 0;
    }
}

int32_t LocalFile::Flush(uint32_t* error_code) {
    CHECK_HANDLE(m_fd);

#ifndef WIN32
    fsync(m_fd);
#endif
    SetErrorCode(error_code,ERR_FILE_OK);
    return 0;
}


int64_t LocalFile::Write(const void* buf, int64_t buf_size, uint32_t* error_code) {
    CHECK_HANDLE(m_fd);
    if(!buf || buf_size <= 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }

#ifdef WIN32
    int64_t ret_size = WRITE(m_fd, buf, static_cast<uint32_t>(buf_size));
#else
    int64_t ret_size = WRITE(m_fd, buf, static_cast<size_t>(buf_size));
#endif

    SetErrorCode(error_code, ret_size == buf_size ? ERR_FILE_OK : ERR_FILE_FAIL);
    return ret_size > 0 ? ret_size : -1;
}

int64_t LocalFile::Read(void* buf, int64_t buf_size, uint32_t* error_code) {
    CHECK_HANDLE(m_fd);
    if(!buf || buf_size <= 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }

#ifdef WIN32
    int64_t read_bytes=READ(m_fd,buf,static_cast<uint32_t>(buf_size));
#else
    int64_t read_bytes=READ(m_fd,buf,static_cast<size_t>(buf_size));
#endif

    SetErrorCode(error_code, read_bytes >=0 ? ERR_FILE_OK : ERR_FILE_FAIL);
    return read_bytes >= 0 ? read_bytes : -1;
}


void LocalFile::AsyncWriteAIOCallback(
    Closure<void, int64_t, uint32_t>* out_callback,
    int64_t size,
    uint32_t status_code) {
    uint32_t error_code = ERR_FILE_OK;
    // the wanted write size must not 0. So writen 0 bytes is wrong.
    if (status_code != 0u || size == 0) {
        error_code = ERR_FILE_FAIL;
    }

    out_callback->Run(size, error_code);
}

int32_t LocalFile::AsyncWrite(const void* buf, int64_t buf_size,
    Closure<void, int64_t, uint32_t>* callback,
    uint32_t time_out, uint32_t* error_code) {
    CHECK_HANDLE(m_fd);
    if(!buf || buf_size <= 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }

#ifdef WIN32
    Closure<void>* task =
        NewClosure(
            this,
            &LocalFile::DoAsyncWrite,
            buf, buf_size, callback, time_out);
    g_local_thread_pool.AddTask(task);

    SetErrorCode(error_code, ERR_FILE_OK);
    return 0;
#else
    int64_t start_position = Tell();
    aioframe::StatusCode code;

    Closure<void, int64_t, uint32_t>* aio_callback =
        NewClosure(this, &LocalFile::AsyncWriteAIOCallback, callback);

    bool success = g_aioframe.AsyncWrite(m_fd, buf, buf_size, start_position, aio_callback, &code);
    if (success) {
        *error_code = ERR_FILE_OK;
    } else {
        *error_code = ERR_FILE_FAIL;
    }

    return success ? 0 : -1;
#endif
}


int64_t LocalFile::Seek(int64_t offset, int32_t origin, uint32_t* error_code) {
    CHECK_HANDLE(m_fd);
    int64_t ret = LSEEKI64(m_fd, offset, origin);
    SetErrorCode(error_code, ret != -1 ? ERR_FILE_OK : ERR_FILE_FAIL);
    return ret;
}

int64_t LocalFile::Tell(uint32_t* error_code) {
    CHECK_HANDLE(m_fd);
    int64_t ret = FTELL64(m_fd);
    SetErrorCode(error_code,ret!=-1?ERR_FILE_OK:ERR_FILE_FAIL);
    return ret;
}

int32_t LocalFile::Truncate( uint64_t length, uint32_t* error_code ) {
    CHECK_HANDLE(m_fd);
#ifdef WIN32
    if (0!=_chsize_s(m_fd, (int64_t)length)) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    } else {
        SetErrorCode(error_code, ERR_FILE_OK);
        return 0;
    }
#else
    if (0!=ftruncate(m_fd, (off_t)length)) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    } else {
        SetErrorCode(error_code, ERR_FILE_OK);
        return 0;
    }

#endif
}

int32_t LocalFile::ListImpl(const char* pattern,
                              AttrsMask* mask,
                              std::vector<AttrsInfo>* buffer,  uint32_t* error_code) {
    SetErrorCode(error_code, ERR_FILE_OK);
    if (!pattern || !mask || !buffer) {
        LOG(ERROR) << "parameter error";
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }


    const size_t len = strlen(pattern);
    if (len == 0u) {
        LOG(ERROR) << "parameter error, empty pattern";
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }

    int32_t path_type = ValidatePathWildcard(pattern);
    if (path_type < 0) {
        LOG(ERROR) << "parameter error, wrong pattern " << pattern;
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }

    std::string path_prefix(pattern);
    std::string name_pattern; // name part for wildcard
    if (path_type > 0) { // has wildcard, split the last path component
        const char* sep_pos = strrchr(pattern, '/');
        if (sep_pos == NULL) {
            // current directory, for pattern like '*.txt'
            name_pattern = path_prefix;
            path_prefix = ".";
        } else {
            name_pattern.assign(sep_pos + 1);
            path_prefix.assign(pattern, sep_pos);
        }
    }
    const char* name = path_prefix.c_str();

    // clear after check parameter
    buffer->clear();

    // Set mask to indicate LocalFile does not support these info.
    mask->modify_user = 0;
    mask->additional_info = 0;
    mask->file_id = 0;
    mask->backup_factor = 0;

#ifdef WIN32
    int32_t file_exist = _access(name,0);
    if(file_exist != 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        if (!error_code) LOG(ERROR) << "file " << name << " not exists";
        return -1;
    }

    // Use "*" instead of "*.*" because some file may have no file types.
    std::string full_file_name(pattern); // windows support wildcard
    if (full_file_name.length() > 0) {
        char lastCh = full_file_name[full_file_name.length() - 1];

        if (lastCh == File::kPathSeparator || lastCh == ':') { // ":" is for "C:"
            // for folders, to search files in folders.
            full_file_name = ConnectPathComponent(full_file_name.c_str(), "*");
        }
    }

    WIN32_FIND_DATA file_data;
    HANDLE find_handle = ::FindFirstFile(full_file_name.c_str(), &file_data);

    if(find_handle == INVALID_HANDLE_VALUE) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        if (!error_code) LOG(ERROR) << "file " << name << " not exists";
        return -1;
    }

    do {
        if(strcmp(file_data.cFileName,".") == 0 || strcmp(file_data.cFileName,"..") == 0 ) {
            continue;
        }

        AttrsInfo cur_attrs;

        cur_attrs.file_name = file_data.cFileName;
        if ((file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) { //win32 dir, add '\\'
            NormalizePath(cur_attrs.file_name);
        }
        if (mask->access_time) {
            FILETIME ft = file_data.ftLastAccessTime;
            LONGLONG  ll;
            ULARGE_INTEGER ui;
            ui.LowPart =  ft.dwLowDateTime;
            ui.HighPart = ft.dwHighDateTime;
            ll = (static_cast<uint64_t>(ft.dwHighDateTime) << 32)  +  ft.dwLowDateTime;
            time_t at =  ((LONGLONG)(ui.QuadPart  -  116444736000000000)  /  10000000);
            cur_attrs.access_time = static_cast<int32_t>(at);
        }
        if(mask->create_time) {
            FILETIME ft = file_data.ftCreationTime;
            LONGLONG  ll;
            ULARGE_INTEGER ui;
            ui.LowPart =  ft.dwLowDateTime;
            ui.HighPart = ft.dwHighDateTime;
            ll = (static_cast<uint64_t>(ft.dwHighDateTime) << 32)  +  ft.dwLowDateTime;
            time_t ct =  ((LONGLONG)(ui.QuadPart  -  116444736000000000)  /  10000000);
            cur_attrs.create_time = static_cast<int32_t>(ct);
        }
        if(mask->modify_time) {
            FILETIME ft = file_data.ftLastWriteTime;
            LONGLONG  ll;
            ULARGE_INTEGER ui;
            ui.LowPart =  ft.dwLowDateTime;
            ui.HighPart = ft.dwHighDateTime;
            ll = (static_cast<uint64_t>(ft.dwHighDateTime) << 32)  +  ft.dwLowDateTime;
            time_t mt =  ((LONGLONG)(ui.QuadPart  -  116444736000000000)  /  10000000);
            cur_attrs.modify_time = static_cast<int32_t>(mt);
        }

        if(mask->file_type) {
            ENUM_FILE_TYPE fileType = FILE_TYPE_NORMAL;
            if((file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
                fileType = FILE_TYPE_DIR;
            cur_attrs.file_type = fileType;
        }
        if(mask->file_size) {
            cur_attrs.file_size = file_data.nFileSizeHigh * 1024 * 1024 *1024 * 4 + file_data.nFileSizeLow;
        }

        buffer->push_back(cur_attrs);
    } while(::FindNextFile(find_handle, &file_data));

    ::FindClose(find_handle);
    return 0;

#else
// Linux
    int32_t file_exist = access(name, F_OK);
    if(file_exist != 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        if (!error_code) LOG(ERROR) << "file " << name << " not exists";
        return -1;
    }

    DIR * dir_ptr;
    struct dirent * dirent_ptr;
    struct stat stat_buf;

    if(stat(name, &stat_buf) < 0) {
        LOG(ERROR) << "stat error for " << name;
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }

    // directory
    if(S_ISDIR(stat_buf.st_mode)) {
        dir_ptr=opendir(name);
        while( (dirent_ptr=readdir(dir_ptr))!=NULL ) {
            if(strcmp(dirent_ptr->d_name,".") == 0 || strcmp(dirent_ptr->d_name,"..") == 0) {
                continue;
            }

            if (path_type > 0 && !Wildcard::Match(name_pattern.c_str(), dirent_ptr->d_name)) {
                continue; // ignore don't match files
            }

            std::string full_file_name = ConnectPathComponent(name, dirent_ptr->d_name);
            if(stat(full_file_name.c_str(), &stat_buf) < 0) {
                LOG(ERROR) << "stat error for " << full_file_name << ", may be deleted, ignore it";
                continue;
            }

            AttrsInfo cur_attrs;

            cur_attrs.file_name = dirent_ptr->d_name;

            if (mask->access_time) {
                cur_attrs.access_time = static_cast<uint32_t>(stat_buf.st_atime);
            }

            if(mask->create_time) {
                cur_attrs.create_time = static_cast<uint32_t>(stat_buf.st_ctime);
            }

            if(mask->modify_time) {
                cur_attrs.modify_time = static_cast<uint32_t>(stat_buf.st_mtime);
            }

            if(mask->file_type) {
                ENUM_FILE_TYPE file_type = FILE_TYPE_NORMAL;
                if(S_ISDIR(stat_buf.st_mode)) {
                    file_type = FILE_TYPE_DIR;
                }
                cur_attrs.file_type = file_type;
            }

            if(mask->file_size) {
                cur_attrs.file_size = static_cast<int64_t>(stat_buf.st_size);
            }

            buffer->push_back(cur_attrs);
            // should not recursively list
        }
        closedir(dir_ptr);
    } else {    // normal file.
        AttrsInfo cur_attrs;

        cur_attrs.file_name = name;
        size_t idx = cur_attrs.file_name.rfind('/');
        if (idx != std::string::npos) {
            cur_attrs.file_name = cur_attrs.file_name.substr(idx+1);
        }

        if (mask->access_time) {
            cur_attrs.access_time = static_cast<uint32_t>(stat_buf.st_atime);
        }
        if(mask->create_time) {
            cur_attrs.create_time = static_cast<uint32_t>(stat_buf.st_ctime);
        }
        if(mask->modify_time) {
            cur_attrs.modify_time = static_cast<uint32_t>(stat_buf.st_mtime);
        }
        if(mask->file_type) {
            cur_attrs.file_type = FILE_TYPE_NORMAL;
        }
        if(mask->file_size) {
            cur_attrs.file_size = static_cast<int64_t>(stat_buf.st_size);
        }

        buffer->push_back(cur_attrs);
    }
    return 0;
#endif
}

bool LocalFile::CheckExistImpl(const char* full_file_name, uint32_t* error_code) {
    const char* file_name = full_file_name;
    int32_t ret = ACCESS(file_name, 00);
    SetErrorCode(error_code,ret==-1?ERR_FILE_FAIL:ERR_FILE_OK);
    return (ret==0) ? true : false;
}


int64_t LocalFile::DuImpl(const char* file_name, uint32_t* error_code) {
    int64_t ret = -1;
#ifdef WIN32
    if(!file_name) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "file " << file_name << " not exists";
        return ret;
    }

    int32_t file_exist = ACCESS(file_name,0);
    if(file_exist != 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "file " << file_name << " not exists";
        return ret;
    }

    int64_t len = (int64_t)strlen(file_name);

    // Use "*" instead of "*.*" because some file may have no file types.
    std::string full_file_name(file_name);
    if (full_file_name.length() > 0) {
        char lastCh = full_file_name[full_file_name.length() - 1];

        if (lastCh == File::kPathSeparator || lastCh == ':') { // ":" is for "C:"
            // for folders, to search files in folders.
            full_file_name = ConnectPathComponent(full_file_name.c_str(), "*");
        }
    }

    WIN32_FIND_DATA file_data;
    HANDLE find_handle=::FindFirstFile(full_file_name.c_str(), &file_data);

    if(INVALID_HANDLE_VALUE == find_handle) {
        ret = 0;
        // Hidden files in Windows.
        SetErrorCode(error_code, ERR_FILE_OK);
        return ret;
    }

    ret = 0;
    do {
        if(strcmp(file_data.cFileName,".") == 0 || strcmp(file_data.cFileName,"..") == 0 )
            continue;
        bool is_dir = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        char* cur_file_name = file_data.cFileName;
        if(strcmp(cur_file_name ,".") == 0 || strcmp(cur_file_name,"..") == 0)
            continue;

        if(is_dir) {
            std::string new_file_name = ConnectPathComponent(file_name, cur_file_name);
            new_file_name += "/";
            ret += DuImpl(new_file_name.c_str(), error_code);
        } else {
            ret += file_data.nFileSizeHigh * 1024 * 1024 * 1024 * 4 + file_data.nFileSizeLow;
        }


    }
    while(FindNextFile(find_handle,&file_data));

    ::FindClose(find_handle);
    if (-1!=ret) {
        SetErrorCode(error_code, ERR_FILE_OK);
    } else {
        SetErrorCode(error_code, ERR_FILE_FAIL);
    }
    return ret;

#else
    int32_t file_exist = access(file_name,F_OK);
    if(file_exist != 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        if (!error_code) LOG(ERROR) << "file " << file_name << " not exists";
        return ret;
    }

    DIR * dir_ptr;
    struct dirent * dirent_ptr;
    struct stat stat_buf;

    if(stat(file_name, &stat_buf) < 0) {
        ret = -1;
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "stat error for " << file_name;
        return ret;
    }

    // directory
    if(S_ISDIR(stat_buf.st_mode)) {
        dir_ptr=opendir(file_name);
        ret = 0;
        while((dirent_ptr=readdir(dir_ptr))!=NULL) {
            if(strcmp(dirent_ptr->d_name,".") == 0 || strcmp(dirent_ptr->d_name,"..") == 0)
                continue;

            std::string new_file_name = ConnectPathComponent(file_name, dirent_ptr->d_name);
            ret += static_cast<int64_t>(stat_buf.st_size) + DuImpl(new_file_name.c_str(),error_code);
        }
        closedir(dir_ptr);
    } else {
        // Normal file
        return static_cast<int64_t>(stat_buf.st_size);
    }
    if (-1!=ret) {
        SetErrorCode(error_code, ERR_FILE_OK);
    } else {
        SetErrorCode(error_code, ERR_FILE_FAIL);
    }
    return ret;
#endif
}

int32_t LocalFile::MoveImpl(const char* src_name,const char* dst_name, uint32_t* error_code) {
    // not implement
    int32_t ret = -1;
#ifdef WIN32
    ret = ::MoveFile(src_name,dst_name);
    if (ret != 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
    }
    return ret;
#else
    int32_t file_exist = access(src_name,F_OK);
    if(file_exist != 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "file " << src_name << " not exists";
        return ret;
    }

    DIR * dir_ptr = NULL;
    struct dirent * dirent_ptr = NULL;
    struct stat stat_buf;
    struct stat stat_buf_dst;

    if(stat(src_name, &stat_buf) < 0) {
        ret = -1;
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "stat error for " << src_name;
        return ret;
    }
    if(stat(dst_name, &stat_buf_dst) < 0) {
        ret = -1;
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "stat error for " << src_name;
        return ret;
    }

    // Directory.
    if(S_ISDIR(stat_buf.st_mode)) {
        if (!S_ISDIR(stat_buf_dst.st_mode)) {
            ret = -1;
            SetErrorCode(error_code, ERR_FILE_FAIL);
            return ret;
        }
        dir_ptr=opendir(src_name);
        ret = 0;
        const char* base_name = strrchr(src_name,'/');
        if(base_name) {
            ++base_name;
        } else {
            base_name = src_name;
        }

        std::string complete_dst_name = ConnectPathComponent(dst_name, base_name);
        ret = AddDirImpl(complete_dst_name.c_str(), error_code);
        if (ret != 0) {
            closedir(dir_ptr);
            SetErrorCode(error_code, ERR_FILE_FAIL);
            return ret;
        }
        while((dirent_ptr=readdir(dir_ptr))!=NULL && ret == 0) {
            if(strcmp(dirent_ptr->d_name,".") == 0 || strcmp(dirent_ptr->d_name,"..") == 0)
                continue;

            std::string tmp_file_name = ConnectPathComponent(src_name, dirent_ptr->d_name);
            ret = MoveImpl(tmp_file_name.c_str(), complete_dst_name.c_str(), error_code);
        }
        closedir(dir_ptr);
        remove(src_name);
    } else {
        // Normal file
        const char* base_name = strrchr(src_name,'/');
        if(base_name) {
            ++base_name;
        } else {
            base_name = src_name;
        }

        std::string complete_dst_name = ConnectPathComponent(dst_name, base_name);

        ret = link(src_name, complete_dst_name.c_str());
        if (ret == 0) {
            ret = unlink(src_name);
        }
        if (ret != 0) {
            SetErrorCode(error_code, ERR_FILE_FAIL);
        }
        return ret;
    }

    if (0==ret) {
        SetErrorCode(error_code, ERR_FILE_OK);
    } else {
        SetErrorCode(error_code, ERR_FILE_FAIL);
    }

    return ret;
#endif
}

int32_t LocalFile::RenameImpl(const char* old_file_name,const char* new_file_name,  uint32_t* error_code) {
    SetErrorCode(error_code, ERR_FILE_OK);

    int32_t ret = -1;
    ret = rename(old_file_name,new_file_name);
    if (ret != 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
    }
    return ret;
}

int32_t LocalFile::RemoveImpl(const char* file_name,bool is_recursive, uint32_t* error_code) {
    SetErrorCode(error_code, ERR_FILE_OK);
    int32_t ret = -1;
#ifdef WIN32
    int32_t file_exist = _access(file_name,0);
    if(file_exist != 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "file " << file_name << " not exists";
        return ret;
    }

    int32_t len = (int32_t)strlen(file_name);
    if(len <= 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }
    char complete_file_name[512] = {0};
    bool has_directory = false;
    if(file_name[len - 1] == '/' ||
            file_name[len - 1] == '\\') {
        if (!is_recursive) {
            int32_t is_success = RemoveDirectory(file_name);
            if (is_success == 0) {
                ret = -1;
                SetErrorCode(error_code,ERR_FILE_FAIL);
            } else {
                SetErrorCode(error_code,ERR_FILE_OK);
                ret = 0;
            }
            return ret;
        }
        strcat(complete_file_name,file_name);
        complete_file_name[len - 1] = '\\';
        strcat(complete_file_name,"*.*");
        has_directory = true;
    } else if(file_name[len - 1] == ':') { // c:,d:
        if (!is_recursive) {
            int32_t is_success = RemoveDirectory(file_name);
            if (is_success == 0) {
                ret = -1;
                SetErrorCode(error_code,ERR_FILE_FAIL);
            } else {
                SetErrorCode(error_code, ERR_FILE_OK);
                ret = 0;
            }
            return ret;
        }
        strcat(complete_file_name,file_name);
        strcat(complete_file_name,"\\*.*");
        has_directory = true;
    } else {
        SetFileAttributes(file_name,FILE_ATTRIBUTE_NORMAL);
        int32_t delete_ret = DeleteFile(file_name);
        if (delete_ret == 0) {
            SetErrorCode(error_code, ERR_FILE_FAIL);
            return -1;
        } else {
            SetErrorCode(error_code, ERR_FILE_OK);
            return 0;
        }
    }

    WIN32_FIND_DATA file_data;
    HANDLE find_handle=::FindFirstFile(complete_file_name,&file_data);

    if(INVALID_HANDLE_VALUE == find_handle) {
        ret = -1;
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "file " << file_name << " not exists";
        return ret;
    }

    ret = 0;

    do {
        if(strcmp(file_data.cFileName,".") == 0 || strcmp(file_data.cFileName,"..") == 0 )
            continue;
        bool is_dir = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        char* fname = file_data.cFileName;
        if(strcmp(fname,".") == 0 || strcmp(fname,"..") == 0)
            continue;

        char new_file_name[512];
        new_file_name[0] = '\0';
        strcat(new_file_name,file_name);
        if (new_file_name[strlen(new_file_name) - 1] != '/') {
            strcat(new_file_name,"/");
        }
        strcat(new_file_name,fname);
        if(is_dir) {
            if(!is_recursive) {
                ret = -1;
                SetErrorCode(error_code,ERR_FILE_FAIL);
                break;
            }
            has_directory = true;
            strcat(new_file_name,"/");
            ret = Remove(new_file_name,is_recursive,error_code);
        } else {
            SetFileAttributes(new_file_name,FILE_ATTRIBUTE_NORMAL);
            DeleteFile(new_file_name);
        }


    }
    while(FindNextFile(find_handle,&file_data) && ret == 0);

    ::FindClose(find_handle);
    if (has_directory) {
        RemoveDirectory(file_name);
    }

    if(ret != 0) {
        SetErrorCode(error_code,ERR_FILE_FAIL);
    }
    else {
        SetErrorCode(error_code,ERR_FILE_OK);
    }
    return ret;
#else

    int32_t file_exist = access(file_name,F_OK);
    if(file_exist != 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "file " << file_name << " not exists";
        return ret;
    }

    DIR * dir_ptr = NULL;
    struct dirent * dirent_ptr = NULL;
    struct stat stat_buf;

    if(stat(file_name, &stat_buf) < 0) {
        ret = -1;
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "stat error for " << file_name;
        return ret;
    }

    // Directory
    if(S_ISDIR(stat_buf.st_mode)) {
        dir_ptr = opendir(file_name);
        ret = 0;
        if(!is_recursive) {
            bool is_success = remove(file_name);
            if (!is_success) {
                ret = -1;
                SetErrorCode(error_code,ERR_FILE_FAIL);
            } else {
                SetErrorCode(error_code, ERR_FILE_OK);
                closedir(dir_ptr);
                ret = 0;
            }
            return ret;

        }

        while((dirent_ptr = readdir(dir_ptr)) != NULL && ret == 0) {
            if(strcmp(dirent_ptr->d_name,".") == 0 || strcmp(dirent_ptr->d_name,"..") == 0)
                continue;

            char new_file_name[512] = {0};
            new_file_name[0] = '\0';
            strcat(new_file_name,file_name);
            if (new_file_name[strlen(new_file_name) - 1] != '/') {
                strcat(new_file_name,"/");
            }
            strcat(new_file_name,dirent_ptr->d_name);
            ret = RemoveImpl(new_file_name,is_recursive,error_code);
        }
        closedir(dir_ptr);
        remove(file_name);
    } else {
        // Normal file
        ret = remove(file_name);
    }

    if (0==ret) {
        SetErrorCode(error_code, ERR_FILE_OK);
    } else {
        SetErrorCode(error_code, ERR_FILE_FAIL);
    }
    return ret;
#endif
}

int32_t LocalFile::AddDirImpl(const char* file_name, uint32_t* error_code) {
    int32_t ret = -1;

    bool is_exist = CheckExistImpl(file_name);
    if (is_exist) {
        SetErrorCode(error_code, ERR_FILE_ENTRY_EXIST);

        return ret;
    }

#ifdef WIN32
    int32_t is_success = 0;
    is_success = CreateDirectory(file_name,0);
    if (is_success != 0) {
        SetErrorCode(error_code, ERR_FILE_OK);
        ret = 0;
    } else {
        SetErrorCode(error_code,ERR_FILE_FAIL);
        ret = -1;
    }
    return ret;
#else
    ret = mkdir(file_name,0777);
    if (ret != 0) {
        SetErrorCode(error_code,ERR_FILE_FAIL);
    } else {
        SetErrorCode(error_code, ERR_FILE_OK);
    }
    return ret;
#endif
}

int64_t LocalFile::GetSizeImpl( const char* file_name, uint32_t* error_code) {
    int64_t file_size = -1;
#ifdef WIN32
    if(!file_name) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "input file name is NULL";
        return file_size;
    }

    int32_t file_exist = _access(file_name,0);
    if(file_exist != 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "file " << file_name << " not exists";
        return file_size;
    }

    size_t len = strlen(file_name);
    if (len < 1) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return file_size;
    }

    if(file_name[len - 1] == '/' ||
            file_name[len - 1] == '\\'||
            file_name[len - 1] == ':' ) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "input file name " << file_name << " is a directory";
        return file_size;
    }

    WIN32_FIND_DATA file_data;
    HANDLE find_handle=::FindFirstFile(file_name,&file_data);

    if(INVALID_HANDLE_VALUE == find_handle) {
        file_size = 0;
        // Hidden files in Windows
        SetErrorCode(error_code, ERR_FILE_OK);
        return file_size;
    }

    // file size to return
    file_size = (int64_t)file_data.nFileSizeHigh * 1024 * 1024 * 1024 * 4 + file_data.nFileSizeLow;

    ::FindClose(find_handle);
    SetErrorCode(error_code, ERR_FILE_OK);
    return file_size;

#else
    int32_t file_exist = access(file_name,F_OK);
    if(file_exist != 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "file " << file_name << " not exists";
        return file_size;
    }

    struct stat stat_buf;

    if(stat(file_name, &stat_buf) < 0) {
        file_size = -1;
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "stat error for " << file_name;
        return file_size;
    }

    if(S_ISDIR(stat_buf.st_mode)) {
        file_size = -1;
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "input file name " << file_name << " is a directory";
        return file_size;
    } else {
        SetErrorCode(error_code, ERR_FILE_OK);
        file_size = static_cast<int64_t>(stat_buf.st_size);
    }
    return file_size;
#endif
}

int32_t LocalFile::LocateData( uint64_t start_pos, uint64_t end_pos,
                               std::vector<DataLocation>* buf, uint32_t* error_code) {
    CHECK_HANDLE(m_fd);
    if (!buf || (end_pos < start_pos)) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "parameter error";
        return -1;
    }

    // not implement
    SetErrorCode(error_code, ERR_FILE_FAIL);
    return -1;
}


void LocalFile::DoAsyncWrite(
    const void* buf,
    int64_t buf_size,
    Closure<void, int64_t, uint32_t>* callback,
    uint32_t time_out) {
    uint32_t error_code;
    int64_t writen_size = Write(buf, buf_size, &error_code);
    // callback->Run(writen_size, error_code);
    Closure<void>* task =
        NewClosure(
            callback,
            &Closure<void, int64_t, uint32_t>::Run,
            writen_size,
            error_code);
    g_local_thread_pool.AddTask(task);
}

void LocalFile::DoAsyncReadFrom(
    void* buffer,
    int64_t size,
    int64_t start_position,
    Closure<void, int64_t, uint32_t>* callback,
    uint32_t timeout) {
    uint32_t error_code = ERR_FILE_OK;
    int64_t read_size = 0;

    #ifdef WIN32
    {
        MutexLocker lock(g_file_mutex);
        if (start_position == Seek(start_position, SEEK_SET, &error_code)) {
            // only read when seek OK.
            read_size = Read(buffer, size, &error_code);
        }
    }
    #else
    read_size = pread64(m_fd, buffer, size, start_position);
    if (read_size < 0) {
        error_code = ERR_FILE_FAIL;
    }
    #endif

    // callback->Run(read_size, error_code);
    Closure<void>* task =
        NewClosure(
            callback,
            &Closure<void, int64_t, uint32_t>::Run,
            read_size,
            error_code);
    g_local_thread_pool.AddTask(task);
}

void LocalFile::AsyncReadFromAIOCallback(
    Closure<void, int64_t, uint32_t>* out_callback,
    int64_t size,
    uint32_t status_code) {
    uint32_t error_code = ERR_FILE_OK;

    // read size 0 is error.
    // Because we check start_position is less than file size before submitting read request.
    // When reading, the file is truncated so read size 0, return fail.
    if (status_code != 0u || size == 0) {
        error_code = ERR_FILE_FAIL;
    }

    out_callback->Run(size, error_code);
}

int32_t LocalFile::AsyncReadFrom( void* buffer,
                                  int64_t size,
                                  int64_t start_position,
                                  Closure<void, int64_t, uint32_t>* callback,
                                  uint32_t timeout,
                                  uint32_t* error_code) {
    CHECK_HANDLE(m_fd);
    if(!buffer || size <= 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }
    if (start_position < 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }
#ifdef WIN32
    Closure<void>* task =
        NewClosure(
            this,
            &LocalFile::DoAsyncReadFrom,
            buffer, size, start_position, callback, timeout);
    g_local_thread_pool.AddTask(task);

    SetErrorCode(error_code, ERR_FILE_OK);
    return 0;
#else
    off_t fsize = lseek(m_fd, 0, SEEK_END);

    if (start_position + 1 >= fsize) { // Reading any position exceeds file size is EOF.
        SetErrorCode(error_code, ERR_FILE_OK);

        Closure<void>* task =
            NewClosure(
                callback,
                &Closure<void, int64_t, uint32_t>::Run,
                static_cast<int64_t>(0),
                static_cast<uint32_t>(ERR_FILE_OK));
        g_local_thread_pool.AddTask(task);

        return 0;
    }

    Closure<void, int64_t, uint32_t>* aio_callback =
        NewClosure(this, &LocalFile::AsyncReadFromAIOCallback, callback);
    aioframe::StatusCode code;
    bool success = g_aioframe.AsyncRead(m_fd, buffer, size, start_position, aio_callback, &code);
    if (success) {
        *error_code = ERR_FILE_OK;
    } else {
        *error_code = ERR_FILE_FAIL;
        if (code == EAGAIN || code == ENOMEM) {
            *error_code = ERR_FILE_RETRY;
        }
    }

    return success ? 0 : -1;
#endif
}

bool LocalFile::CheckOpenModeValid(const uint32_t flags) {
    // Could not both W and A, or both W and R, or both A and R
    uint32_t invalid_mode
        = File::ENUM_FILE_OPEN_MODE_W | File::ENUM_FILE_OPEN_MODE_A;
    bool valid = (flags & invalid_mode) != invalid_mode;
    if (!valid) return false;

    invalid_mode
        = File::ENUM_FILE_OPEN_MODE_W | File::ENUM_FILE_OPEN_MODE_R;
    valid = (flags & invalid_mode) != invalid_mode;
    if (!valid) return false;

    invalid_mode
        = File::ENUM_FILE_OPEN_MODE_A | File::ENUM_FILE_OPEN_MODE_R;
    valid = (flags & invalid_mode) != invalid_mode;
    if (!valid) return false;

    return true;
}

int32_t LocalFile::ValidatePathWildcard(const char* file_path) {
    if (file_path == NULL) return -1;

    bool has_wildcard = false;

    size_t path_len = strlen(file_path);
    for (size_t i = 0; i < path_len; ++i) {
        char ch = file_path[i];
        if (ch == '\\') return false; // forbidden windows style seperator. For windows also use /
        if (ch == '*' || ch == '?' || ch == '[') has_wildcard = true;
        if (ch == '/' && has_wildcard) return -1; // not in last path component
    }

    return has_wildcard ? 1 : 0;
}

bool LocalFile::ChmodImpl(const char* path_name,
                          const uint32_t permission,
                          uint32_t* error_code)
{
    if (!path_name || permission > 0777u) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "Invalid param";
        return false;
    }
#ifndef WIN32
    if (chmod(path_name, permission) < 0) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return false;
    }
    SetErrorCode(error_code, ERR_FILE_OK);
    return true;
#endif
    SetErrorCode(error_code, ERR_FILE_FAIL);
    return false;
}

bool LocalFile::ChangeRoleImpl(const char* path_name,
                               const char* role_name,
                               uint32_t* error_code)
{
    if (!path_name || !role_name) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "Invalid param";
        return false;
    }

#ifndef WIN32
    std::string buf(role_name);
    std::vector<std::string> arr;
    SplitString(buf, ":", &arr);
    if (arr.size() != 2) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "Invalid role_name format, user_name:group_name";
        return false;
    }
    uid_t uid = 0;
    gid_t gid = 0;

    {
        MutexLocker lock(g_file_mutex);
        struct passwd* pw = getpwnam(arr[0].c_str());
        if (!pw) {
            SetErrorCode(error_code, ERR_FILE_FAIL);
            LOG(ERROR) << "get uid FAIL";
            return false;
        }
        uid = pw->pw_uid;
        struct group* gr = getgrnam(arr[1].c_str());
        if (!gr) {
            SetErrorCode(error_code, ERR_FILE_FAIL);
            LOG(ERROR) << "get gid FAIL";
            return false;
        }
        gid = gr->gr_gid;
    }

    if (0 == chown(path_name, uid, gid)) {
        SetErrorCode(error_code, ERR_FILE_OK);
        return true;
    } else {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        LOG(ERROR) << "Chown fail, error: " << strerror(errno);
        return false;
    }
#endif
    SetErrorCode(error_code, ERR_FILE_FAIL);
    return false;
}
