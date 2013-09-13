#include "common/file/recordio/recordio.h"
#include "common/system/time/time_utils.hpp"
#include "common/crypto/ca/ca_public/ca_struct.h"
#include "common/crypto/ca/ca_server/ca_storage.pb.h"
#include "common/crypto/ca/ca_server/role_manager.h"
#include "common/crypto/ca/ca_server/quota_manager.h"

namespace ca {

RoleManager* RoleManager::s_role_manager = NULL;
CXThreadMutex RoleManager::s_mutex;

RoleManager::RoleManager() : m_max_identity_id(0), m_max_role_id(0) {
    // set role dir
    char module_dir[kMaxDirLen] = {0};
    GetModuleFileName(NULL, module_dir, sizeof(module_dir));
    char* p = strrchr(module_dir, '/');
    if (!p)
        p = strrchr(module_dir, '\\');
    CHECK(p);
    p++;
    CHECK_GT(sizeof(module_dir) - 1, STRLEN(module_dir) + STRLEN(kCaDir));
    safe_snprintf(p, sizeof(module_dir), "%s", kCaDir);

    // init filenames
    safe_snprintf(m_role_filename, kMaxDirLen, "%s%s", module_dir, kRoleFileName);
    safe_snprintf(m_identity_filename, kMaxDirLen, "%s%s", module_dir, kIdentityFileName);
    safe_snprintf(m_related_filename, kMaxDirLen, "%s%s", module_dir, kRelatedFileName);
}

RoleManager::~RoleManager() {
    CleanConfig();
}

bool RoleManager::LoadConfig() {
    // init role
    bool ret = LoadRoleNames();
    // init identity
    ret &= LoadIdentityNames();
    // init related
    ret &= LoadRelations();

    if(!ret)
        CleanConfig();
    return ret;
}

bool RoleManager::ReloadConfig() {
    CXThreadAutoLock auto_lock(&s_mutex);
    CleanConfig();
    return LoadConfig();
}

void RoleManager::CleanConfig() {
    map<string, Item*>::iterator it;
    for (it = m_identities.begin(); it != m_identities.end(); ++ it)
        delete it->second;

    for (it = m_roles.begin(); it != m_roles.end(); ++ it)
        delete it->second;

    m_identity_names.clear();
    m_role_names.clear();
    m_identities.clear();
    m_roles.clear();
}

bool RoleManager::AddIdentity(const char* identity_name, CA_ERROR_CODE* ptr_error_code) {
    CXThreadAutoLock auto_lock(&s_mutex);
    return AddIdentityWithoutLock(identity_name, ptr_error_code);
}
bool RoleManager::AddIdentityWithoutLock(const char* identity_name, CA_ERROR_CODE* ptr_error_code) {
    uint32_t identity_name_len = STRLEN(identity_name);
    if (!(identity_name_len < kMaxNameLen && identity_name_len > 0)) {
        LOG(ERROR) << "identity " << identity_name << " is null or too long!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_PAPRAM_INVALID);
        return false;
    }

    // exist?
    uint32_t iid = GetIdentityIdByName(identity_name);
    if (iid != 0) {
        LOG(ERROR) << "identity " << identity_name << " already exist!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_ADD_TWICE);
        return false;
    }

    if (m_max_identity_id >= kMaxIdNum) {
        LOG(ERROR) << "Identity id reach the max num : " << kMaxIdNum;
        return false;
    }

    // add to map
    ++m_max_identity_id;
    iid = m_max_identity_id;
    m_identity_names.insert(pair<uint32_t, string>(iid, string(identity_name)));

    Item* item = new Item;
    item->id = iid;
    m_identities.insert(pair<string, Item*>(identity_name, item));

    // update local file
    bool ret = DumpIdentityNames(ptr_error_code);

    // add role and relation
    if (ret) {
        ret = AddRoleWithoutLock(identity_name, ptr_error_code);
        if (ret) {
            ret = AddIdentityToRoleWithoutLock(identity_name, identity_name, ptr_error_code);
            if (!ret)
                DelIdentityWithoutLock(identity_name, ptr_error_code);
        }
    }

    return ret;
}

bool RoleManager::DelIdentity(const char* identity_name,
                              CA_ERROR_CODE* ptr_error_code) {
    CXThreadAutoLock auto_lock(&s_mutex);
    return DelIdentityWithoutLock(identity_name, ptr_error_code);
}
bool RoleManager::DelIdentityWithoutLock(const char* identity_name,
                                         CA_ERROR_CODE* ptr_error_code) {
    uint32_t identity_name_len = STRLEN(identity_name);
    if (!(identity_name_len < kMaxNameLen && identity_name_len > 0)) {
        LOG(ERROR) << "role " << identity_name << " is null or too long!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_PAPRAM_INVALID);
        return false;
    }

    // delete from map
    uint32_t iid = GetIdentityIdByName(identity_name);
    if (iid == 0) {
        LOG(ERROR) << "role " << identity_name << "does not exist!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_IDENTITY_NOT_EXIST_IN_MEM);
        return false;
    } else {
        m_identity_names.erase(iid);
        map<string, Item*>::iterator it = m_identities.find(identity_name);
        Item* item = it->second;
        for (vector<uint32_t>::iterator it_rid = item->list.begin();
             it_rid != item->list.end(); ++it_rid) {
            map<uint32_t, string>::iterator it_gname = m_role_names.find(*it_rid);
            it = m_roles.find(it_gname->second);
            it->second->list.erase(
                find(it->second->list.begin(), it->second->list.end(), iid));
        }
        delete item;
        item = 0;
        m_identities.erase(identity_name);
    }

    // update relation file
    bool ret = DumpIdentityNames(ptr_error_code);
    ret &= DelRoleWithoutLock(identity_name);
    ret &= DumpRelations(ptr_error_code);
    return ret;
}

bool RoleManager::QueryIdentityRoles(const char* identity_name, vector<string>* role_list,
                                     CA_ERROR_CODE* ptr_error_code) {
   CXThreadAutoLock auto_lock(&s_mutex);
    uint32_t identity_name_len = STRLEN(identity_name);
    if (!(identity_name_len < kMaxNameLen && identity_name_len > 0)) {
        LOG(ERROR) << "role " << identity_name << " is null or too long!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_PAPRAM_INVALID);
        return false;
    }

    role_list->clear();
    map<string, Item*>::iterator it_identities = m_identities.find(identity_name);
    if (it_identities == m_identities.end()) {
        LOG(ERROR) << "role " << identity_name << " not find!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_IDENTITY_NOT_EXIST_IN_MEM);
        return false;
    }
    Item* item = it_identities->second;
    map<uint32_t, string>::iterator it_role_names;
    vector<uint32_t>::iterator it_role_list = item->list.begin();
    for (; it_role_list != item->list.end(); ++ it_role_list) {
        it_role_names = m_role_names.find(*it_role_list);
        if (it_role_names != m_role_names.end()) {
            role_list->push_back(it_role_names->second);
        }
    }

    return true;
}

bool RoleManager::AddRole(const char* role_name, CA_ERROR_CODE* ptr_error_code) {
    CXThreadAutoLock auto_lock(&s_mutex);
    return AddRoleWithoutLock(role_name, ptr_error_code);
}

bool RoleManager::AddRoleWithoutLock(const char *role_name,
                                     CA_ERROR_CODE* ptr_error_code) {
    uint32_t role_name_len = STRLEN(role_name);
    if (!(role_name_len < kMaxNameLen && role_name_len > 0)) {
        LOG(ERROR) << "role " << role_name << " is null or too long!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_PAPRAM_INVALID);
        return false;
    }

    // exist?
    uint32_t rid = GetRoleIdByName(role_name);
    if (rid != 0) {
        LOG(ERROR) << "role " << role_name << " already exist!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_ADD_TWICE);
        return false;
    }

    if (m_max_role_id >= kMaxIdNum) {
        LOG(ERROR) << "Role id reach the max num : " << kMaxIdNum;
        return false;
    }
    // add to map
    ++m_max_role_id;
    rid = m_max_role_id;
    m_role_names.insert(pair<uint32_t, string>(rid, string(role_name)));

    Item* item = new Item;
    item->id = rid;
    m_roles.insert(pair<string, Item*>(role_name, item));

    // update local file
    bool ret = DumpRoleNames(ptr_error_code);
    return ret;
}

bool RoleManager::DelRole(const char* role_name, CA_ERROR_CODE* ptr_error_code) {
    CXThreadAutoLock auto_lock(&s_mutex);
    return DelRoleWithoutLock(role_name, ptr_error_code);
}

bool RoleManager::DelRoleWithoutLock(const char* role_name,
                                     CA_ERROR_CODE* ptr_error_code) {
    uint32_t role_name_len = STRLEN(role_name);
    if (!(role_name_len < kMaxNameLen && role_name_len > 0)) {
        LOG(ERROR) << "relation " << role_name << " is null or too long!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_PAPRAM_INVALID);
        return false;
    }

    // delete from map
    uint32_t rid = GetRoleIdByName(role_name);
    if (rid == 0) {
        LOG(ERROR) << "relation " << role_name << "does not exist!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_ROLE_NOT_EXIST_IN_MEM);
        return false;
    } else if (GetIdentityIdByName(role_name) != 0) {
        LOG(ERROR) << "relation " << role_name << " is a relation!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_PAPRAM_INVALID);
        return false;
    } else {
        m_role_names.erase(rid);
        map<string, Item*>::iterator it = m_roles.find(role_name);
        Item* item = it->second;
        for (vector<uint32_t>::iterator it_iid = item->list.begin();
            it_iid != item->list.end(); ++it_iid) {
            map<uint32_t, string>::iterator it_name = m_identity_names.find(*it_iid);
            it = m_identities.find(it_name->second);
            it->second->list.erase(find(it->second->list.begin(),
                                   it->second->list.end(),
                                   rid));
        }
        delete item;
        item = 0;
        m_roles.erase(role_name);
    }

    // update local file
    bool ret = DumpRoleNames(ptr_error_code);
    ret &= DumpRelations(ptr_error_code);
    return ret;
}

bool RoleManager::QueryRoleIdentities(const char* role_name, vector<string>* identity_list,
                                   CA_ERROR_CODE* ptr_error_code) {

    CXThreadAutoLock auto_lock(&s_mutex);

    uint32_t role_name_len = STRLEN(role_name);
    if (!(role_name_len < kMaxNameLen && role_name_len > 0)) {
        LOG(ERROR) << "relation " << role_name << " is null or too long!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_PAPRAM_INVALID);
        return false;
    }

    identity_list->clear();
    map<string, Item*>::iterator it_roles = m_roles.find(role_name);
    if (it_roles == m_roles.end()) {
        LOG(ERROR) << "relation " << role_name << " not find!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_ROLE_NOT_EXIST_IN_MEM);
        return false;
    }
    Item* item = it_roles->second;
    map<uint32_t, string>::iterator it_identity_names;
    vector<uint32_t>::iterator it_identity_list = item->list.begin();
    for (; it_identity_list != item->list.end(); ++ it_identity_list) {
        it_identity_names = m_identity_names.find(*it_identity_list);
        if (it_identity_names != m_identity_names.end()) {
            identity_list->push_back(it_identity_names->second);
        }
    }

    return true;
}

bool RoleManager::AddIdentityToRole(const char* role_name, const char* identity_name,
                                    CA_ERROR_CODE* ptr_error_code) {
    CXThreadAutoLock auto_lock(&s_mutex);
    return AddIdentityToRoleWithoutLock(role_name, identity_name, ptr_error_code);
}

bool RoleManager::AddIdentityToRoleWithoutLock(const char* role_name, const char* identity_name,
                                               CA_ERROR_CODE* ptr_error_code) {
    uint32_t identity_name_len = STRLEN(identity_name);
    uint32_t role_name_len = STRLEN(role_name);
    if (!(role_name_len > 0 && role_name_len < kMaxNameLen &&
         identity_name_len > 0 && identity_name_len < kMaxNameLen)) {
            LOG(ERROR) << identity_name << " or " << role_name << " is null or too long!";
            SET_ERRORCODE(ptr_error_code, ERROR_CA_PAPRAM_INVALID);
            return false;
    }

    // if relation has relation with the same name, add fail
    if (GetIdentityIdByName(role_name) != 0 && strcmp(role_name, identity_name)) {
        LOG(ERROR) << role_name << " has relation with the same name, add FAIL!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_PAPRAM_INVALID);
        return false;
    }

    if (!CheckRoleNum(identity_name)) {
        SET_ERRORCODE(ptr_error_code, ERROR_CA_HAS_MAX_ROLES);
        return false;
    }

    uint32_t rid = GetRoleIdByName(role_name);
    uint32_t iid = GetIdentityIdByName(identity_name);
    if (rid == 0 || iid == 0) {
        LOG(ERROR) << role_name << (rid == 0 ? " not" : "") << " exist!"
                   << identity_name  << (iid == 0 ? " not" : "") << " exist!";
        SET_ERRORCODE(ptr_error_code,  (iid == 0) ? ERROR_CA_IDENTITY_NOT_EXIST_IN_MEM :
                                                    ERROR_CA_ROLE_NOT_EXIST_IN_MEM);
        return false;
    }

    // add to maps
    map<string, Item*>::iterator it_roles = m_roles.find(role_name);
    map<string, Item*>::iterator it_identities = m_identities.find(identity_name);
    vector<uint32_t>::iterator it_list;
    VLOG(3) << "Add relation <" << role_name << "," << identity_name << "> to maps";
    if ((it_list = find(it_roles->second->list.begin(), it_roles->second->list.end(), iid))
        != it_roles->second->list.end()) {
        LOG(ERROR) << "Relation <" << role_name << ","
                   << identity_name << "> already exist!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_ADD_TWICE);
        return false;
    } else {
        it_roles->second->list.push_back(iid);
    }
    if ((it_list = find(it_identities->second->list.begin(), it_identities->second->list.end(), rid))
        != it_identities->second->list.end()) {
        LOG(ERROR) << "Relation <" << role_name << ","
                   << identity_name << "> already exist!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_ADD_TWICE);
        return false;
    } else {
        it_identities->second->list.push_back(rid);
    }

    // update local file
    bool ret = DumpRelations(ptr_error_code);
    return ret;
}

bool RoleManager::DelIdentityFromRole(const char* role_name, const char* identity_name,
                                      CA_ERROR_CODE* ptr_error_code) {
    CXThreadAutoLock auto_lock(&s_mutex);
    return DelIdentityFromRoleWithoutLock(role_name, identity_name, ptr_error_code);
}

bool RoleManager::DelIdentityFromRoleWithoutLock(const char* role_name, const char* identity_name,
                                                 CA_ERROR_CODE* ptr_error_code) {
    uint32_t identity_name_len = STRLEN(identity_name);
    uint32_t role_name_len = STRLEN(role_name);
    if (!(role_name_len > 0 && role_name_len < kMaxNameLen &&
        identity_name_len > 0 && identity_name_len < kMaxNameLen)) {
        LOG(ERROR) << identity_name << " or " << role_name << " is null or too long!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_PAPRAM_INVALID);
        return false;
    }

    if (strcmp(role_name, identity_name) == 0) {
        LOG(ERROR) << "can't del " << identity_name << "'s own role";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_PAPRAM_INVALID);
        return false;
    }

