//////////////////////////////////////////////////////////////////////////
//
//@file:   file.cc
//@author: wookin
//@author: aaronzou     2011.01.10
//                      Refactor to use polymorphism to forward to subclasses.
//////////////////////////////////////////////////////////////////////////

#ifndef WIN32
#include <pthread.h>
#include <sys/un.h>
#endif

#include <stdlib.h>

#include <map>
#include <vector>

#include "common/file/file.h"
#include "common/file/local_file.h"
#include "common/system/concurrency/mutex.hpp"
#include "thirdparty/glog/logging.h"
//////////////////////////////////////////////////////////////////////////
//成员函数

std::string File::GetFilePrefix(const char* file_path) {
    std::string prefix;

    if (NULL == file_path) {
        return prefix;
    }

    std::string file(file_path);

    std::string prefixSeg;
    prefixSeg += File::kPathSeparator;

    // skip the head segment.
    size_t idx = file.find(prefixSeg, prefixSeg.size());
    if (std::string::npos == idx) {
        return prefix;
    }

    return file.substr(0, idx + prefixSeg.size());
}

File* File::CreateFileImpl(const std::string& prefix) {
    File* file_impl = CREATE_FILE_IMPL(prefix);
    if (file_impl == NULL) {
        file_impl = CREATE_FILE_IMPL(LOCAL_FILE_PREFIX);
    }

    return file_impl;
}

File* File::GetFileImplSingleton(const std::string& prefix) {
    File* file_impl = GET_FILE_IMPL_SINGLETON(prefix);
    if (file_impl == NULL) {
        file_impl = GET_FILE_IMPL_SINGLETON(LOCAL_FILE_PREFIX);
    }

    return file_impl;
}

bool File::Init(const char* user_name) {
    // init all file impl.
    bool has_error = false;
    for (size_t i = 0; i < FILE_IMPL_COUNT(); ++i) {
        std::string file_impl_name = FILE_IMPL_NAME(i);
        File *file_impl = GetFileImplSingleton(file_impl_name);
        assert(file_impl != NULL);

        bool this_OK = file_impl->InitImpl(user_name);
        if (this_OK != true) {
            has_error = true;
        }

        LOG(INFO) << "Init file impl " << file_impl_name
            << " Success? " << (this_OK ? "true" : "false");
    }

    CHECK(FILE_IMPL_COUNT() > 0) << " No file implementation is initilized. "
        << "May have force link problems.";

    LOG(INFO) << "Init all " << FILE_IMPL_COUNT() << " file implementations "
        << ". Success? " << (!has_error ? "true" : "false");

    return !has_error;
}

void File::CleanUp() {
    // init all file impl.
    bool has_error = false;
    for (size_t i = 0; i < FILE_IMPL_COUNT(); ++i) {
        std::string file_impl_name = FILE_IMPL_NAME(i);
        File *file_impl = GetFileImplSingleton(file_impl_name);

        if (file_impl->CleanupImpl() != true) {
            has_error = true;
            LOG(ERROR) << "fail to cleanup file impl " << file_impl_name;
        } else {
            DLOG(INFO) << "success to cleanup file impl " << file_impl_name;
        }
    }

    if (has_error) {
        LOG(ERROR) << "Fail to cleanup all file implementations.";
    } else {
        LOG(INFO) << "success to cleanup all " << FILE_IMPL_COUNT() << " file implementations";
    }
}

File* File::Open(const char* file_path, uint32_t flags,
    const OpenFileOptions& options,
    uint32_t* error_code) {
    if (!file_path) {
        SetErrorCode(error_code, ERR_FILE_PARAMETER);
        return NULL;
    }
    std::string prefix = GetFilePrefix(file_path);
    // Must create a new File object and return.
    File* file_obj = CreateFileImpl(prefix);
    if (NULL == file_obj) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return NULL;
    }

    if (true == file_obj->OpenImpl(file_path, flags, options, error_code)) {
        return file_obj;
    } else {
        delete file_obj;
        return NULL;
    }
}

