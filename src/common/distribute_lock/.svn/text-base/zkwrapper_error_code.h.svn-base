// zkwrapper_error_code.h
//
//  typherque@tencent.com
//
// ---------------------------------------------------
#ifndef DISTRIBUTE_LOCK_ZKWRAPPER_ERROR_CODE_H_
#define DISTRIBUTE_LOCK_ZKWRAPPER_ERROR_CODE_H_
#include "common/base/stdint.h"

//
//    ZKWRAPPER ERROR CODE
//    1：这部分ERROR CODE是XFS_INTERNAL_ERROR_CODE中间摘出的一部分。
//    2：这部分ERROR CODE在XFS_INTERNAL_ERROR_CODE中也有对应的定义。
//    3：只使用ZKWRAPPER ERROR CODE的时候不用关注XFS_INTERNAL_ERROR_CODE
//
enum ZKWRAPPER_ERROR_CODE {
    // ------------------------------------------------------
    // general
    ERR_OK                          =1,  // ok,TRUE
    // ...
    // ------------------------------------------------------

    // ------------------------------------------------------
    // 14000-14999 分给zookeeperWrapper使用
    ERROR_ZKWRAPPER_START                              = 14000,
    ERROR_ZKWRAPPER_OK,
    ERROR_ZKWRAPPER_OPEN_CONFIG_FILE_FAIL,
    ERROR_ZKWRAPPER_INIT_ZK_FAIL,
    ERROR_ZKWRAPPER_INVALID_CLUSTER_NAME,
    ERROR_ZKWRAPPER_INIT_ACL_FAIL,
    ERROR_ZKWRAPPER_ZOOGET_FAIL,
    ERROR_ZKWRAPPER_END                                = 14999,
    // ------------------------------------------------------

    // 按需要添加其他错误码
    // 如有需要请先参考XFS_INTERNAL_ERROR_CODE
    // ------------------------------------------------------
    // ...
    // ------------------------------------------------------
};

#ifndef DESCRIBE_ERROR
#define DESCRIBE_ERROR(code) case (code): return (#code)
#endif

inline const char* ZKWrapperGetErrorCodeStr(uint32_t err) {
    switch(err) {
        DESCRIBE_ERROR(ERR_OK);
        DESCRIBE_ERROR(ERROR_ZKWRAPPER_OK);
        DESCRIBE_ERROR(ERROR_ZKWRAPPER_OPEN_CONFIG_FILE_FAIL);
        DESCRIBE_ERROR(ERROR_ZKWRAPPER_INIT_ZK_FAIL);
        DESCRIBE_ERROR(ERROR_ZKWRAPPER_INVALID_CLUSTER_NAME);
        DESCRIBE_ERROR(ERROR_ZKWRAPPER_INIT_ACL_FAIL);
        DESCRIBE_ERROR(ERROR_ZKWRAPPER_ZOOGET_FAIL);
        default:
            return "ERROR_ZKWRAPPER_UNKNOWN";
    }
}


#ifndef SET_ERRORCODE
#define SET_ERRORCODE(ptr,error_code)   {if(ptr) *ptr=error_code;}
#endif //


inline const char* ZKGetErrorCodeStr(uint32_t* error_code)
{
    if(!error_code)
        return "error code is NULL.";
    else
        return ZKWrapperGetErrorCodeStr(*error_code);
}

#endif // DISTRIBUTE_LOCK_ZKWRAPPER_ERROR_CODE_H_