    // if role not exist, return false;
    uint32_t rid = GetRoleIdByName(role_name);
    uint32_t iid = GetIdentityIdByName(identity_name);
    if (rid == 0 || iid == 0) {
        LOG(ERROR) << role_name << (rid == 0 ? " not" : "") << " exist!"
                   << identity_name  << (iid == 0 ? " not" : "") << " exist!";
        SET_ERRORCODE(ptr_error_code, (iid == 0) ? ERROR_CA_IDENTITY_NOT_EXIST_IN_MEM :
                                                   ERROR_CA_ROLE_NOT_EXIST_IN_MEM);
        return false;
    }

    // remove from maps
    map<string, Item*>::iterator it_roles = m_roles.find(role_name);
    map<string, Item*>::iterator it_identities = m_identities.find(identity_name);
    if (it_roles == m_roles.end() || it_identities == m_identities.end()) {
        LOG(ERROR) << "can't find role " << role_name
                   << " or role " << identity_name << "in maps";
        SET_ERRORCODE(ptr_error_code, (it_roles == m_roles.end()) ?
                      ERROR_CA_ROLE_NOT_EXIST_IN_MEM : ERROR_CA_IDENTITY_NOT_EXIST_IN_MEM);
        return false;
    }
    vector<uint32_t>::iterator it_iid = find(it_roles->second->list.begin(),
                                            it_roles->second->list.end(), iid);
    vector<uint32_t>::iterator it_rid = find(it_identities->second->list.begin(),
                                             it_identities->second->list.end(), rid);
    if (it_rid != it_identities->second->list.end() &&
        it_iid != it_roles->second->list.end()) {
        it_roles->second->list.erase(it_iid);
        it_identities->second->list.erase(it_rid);
    } else {
        LOG(ERROR) << "Relation <" << role_name << " , " << identity_name
                   << "> not exist!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_RELATION_NOT_EXIST_IN_MEM);
        return false;
    }