int32_t File::Copy(const char*   src_file_path,
                   const char*   dest_file_path,
                   uint32_t*     error_code) {
    if(!src_file_path || !dest_file_path) {
        SetErrorCode(error_code, ERR_FILE_PARAMETER);
        return -1;
    }

    int32_t   ret = 0;
    //////////////////////////////////////////////////////////////////////////
    //1.打开原文件
    File* src_file=File::Open(src_file_path,
                              File::ENUM_FILE_OPEN_MODE_R,
                              OpenFileOptions(), error_code);
    if (!src_file) {
        return -1;
    }
    //////////////////////////////////////////////////////////////////////////
    //2.得到原文件的长度
    int64_t src_file_len = File::GetSize(src_file_path, error_code);

    if (src_file_len == -1) {
        src_file->Close();
        delete src_file;
        src_file = NULL;
        return -1;
    }
    //////////////////////////////////////////////////////////////////////////
    //3.打开或创建目标文件
    File* dest_file=File::Open(dest_file_path,
                               File::ENUM_FILE_OPEN_MODE_W,
                               OpenFileOptions(), error_code);
    if (!dest_file) {
        src_file->Close();
        delete src_file;
        src_file = NULL;
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////
    //4.计算copy的range范围
    uint64_t remain_len_in_src = src_file_len;  // 还有多少数据需要从源文件获取

    //5.移动到读的位置
    src_file->Seek(0,SEEK_SET);
    //6.计划读取多长数据
    uint64_t plan_read_count = src_file_len;

    uint32_t buff_len = 4 * 1024* 1024; //4M
    char*    buff     = new char[buff_len];

    uint64_t all_write_count = 0;
    uint32_t received_errcode = ERR_FILE_OK;
    // 移动到指定位置
    src_file->Seek(0,SEEK_SET,error_code);
    while (remain_len_in_src>0 && ret != -1) {
        // 同步读取文件
        int64_t max_read_len = (remain_len_in_src > buff_len)?buff_len:(uint32_t)remain_len_in_src;
        int64_t read_count = src_file->Read(buff,max_read_len, &received_errcode);
        if (read_count == -1) {
            ret = -1;
            break;
        }
        else if (read_count > 0) {
            remain_len_in_src -= read_count;
            while (read_count>0) {
                int64_t write_count = dest_file->Write(buff,read_count, &received_errcode);
                if (write_count <= 0) { // read-count>0, 那么write_count不能小于等于0
                    ret = -1;
                    break;
                }
                else if (write_count > 0) {
                    all_write_count += write_count;
                    read_count -= write_count;
                }
            }
        }
        else if (read_count == 0) { //read over
            break;
        }
    }

    uint32_t* code = (received_errcode == ERR_FILE_OK) ? (&received_errcode) : NULL;
    bool is_close_success = true;
    if (src_file->Close(code) != 0) {
        ret = -1;
        is_close_success = false;
    }
    if (dest_file->Close(code) != 0) {
        ret = -1;
        is_close_success = false;
    }

    delete []buff;
    buff = NULL;

    delete src_file;
    src_file = NULL;

    delete dest_file;
    dest_file = NULL;

    if((all_write_count == plan_read_count) && is_close_success) {
        SetErrorCode(error_code, ERR_FILE_OK);
        ret = 0;
    }
    else {
        SetErrorCode(error_code, static_cast<FILE_ERROR_CODE>(received_errcode));
        ret = -1;
    }
    return ret;
}

int32_t File::List(const char* name,
                    AttrsMask* mask,
                    std::vector<AttrsInfo>* buffer,
                    uint32_t* error_code) {
    if (!name || !buffer) {
        SetErrorCode(error_code, ERR_FILE_PARAMETER);
        return -1;
    }
    // Add list for other fs type here
    std::string prefix = GetFilePrefix(name);
    File* file_impl = GetFileImplSingleton(prefix);
    if (NULL == file_impl) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }

    return file_impl->ListImpl(name, mask, buffer, error_code);
}

bool File::CheckExist(const char* name, uint32_t* error_code) {
    return File::IsExist(name, error_code);
}
bool File::IsExist(const char* name, uint32_t* error_code) {
    if (!name) {
        SetErrorCode(error_code, ERR_FILE_PARAMETER);
        return false;
    }
    std::string prefix = GetFilePrefix(name);
    File* file_impl = GetFileImplSingleton(prefix);
    if (NULL == file_impl) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return false;
    }

    return file_impl->CheckExistImpl(name, error_code);
}

