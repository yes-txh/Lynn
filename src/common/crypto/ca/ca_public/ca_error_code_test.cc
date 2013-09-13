#include "common/crypto/ca/ca_public/ca_error_code.h"
#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "thirdparty/gtest/gtest.h"

using namespace::ca;

class CaErrorCodeTest : public :: testing :: Test {
public:
};

TEST_F(CaErrorCodeTest, CaGetErrorCodeStr) {
    for(int32_t e = ERROR_CA_OK; e < ERROROR_CA_END; ++e)
        CaGetErrorCodeStr(static_cast<CA_ERROR_CODE>(e));

    uint32_t err = ERROR_CA_OK;
    CHECK_EQ(0, strcmp(CaGetErrorCodeStr(&err), "goes well!"));
}