    // update local file
    bool ret = DumpRelations(ptr_error_code);
    return ret;
}

bool RoleManager::CheckRoleNum(const char* identity_name) {
    map<string, Item*>::iterator it = m_identities.find(identity_name);
    if (it != m_identities.end() && it->second->list.size() >= kMaxRoles) {
        LOG(ERROR) << "Get max role num " << kMaxRoles
                   << "Please quit some roles!";
        return false;
    }
    return true;
}

uint32_t RoleManager::GetIdentityIdByName(const char* identity_name) {
    map<string, Item*>::iterator it = m_identities.find(identity_name);
    if (it == m_identities.end())
        return 0;

    Item* item = it->second;
    if (item->valid) {
        return item->id;
    } else {
        return 0;
    }
}

const char* RoleManager::GetIdentityNameById(uint32_t identity_id) {
    map<uint32_t, string>::iterator it_names = m_identity_names.find(identity_id);
    if (it_names != m_identity_names.end()) {
        return it_names->second.c_str();
    } else 
        return NULL;
}

uint32_t RoleManager::GetRoleIdByName(const char* role_name) {
    map<string, Item*>::iterator it = m_roles.find(role_name);
    if (it == m_roles.end())
        return 0;

    Item* item = it->second;
    if (item->valid) {
        return item->id;
    } else {
        return 0;
    }
}