int64_t File::Du( const char* name, uint32_t* error_code  ) {
    if (!name) {
        SetErrorCode(error_code, ERR_FILE_PARAMETER);
        return -1;
    }
    std::string prefix = GetFilePrefix(name);
    File* file_impl = GetFileImplSingleton(prefix);
    if (NULL == file_impl) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }

    return file_impl->DuImpl(name, error_code);
}

int32_t File::Move( const char* old_file_name, const char* new_file_name, uint32_t* error_code  ) {
    //Use MoveImpl instead of Rename for polymorphism.
    //return File::Rename(src_name, dest_name, error_code);
    if (!old_file_name || !new_file_name) {
        SetErrorCode(error_code, ERR_FILE_PARAMETER);
        return -1;
    }

    //TODO How to handle the different src and dest file type?
    std::string prefix = GetFilePrefix(old_file_name);
    std::string dest_prefix = GetFilePrefix(new_file_name);
    if (prefix != dest_prefix) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }

    File* file_impl = GetFileImplSingleton(prefix);
    if (NULL == file_impl) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }
    return file_impl->MoveImpl(old_file_name, new_file_name, error_code);
}

int32_t File::Rename( const char* old_file_name, const char* new_file_name, uint32_t* error_code) {
    if (!old_file_name || !new_file_name) {
        SetErrorCode(error_code, ERR_FILE_PARAMETER);
        return -1;
    }

    //TODO How to handle the different src and dest file type?
    std::string prefix = GetFilePrefix(old_file_name);
    std::string dest_prefix = GetFilePrefix(new_file_name);
    if (prefix != dest_prefix) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }

    File* file_impl = GetFileImplSingleton(prefix);
    if (NULL == file_impl) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }
    return file_impl->RenameImpl(old_file_name, new_file_name, error_code);
}

int32_t File::Remove(const char* file_path, bool is_recursive, uint32_t* error_code) {
    if (!file_path) {
        SetErrorCode(error_code, ERR_FILE_PARAMETER);
        return -1;
    }

    std::string prefix = GetFilePrefix(file_path);

    File* file_impl = GetFileImplSingleton(prefix);
    if (NULL == file_impl) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }
    return file_impl->RemoveImpl(file_path, is_recursive, error_code);
}

int32_t File::AddDir(const char* file_path, uint32_t* error_code) {
    if (!file_path) {
        SetErrorCode(error_code, ERR_FILE_PARAMETER);
        return -1;
    }

    std::string prefix = GetFilePrefix(file_path);

    File* file_impl = GetFileImplSingleton(prefix);
    if (NULL == file_impl) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }
    return file_impl->AddDirImpl(file_path, error_code);
}

int64_t File::GetSize(const char* file_path, uint32_t* error_code) {
    if (!file_path) {
        SetErrorCode(error_code, ERR_FILE_PARAMETER);
        return -1;
    }

    std::string prefix = GetFilePrefix(file_path);

    File* file_impl = GetFileImplSingleton(prefix);
    if (NULL == file_impl) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return -1;
    }
    return file_impl->GetSizeImpl(file_path, error_code);
}


File* File::OpenOrDie( const char* file_path, uint32_t flags,
    const OpenFileOptions& options) {
    File* fp = File::Open(file_path, flags, options);
    if (fp == NULL) {
        //log and exit.
        LOG(FATAL) << "failed to open " << file_path << " with flags " << flags;
    }
    return fp;
}

int32_t File::GetMatchingFiles(const char* pattern, std::vector<std::string>* files) {
    if (!pattern || !files) {
        return -1;
    }
    AttrsMask mask;
    uint32_t error_code = ERR_FILE_OK;
    std::vector<AttrsInfo> file_info;
    if (0 == File::List(pattern, &mask, &file_info, &error_code)) {
        files->reserve(file_info.size());
        for (std::vector<AttrsInfo>::iterator iter = file_info.begin();
                iter != file_info.end();
                ++iter) {
            files->push_back(iter->file_name);
        }
        return 0;
    }
    else {
        LOG(ERROR) << "GetMatchingFiles fail: " << GetFileErrorCodeStr(error_code);
        return -1;
    }
}

bool File::Chmod(const char* path_name,
                 const uint32_t permission,
                 uint32_t* error_code) {
    if (!path_name) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return false;
    }

    std::string prefix = GetFilePrefix(path_name);

    File* file_impl = GetFileImplSingleton(prefix);
    if (NULL == file_impl) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return false;
    }

    return file_impl->ChmodImpl(path_name, permission, error_code);
}

