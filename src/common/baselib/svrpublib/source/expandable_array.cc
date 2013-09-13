//
//  expandable_array.cc
//  bayou@tencent.com
// ///////////////////////////////////////////////////////

#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_


VarDataSerialize::VarDataSerialize() {
    m_is_inited = false;
}

VarDataSerialize::~VarDataSerialize() {
}

bool VarDataSerialize::ResetContent() {
    // 头部4字节按网络序保存元素计数
    const uint32_t kCountFieldLen   = sizeof(uint32_t);

    m_buff.ResetParam();

    if (m_buff.CheckBuffer(kCountFieldLen)) {
        // 清零count计数
        memset(m_buff.buff, 0, sizeof(uint32_t));
        m_buff.valid_len            = kCountFieldLen;
        m_is_inited                 = true;
        return true;
    }
    return false;
}

void VarDataSerialize::SetExtendStep(uint32_t extend_step) {
    m_buff.SetExtendStep(extend_step);
}

bool VarDataSerialize::AddVarData(const char* data, uint32_t len) {
    const uint32_t kCountFieldLen   = sizeof(uint32_t);

    if ((NULL == data)
            || (!m_is_inited && !ResetContent())) {
        return false;
    }

    if (m_buff.CheckNewAppendBuffer(sizeof(len) + len)) {
        uint32_t len_netorder = htonl(len);
        //  add data_len(net order)
        memcpy(m_buff.buff + m_buff.valid_len,
               &len_netorder,
               sizeof(len_netorder));

        //  add data
        memcpy(m_buff.buff + m_buff.valid_len + sizeof(len),
               data,
               len);

        //  count++
        uint32_t elem_count_hostord = 0;            // count计数，主机序
        uint32_t elem_count_netord  = 0;            // count计数，网络序
        memcpy(&elem_count_netord, m_buff.buff, kCountFieldLen);
        elem_count_hostord  = ntohl(elem_count_netord);
        elem_count_hostord++;
        elem_count_netord   = htonl(elem_count_hostord);
        memcpy(m_buff.buff, &elem_count_netord, kCountFieldLen);

        //  change valid_len
        m_buff.valid_len += sizeof(len) + len;
        return true;
    }

    return false;
}

bool VarDataSerialize::GetPackage(const char** data, uint32_t* len,
                                  uint32_t* item_count) {
    const uint32_t kCountFieldLen   = sizeof(uint32_t);
    uint32_t elem_count_netord      = 0;     // count计数，网络序

    if ((NULL == data)
            || (NULL == len)
            || (!m_is_inited && !ResetContent())) {
        return false;
    }

    *data       = m_buff.buff;
    *len        = m_buff.valid_len;
    memcpy(&elem_count_netord, m_buff.buff, kCountFieldLen);

    if (NULL != item_count) {
        *item_count = htonl(elem_count_netord);
    }

    return true;
}

VarDataUnSerialize::VarDataUnSerialize() {
    m_data          = NULL;
    m_valid_len     = 0;
    m_cursor        = NULL;
    m_current_pos   = 0;
    m_package_len   = 0;
}

VarDataUnSerialize::~VarDataUnSerialize() {
}

bool VarDataUnSerialize::AttachPackage(const char* data,
                                       uint32_t len) {
    const uint32_t kCountFieldLen   = sizeof(uint32_t);

    // count计数，网络序
    uint32_t elem_count_netord      = 0;

    if ((NULL == data)
            || (len < kCountFieldLen)) {
        return false;
    }

    m_data          = data;
    memcpy(&elem_count_netord, m_data, kCountFieldLen);
    m_valid_len     = ntohl(elem_count_netord);
    m_cursor        = m_data + kCountFieldLen;
    m_current_pos   = 0;
    m_package_len   = len;

    return true;
}

bool VarDataUnSerialize::GetNextVal(const char** data,
                                    uint32_t* len,
                                    uint32_t* pos) {
    if ((NULL == data)
            || (NULL == len)
            || (NULL == m_data)) {
        return false;
    }

    if (m_current_pos >= m_valid_len) {
        return false;
    }

    uint32_t elem_len_netord    = 0;
    uint32_t elem_len_hostord   = 0;
    memcpy(&elem_len_netord, m_cursor, sizeof(uint32_t));
    elem_len_hostord   = ntohl(elem_len_netord);

    if (m_cursor - m_data + sizeof(elem_len_hostord) + elem_len_hostord <=
            m_package_len) {
        *data    = m_cursor + sizeof(elem_len_hostord);
        *len     = elem_len_hostord;
        m_cursor += sizeof(elem_len_hostord) + elem_len_hostord;

        if (NULL != pos) {
            *pos = m_current_pos;
        }

        m_current_pos++;
        return true;
    }

    return false;
}


uint32_t VarDataUnSerialize::GetValidItemsCount() const {
    return m_valid_len;
}



bool expandable_array_unit_test() {
    bool is_ok = true;
    VarDataSerialize pack;

    // ////////////////////////////////////////////////////////////////////////
    // pack
    // ////////////////////////////////////////////////////////////////////////

    // set each element
    const char * test_str = "Google Buzz lets you share updates, "
                            "photos, links, and pretty much anything "
                            "else you'd like with your Gmail contacts; ";

    is_ok = is_ok && pack.AddVarData(test_str, (uint32_t)strlen(test_str) + 1);

    test_str = "it's an easy way to follow your friends, too. ";
    is_ok = is_ok && pack.AddVarData(test_str, (uint32_t)strlen(test_str) + 1);

    test_str = "When you click Buzz in your Gmail account, "
               "you'll see the stream of posts from people you're following, "
               "and a box for you to post your updates. ";
    is_ok = is_ok && pack.AddVarData(test_str, (uint32_t)strlen(test_str) + 1);

    test_str = " Here's a run-down of the basic functions:";
    is_ok = is_ok && pack.AddVarData(test_str, (uint32_t)strlen(test_str) + 1);
    // get package
    const char * package_data    = NULL;
    uint32_t package_size           = 0;
    is_ok = is_ok && pack.GetPackage(&package_data, &package_size);



    // ////////////////////////////////////////////////////////////////////////
    // unpack
    // ////////////////////////////////////////////////////////////////////////
    VarDataUnSerialize unpack;
    // attach package
    is_ok = is_ok && unpack.AttachPackage(package_data, package_size);

    uint32_t elem_count = unpack.GetValidItemsCount();

    for (uint32_t i = 0; i < elem_count; i++) {
        const char* elem = NULL;
        uint32_t elem_len   = 0;
        uint32_t pos        = 0;

        if (unpack.GetNextVal(&elem, &elem_len, &pos)) {
            VLOG(3) << "The " << pos <<" elem, len = " << elem_len << ", content:" << elem;
        }
    }

    return is_ok;
}

_END_XFS_BASE_NAMESPACE_