const char* RoleManager::GetRoleNameById(uint32_t role_id) {
    map<uint32_t, string>::iterator it_names = m_role_names.find(role_id);
    if (it_names != m_role_names.end()) {
        return it_names->second.c_str();
    } else 
        return NULL;
}

bool RoleManager::LoadRoleNames() {
    std::ifstream role_stream(m_role_filename, std::ios::in | std::ios::binary);
    RecordReader record_reader(&role_stream);
    RoleList list;
    record_reader.ReadMessage(&list);

    // read role message.
    for (int32_t i = 0; i < list.role_size(); ++i) {
        IdNameRecord record = list.role(i);
        // update m_role_names
        m_role_names.insert(pair<uint32_t, string>(record.id(), record.name()));
        // update m_roles
        Item* item = new Item;
        item->id = record.id();
        m_roles.insert(pair<string, Item*>(record.name(), item));
        // update m_max_role_id
        m_max_role_id = record.id() > m_max_role_id ? record.id() : m_max_role_id;
    }

    if (m_max_role_id == 0) {
        LOG(INFO) << "Empty file " << m_role_filename;
    }

    return true;
}

bool RoleManager::LoadIdentityNames() {
    std::ifstream identity_stream(m_identity_filename, std::ios::in | std::ios::binary);
    RecordReader record_reader(&identity_stream);
    IdentityList list;
    record_reader.ReadMessage(&list);
    
    // read identity message.
    for (int32_t i = 0; i < list.identity_size(); ++i) {
        IdNameRecord record = list.identity(i);
        // update m_identity_names
        m_identity_names.insert(pair<uint32_t, string>(record.id(), record.name()));
        // update m_identities
        Item* item = new Item;
        item->id = record.id();
        m_identities.insert(pair<string, Item*>(record.name(), item));
        // update m_max_identity_id
        m_max_identity_id = record.id() > m_max_identity_id ? record.id() : m_max_identity_id;
    }

    if (m_max_identity_id == 0) {
        LOG(INFO) << "Empty file " << m_identity_filename;
    }

    return true;
}

