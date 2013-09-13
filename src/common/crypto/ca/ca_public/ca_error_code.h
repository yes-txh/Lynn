// XFS_InternalErrorCode.h
// XFS internal error code
//
//  wookin@tencent.com
//
// ---------------------------------------------------
#ifndef COMMON_CRYPTO_CA_CA_PUBLIC_ERRORCODE_H_
#define COMMON_CRYPTO_CA_CA_PUBLIC_ERRORCODE_H_
#include "common/base/stdint.h"

namespace ca {
//
//    CA ERROR CODE
//    1：这部分ERROR CODE是XFS_INTERNAL_ERROR_CODE中间摘出的一部分。
//    2：这部分ERROR CODE在XFS_INTERNAL_ERROR_CODE中也有对应的定义。
//    3：只使用CA ERROR CODE的时候不用关注XFS_INTERNAL_ERROR_CODE
//
enum CA_ERROR_CODE{
    // ------------------------------------------------------
    // general
	// ...
    // ------------------------------------------------------

    // ------------------------------------------------------
    // CA add by joeytian 
    ERROROR_CA_START                                     =13000,
    ERROR_CA_OK,
    ERROR_CA_PARAMETER_ERROR,
    ERROR_CA_UNREGISTERED_USER,
    ERROR_CA_OUT_OF_DATE,
    ERROR_CA_USER_LOCKED,
    ERROR_CA_INVALID_CERTIFICATE,
    ERROR_CA_INVALID_PRIVATEKEY,
    ERROR_CA_INVALID_USER_SIGN,
    ERROR_CA_DELETE_CERTIFICATE_FAIL,
    ERROR_CA_REGISTER_FAIL,
    ERROR_CA_READ_FAIL,
    ERROR_CA_OPEN_FAIL,
    ERROR_CA_IDENTITY_NOT_EXIST_IN_MEM,
    ERROR_CA_ROLE_NOT_EXIST_IN_MEM,
    ERROR_CA_RELATION_NOT_EXIST_IN_MEM,
    ERROR_CA_PAPRAM_INVALID,
    ERROR_CA_HAS_MAX_ROLES,
    ERROR_CA_ADD_TWICE,
    ERROR_CA_INVALID_FORMAT,
    ERROR_CA_INVALID_VERSION,
    ERROR_CA_INVALID_IDENTIY,
    ERROR_CA_LOAD_FILE,
    ERROR_CA_WRITE_FILE,
    ERROR_CA_FLUSH_FILE,
    ERROR_CA_RENAME_FILE,
    ERROR_CA_CLUSTER_NOT_EXIST,
    // ..
    ERROROR_CA_END                                       =13999,
    // ------------------------------------------------------

    // 按需要添加其他错误码
	// 如有需要请先参考XFS_INTERNAL_ERROR_CODE
    // ------------------------------------------------------
	// ...
    // ------------------------------------------------------
};

inline const char* CaGetErrorCodeStr(uint32_t err){
    switch(err){

    // add by joeytian
    case ERROR_CA_OK:                                     return "goes well!";
    case ERROR_CA_PARAMETER_ERROR:                        return "parameter error!";
    case ERROR_CA_UNREGISTERED_USER:                      return "unregistered user, "
                                                                 "register first!";
    case ERROR_CA_OUT_OF_DATE:                            return "certificate out of date, "
                                                                 "please renew!";
    case ERROR_CA_USER_LOCKED:                            return "user locked";
    case ERROR_CA_INVALID_CERTIFICATE:                    return "invalid certificate";
    case ERROR_CA_INVALID_USER_SIGN:                      return "invalid user sign";
    case ERROR_CA_DELETE_CERTIFICATE_FAIL:                return "delete certificate fail";
    case ERROR_CA_REGISTER_FAIL:                          return "register fail";
    case ERROR_CA_READ_FAIL:                              return "read local file fail";
    case ERROR_CA_OPEN_FAIL:                              return "open local file fail";
    case ERROR_CA_IDENTITY_NOT_EXIST_IN_MEM:              return "user not exist in map";
    case ERROR_CA_ROLE_NOT_EXIST_IN_MEM:                  return "role not exist in map";
    case ERROR_CA_RELATION_NOT_EXIST_IN_MEM:              return "relation not exist in map";
    case ERROR_CA_PAPRAM_INVALID:                         return "HTTP parameter err";
    case ERROR_CA_HAS_MAX_ROLES:                          return "user has too many roles, "
                                                                 "please get out of some";
    case ERROR_CA_ADD_TWICE:                              return "item existed, don't add twice!";
    case ERROR_CA_INVALID_FORMAT:                         return "key file format error";
    case ERROR_CA_INVALID_VERSION:                        return "key file version error";
    case ERROR_CA_INVALID_IDENTIY:                        return "key file identity error";

    case ERROR_CA_LOAD_FILE:                              return "load file error";
    case ERROR_CA_WRITE_FILE:                             return "write message to disk error";
    case ERROR_CA_FLUSH_FILE:                             return "flush message to disk error";
    case ERROR_CA_RENAME_FILE:                            return "rename file error";
    case ERROR_CA_CLUSTER_NOT_EXIST:                         return "request cluster not exist";

    // other error code
  //  case ERROR_PARSER_HTTP_CONTENT_FAIL:                return "parse HTTP content error";
  //  case ERROR_PARSE_PACKAGE_FAIL:                      return "parse package error";

    default:                                            return "unknown error code.";
    }
}


#ifndef SET_ERRORCODE
#define SET_ERRORCODE(ptr,error_code)   {if(ptr) *ptr=error_code;}
#endif //


inline const char* CaGetErrorCodeStr(uint32_t* error_code){
    if(!error_code)
        return "pErrCode is NULL.";
    else
        return CaGetErrorCodeStr(*error_code);
}

} // namespace ca

#endif // COMMON_CRYPTO_CA_CA_PUBLIC_ERRORCODE_H_
