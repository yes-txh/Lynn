// ca_struc.h
// joeytian@tencent.com
#ifndef COMMON_CRYPTO_CA_CA_PUBLIC_CA_STRUCT_H_
#define COMMON_CRYPTO_CA_CA_PUBLIC_CA_STRUCT_H_

#include <string.h>
#include <time.h>
#include <string>
#ifdef WIN32
#include <io.h>
#else
#include <sys/io.h>
#endif

#include "common/crypto/rsa2/rsa2.h"
#include "common/crypto/rsa2/rsa2_ref/rsa_source/rsa_ref.h"
#include "gflags/gflags.h"

// set ca_server=mockca.soso.oa.com to skip ca verify
DECLARE_string(ca_server);
// using --identity=xxx to set identity(user) name
DECLARE_string(identity);
// using --role=xxx to set role(group) name
DECLARE_string(role);

namespace ca {

//  证书的有效期是半年，按182天算的
#define DEFAULT_CA_CERTIFICATE_TIMEOUT 15724800
#define CA_MEMO                        "XFS_CA authenticated,Tencent"
#define DEFAULT_CA_TICKET_TIMEOUT      86400

// set error code
#ifndef SET_ERRORCODE
#define SET_ERRORCODE(ptr, error_code)   {if (ptr) *ptr = error_code;}
#endif //

#ifdef WIN32 // win32下没用，目前只是调试时用到
#define SPLIT_SIGN "\\"
#else
#define SPLIT_SIGN "/"
#endif

const uint32_t kUserNameLen  = 32;
const uint32_t kModuleNameLen  = 32;
const uint32_t kMemoLen  = 64;
const uint32_t kCaSignLen  = 128;
const uint32_t kFmtLen = 32;
const uint16_t kCurrentVersion = 2;
const uint32_t kMaxPrivateKeyLen = 1024;
const uint32_t kMaxInfoLen = 1024;
const uint32_t kMaxPublicKeyLen = 512;
const uint32_t kEncryptCaTokenLen = 260;
const uint32_t kRsaKeyBitsCount = 600;
const char* const kCaPriFmt = "CA_PRIVATE_KEY";
const char* const kCaPubFmt = "CA_PBULIC_KEY";
const char* const kPubFileName = "_certificate.dat";
const char* const kPriFileName = "_private_key.dat";
const char* const kApacheHtdocsName = "htdocs";
const char* const kCaDirName = "ca_dir";
// role manager
const uint32_t kMaxIdNum    = 524287;
const uint32_t kMaxDirLen   = 256;
const uint32_t kMaxNameLen  = 32;
const uint32_t kMaxRoles    = 10;
const char* const kCaDir            = "htdocs"SPLIT_SIGN"ca_dir"SPLIT_SIGN;
const char* const kRoleFileName     = "role_pb.dat";
const char* const kIdentityFileName = "identity_pb.dat";
const char* const kRelatedFileName  = "related_pb.dat";
const char* const kQuotaFileName  = "quota_pb.dat";

#ifdef WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif //

struct CACertificateData {
    char user_name[kUserNameLen];
    char user_public_key[kMaxPublicKeyLen];
    // duration
    time_t certificate_time_begin;
    time_t certificate_time_end;
    CACertificateData() : certificate_time_begin(0),
                          certificate_time_end(0){
        user_name[0] = 0;
        user_public_key[0] = 0;
    }
};

struct CACertificate {
    char fmt[kFmtLen];
    uint16_t version;
    uint32_t user_name_len;
    uint32_t public_key_len;
    CACertificateData ca_certificate_data;
    uint32_t sign_len;
    char* ca_sign; // 用CA的private_key来对ca_certificate_data值的hash进行加密
    char memo[kMemoLen];
    CACertificate() : version(0),
                      user_name_len(0),
                      public_key_len(0),
                      sign_len(0),
                      ca_sign(NULL){
        fmt[0] = 0;
        memo[0] = 0;
    }
};


struct CAPrivate{
    char fmt[kFmtLen];
    uint16_t version;
    uint32_t private_key_len;
    char* private_key;
    uint32_t user_name_len;
    char* user_name;
    uint32_t sign_len;
    char* sign; // 用用户的private_key来对用户名的hash进行加密
    CAPrivate() : version(0),
                  private_key_len(0),
                  private_key(NULL),
                  user_name_len(0),
                  user_name(NULL),
                  sign_len(0),
                  sign(NULL) {
        fmt[0] = 0;
    }
};

struct RsaKeyPair {
    R_RSA_PUBLIC_KEY  rsa_public_key;
    R_RSA_PRIVATE_KEY rsa_private_key;
};

// role manager
typedef struct Item {
    uint32_t id;
    bool valid;
    std::vector<uint32_t> list;
    Item(): id(0), valid(true) {
    }
} Item;

typedef struct Quota {
    uint32_t num_chunks;
    uint32_t num_files;
    uint32_t num_directories;
    explicit Quota() : num_chunks(0), num_files(0), num_directories(0) {
    }
} Quota;

#ifdef WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif //

} // namespace ca
#endif // COMMON_CRYPTO_CA_CA_PUBLIC_CA_STRUCT_H_