bool RoleManager::LoadRelations() {
    std::ifstream relation_stream(m_related_filename, std::ios::in | std::ios::binary);
    RecordReader record_reader(&relation_stream);
    RelationList list;
    record_reader.ReadMessage(&list);

    // read relation message.
    for (int32_t i = 0; i < list.relation_size(); ++i) {
        RelationRecord record = list.relation(i);
        uint32_t iid = record.iid();
        uint32_t rid = record.rid();

        // update m_identities
        map<uint32_t, string>::iterator it_names;
        if ((it_names = m_identity_names.find(iid)) != m_identity_names.end()) {
            map<string, Item*>::iterator it_identities = m_identities.find(it_names->second);
            vector<uint32_t>::iterator it_rid = find(it_identities->second->list.begin(),
                it_identities->second->list.end(), rid);
            if (it_identities != m_identities.end() && it_rid == it_identities->second->list.end()) {
                it_identities->second->list.push_back(rid);
            }
        } else {
            LOG(ERROR) << "identity id " << iid << " not in identity map";
        }

        // update m_roles
        if ((it_names = m_role_names.find(rid)) != m_role_names.end()) {
            map<string, Item*>::iterator it_roles = m_roles.find(it_names->second);
            vector<uint32_t>::iterator it_iid = find(it_roles->second->list.begin(),
                it_roles->second->list.end(), iid);
            if (it_roles != m_roles.end() && it_iid == it_roles->second->list.end()) {
                it_roles->second->list.push_back(iid);
            }
        } else {
            LOG(ERROR) << "role id " << rid << " not in role map";
        }
    }

    if (m_max_identity_id == 0) {
        LOG(INFO) << "Empty file " << m_related_filename;
    }

    return true;
}