bool File::ChangeRole(const char* path_name,
                 const char* role_name,
                 uint32_t* error_code) {
    if (!path_name || !role_name) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return false;
    }

    std::string prefix = GetFilePrefix(path_name);

    File* file_impl = GetFileImplSingleton(prefix);
    if (NULL == file_impl) {
        SetErrorCode(error_code, ERR_FILE_FAIL);
        return false;
    }

    return file_impl->ChangeRoleImpl(path_name, role_name, error_code);
}

// for switch-case.
#define CONSIDER_CASE(type) case type: return #type


const char* GetFileTypeDesc(ENUM_FILE_TYPE type) {
    switch (type) {
        CONSIDER_CASE(FILE_TYPE_UNKNOWNTYPE);
        CONSIDER_CASE(FILE_TYPE_NORMAL);
        CONSIDER_CASE(FILE_TYPE_DIR);
        CONSIDER_CASE(FILE_TYPE_RA);
        default:
            return "---unknown file type---";
    }
}


const char* GetFileErrorCodeStr(unsigned int error_code) {
    switch(error_code) {
        CONSIDER_CASE(ERR_FILE_FAIL);
        CONSIDER_CASE(ERR_FILE_OK);
        CONSIDER_CASE(ERR_FILE_RETRY);
        CONSIDER_CASE(ERR_FILE_REOPEN_FOR_WRITE);
        CONSIDER_CASE(ERR_FILE_ENTRY_EXIST);
        CONSIDER_CASE(ERR_FILE_ENTRY_NOT_EXIST);
        CONSIDER_CASE(ERR_FILE_NOT_INIT);
        CONSIDER_CASE(ERR_FILE_CLOSED);
        CONSIDER_CASE(ERR_FILE_OPENMODE);
        CONSIDER_CASE(ERR_FILE_PARAMETER);
        CONSIDER_CASE(ERR_FILE_PERMISSION_DENIED);
        default:
            return "ERR_FILE_UNKNOWN";
    }
}

bool KeyValueInfoHelper::ParseKeyValueInfo(
    const std::string& info,
    std::map<std::string, std::string>* info_map) {
    if (info_map == NULL) return false;
    if (info_map->size() > 0) info_map->clear();

    // segment options by ':'
    std::vector<std::string> segs;
    std::string::size_type index = -1;
    std::string::size_type preIndex = 0;
    while((index = info.find(':', preIndex)) != -1) {
        segs.push_back(info.substr(preIndex, index - preIndex));
        preIndex = index + 1;
    }
    if (preIndex < info.length()) {
        segs.push_back(info.substr(preIndex));
    }

    // parse each segment.
    for (std::vector<std::string>::size_type i = 0; i < segs.size(); ++i) {
        std::string::size_type eqIndex = segs[i].find('=');
        if (std::string::npos == eqIndex) return false; //no '=', wrong options.

        const std::string key = segs[i].substr(0, eqIndex);
        const std::string value = segs[i].substr(eqIndex + 1);
        if (key.length() == 0 || value.length() == 0) {
            return false; // wrong options.
        }

        info_map->insert(make_pair(key, value));
    }

    return true;
}

bool KeyValueInfoHelper::CreateKeyValueInfo(
    const std::map<std::string, std::string>& info_map,
    std::string* info) {
    if (info == NULL) return false;
    if (info->length() > 0) info->clear();

    std::string& info_ref = *info;

    std::map<std::string, std::string>::const_iterator it = info_map.begin();
    for ( ; it != info_map.end(); ++it) {
        if (info_ref.length() > 0)
            info_ref += ':';
        if (it->first.empty() || it->second.empty()) {
            return false;
        }

        info_ref += it->first;
        info_ref += '=';
        info_ref += it->second;
    }

    return true;
}

bool KeyValueInfoHelper::AppendKeyValueInfo(const std::string& key,
        const std::string& value, std::string* info) {
    if (info == NULL) return false;
    if (key.length() == 0 || value.length() == 0) return false;

    if (info->length() > 0) (*info) += ":";

    (*info) += key;
    (*info) += "=";
    (*info) += value;

    return true;
}
//////////////////////////////////////////////////////////////////////////