void RoleManager::PrintAllIdentities(vector<string>* identity_list) {
     CXThreadAutoLock auto_lock(&s_mutex);

     identity_list->clear();
     map<uint32_t, string>::iterator it_names = m_identity_names.begin();
     for ( ; it_names != m_identity_names.end(); ++it_names) {
         identity_list->push_back(it_names->second);
     }
}

void RoleManager::PrintAllRoles(vector<string>* role_list) {
    CXThreadAutoLock auto_lock(&s_mutex);
    role_list->clear();
    map<uint32_t, string>::iterator it_names = m_role_names.begin();
      for ( ; it_names != m_role_names.end(); ++it_names) {
        role_list->push_back(it_names->second);
    }
}

bool RoleManager::VerifyIdentity(const char* role_name, const char* identity_name,
                                 CA_ERROR_CODE* ptr_error_code) {
    uint32_t identity_name_len = STRLEN(identity_name);
    uint32_t role_name_len = STRLEN(role_name);
    if (!(role_name && role_name_len < kMaxNameLen &&
        identity_name && identity_name_len < kMaxNameLen)) {
            LOG(ERROR) << identity_name << " or " << role_name << " is null or too long!";
            SET_ERRORCODE(ptr_error_code, ERROR_CA_PAPRAM_INVALID);
            return false;
    }

    CXThreadAutoLock auto_lock(&s_mutex);
    uint32_t rid = GetRoleIdByName(role_name);
    uint32_t iid = GetIdentityIdByName(identity_name);
    if (rid == 0 || iid == 0) {
        LOG(ERROR) << role_name << (rid == 0 ? " not" : "") << " exist!"
            << identity_name  << (iid == 0 ? " not" : "") << " exist!";
        SET_ERRORCODE(ptr_error_code,  (iid == 0) ? ERROR_CA_IDENTITY_NOT_EXIST_IN_MEM :
                                                    ERROR_CA_ROLE_NOT_EXIST_IN_MEM);
        return false;
    }

    map<string, Item*>::iterator it_identities = m_identities.find(identity_name);
    if (it_identities == m_identities.end()) {
        LOG(ERROR) << "relation " << identity_name << " not find!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_IDENTITY_NOT_EXIST_IN_MEM);
        return false;
    }
    Item* item = it_identities->second;
    vector<uint32_t>::iterator it_role_list = item->list.begin();
    for (; it_role_list != item->list.end(); ++ it_role_list) {
        if (rid == *it_role_list)
            return true;
    }

    SET_ERRORCODE(ptr_error_code, ERROR_CA_RELATION_NOT_EXIST_IN_MEM);
    return false;
}

bool RoleManager::DumpIdentityNames(CA_ERROR_CODE* ptr_error_code) {
    // Fill in data
    map<uint32_t, string>::iterator it = m_identity_names.begin();
    IdentityList list;
    
    for ( ; it != m_identity_names.end(); ++it) {
        IdNameRecord* record = list.add_identity();
        record->set_id(it->first);
        record->set_name(it->second);
        record->set_valid(true);
    }

    // save old file, add suffix _time
    SaveOldFile(m_identity_filename, ptr_error_code);

    // write to file
    scoped_ptr<RecordWriter> record_writer;
    record_writer.reset(
        new RecordWriter(
        new std::ofstream(
        m_identity_filename,
        std::ios::out | std::ios::binary),
        RecordWriterOptions(RecordWriterOptions::OWN_STREAM)));
    if (!record_writer->WriteMessage(list)) {
        LOG(ERROR) << "Write list failed : " << m_identity_filename;
        SET_ERRORCODE(ptr_error_code, ERROR_CA_WRITE_FILE);
        return false;
    }
    if (!record_writer->Flush()) {
        LOG(ERROR) << "Flush list failed : " << m_identity_filename;
        SET_ERRORCODE(ptr_error_code, ERROR_CA_FLUSH_FILE);
        return false;
    }

    return true;
}


bool RoleManager::DumpRoleNames(CA_ERROR_CODE* ptr_error_code) {
    // Fill in data
    map<uint32_t, string>::iterator it = m_role_names.begin();
    RoleList list;
    for ( ; it != m_role_names.end(); ++it) {
        IdNameRecord* record = list.add_role();
        record->set_id(it->first);
        record->set_name(it->second);
        record->set_valid(true);
    }

    // save old file, add suffix _time
    SaveOldFile(m_role_filename, ptr_error_code);

    // write to file
    scoped_ptr<RecordWriter> record_writer;
    record_writer.reset(
        new RecordWriter(
        new std::ofstream(
        m_role_filename,
        std::ios::out | std::ios::binary),
        RecordWriterOptions(RecordWriterOptions::OWN_STREAM)));
    if (!record_writer->WriteMessage(list)) {
        LOG(ERROR) << "Write list failed : " << m_role_filename;
        SET_ERRORCODE(ptr_error_code, ERROR_CA_WRITE_FILE);
        return false;
    }
    if (!record_writer->Flush()) {
        LOG(ERROR) << "Flush list failed : " << m_role_filename;
        SET_ERRORCODE(ptr_error_code, ERROR_CA_FLUSH_FILE);
        return false;
    }

    return true;
}

bool RoleManager::DumpRelations(CA_ERROR_CODE* ptr_error_code) {
    // Fill in data
    map<string, Item*>::iterator it = m_roles.begin();
    RelationList list;
    for ( ; it != m_roles.end(); ++it) {
        for (vector<uint32_t>::iterator it_list = it->second->list.begin();
            it_list != it->second->list.end(); ++it_list) {
            RelationRecord* record = list.add_relation();
            record->set_rid(it->second->id);
            record->set_iid(*it_list);
            record->set_valid(true);
        }
    }

    // save old file, add suffix _time
    SaveOldFile(m_related_filename, ptr_error_code);

    // write to file
    scoped_ptr<RecordWriter> record_writer;
    record_writer.reset(
        new RecordWriter(
        new std::ofstream(
        m_related_filename,
        std::ios::out | std::ios::binary),
        RecordWriterOptions(RecordWriterOptions::OWN_STREAM)));
    if (!record_writer->WriteMessage(list)) {
        LOG(ERROR) << "Write list failed : " << m_related_filename;
        SET_ERRORCODE(ptr_error_code, ERROR_CA_WRITE_FILE);
        return false;
    }
    if (!record_writer->Flush()) {
        LOG(ERROR) << "Flush list failed : " << m_related_filename;
        SET_ERRORCODE(ptr_error_code, ERROR_CA_FLUSH_FILE);
        return false;
    }

    return true;
}

bool RoleManager::SaveOldFile(const char* filename, CA_ERROR_CODE* ptr_error_code) {
    // move filename to filename+_time
    string last_list_name = filename;
    last_list_name += "_" + TimeUtils::GetCurTime();
    bool success = true;
#ifdef WIN32
    if (MoveFileEx(filename, last_list_name.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == 0)
        success = false;
#else
    if (rename(filename, last_list_name.c_str()))
        success = false;
#endif
    if (!success)
        LOG(INFO) << "File  " << filename << " not Exist!";

    return true;
}

} // namespace ca
