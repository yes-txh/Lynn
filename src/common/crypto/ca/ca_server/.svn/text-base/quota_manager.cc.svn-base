#include "common/file/recordio/recordio.h" 
#include "common/system/time/time_utils.hpp"
#include "common/crypto/ca/ca_public/ca_struct.h"
#include "common/crypto/ca/ca_server/quota_manager.h"
#include "common/crypto/ca/ca_server/role_manager.h"

DEFINE_string(ca_rpc_addr, "172.24.28.193:10090", "CA rpc address");
DEFINE_uint64(ca_rpc_thread_num, 5, "CA rpc thread num");

namespace ca {

QuotaManager* QuotaManager::s_quota_manager = NULL;
scoped_ptr<RWLock> QuotaManager::m_rwlock;

QuotaManager::QuotaManager() {
    // set role dir
    char module_dir[kMaxDirLen] = {0};
    GetModuleFileName(NULL, module_dir, sizeof(module_dir));
    char* p = strrchr(module_dir, '/');
    if (!p)
        p = strrchr(module_dir, '\\');
    CHECK(p);
    ++p;
    CHECK_GT(sizeof(module_dir) - 1, STRLEN(module_dir) + STRLEN(kCaDir));
    safe_snprintf(p, sizeof(module_dir), "%s", kCaDir);

    // init filename
    safe_snprintf(m_quota_filename, kMaxDirLen, "%s%s", module_dir, kQuotaFileName);
}

bool QuotaManager::LoadConfig() {
    m_netframe.reset(new netframe::NetFrame(FLAGS_ca_rpc_thread_num));
    m_http_server.reset(new rpc::HttpServer(m_netframe.get()));
    m_rpc_service.reset(new rpc::RpcService(m_http_server.get()));
    m_rpc_service->RegisterService(this);
    CHECK(m_http_server->StartServer(FLAGS_ca_rpc_addr))
        << "fail to start HttpServer for ca rpc.";
    bool ret = LoadQuota();
    CHECK(ret);
    return ret;
}

bool QuotaManager::ReloadConfig() {
    RWLock::WriterLocker locker(*m_rwlock);
    m_roles_quota.clear();
    return LoadQuota();
}

void QuotaManager::CleanConfig() {
    m_roles_quota.clear();
}

QuotaManager::~QuotaManager() {
    CleanConfig();
}

bool QuotaManager::LoadQuota() {
    std::ifstream quota_stream(m_quota_filename, std::ios::in | std::ios::binary);
    RecordReader record_reader(&quota_stream);
    QuotaList list;
    record_reader.ReadMessage(&list);

    // read identity message.
    for (int32_t i = 0; i < list.quota_size(); ++i) {
        const ::ca::QuotaStorageRecord record = list.quota(i);
        // update m_roles_quota
        Quota quota;
        quota.num_chunks = record.num_chunks();
        quota.num_files = record.num_files();
        quota.num_directories = record.num_dirs();
        const char* name = RoleManager::GetInstance()->GetRoleNameById(record.rid());
        if (name)
            m_roles_quota.insert(
                pair<RoleCluster, Quota>(RoleCluster(name, record.cluster()), quota));
    }

    return true;
}

bool QuotaManager::SetRoleQuota(const char* role, const char* cluster, const Quota quota,
                                CA_ERROR_CODE* ptr_error_code) {
    uint32_t id = 0;
    if (!IsValidRole(role, &id, ptr_error_code)) return false;
    if (!cluster || strcmp(cluster, "") == 0) {
        LOG(ERROR) << "cluster is null!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_PAPRAM_INVALID);
        return false;
    }

    RWLock::WriterLocker locker(*m_rwlock);
    std::map<RoleCluster, Quota>::iterator it = m_roles_quota.find(RoleCluster(role, cluster));
    if (it != m_roles_quota.end()) {
        it->second.num_chunks = quota.num_chunks;
        it->second.num_files = quota.num_files;
        it->second.num_directories = quota.num_directories;
    } else {
        m_roles_quota.insert(std::pair<RoleCluster, Quota>(RoleCluster(role, cluster), quota));
    }

    // todo:可以考虑隔一段时间dump一次
    return DumpQuota(ptr_error_code);
}

bool QuotaManager::QueryRoleQuota(const char* role, const char* cluster, Quota* quota,
                                  CA_ERROR_CODE* ptr_error_code) {
    if (!IsValidRole(role, 0, ptr_error_code)) return false;
    if (!cluster) {
        LOG(ERROR) << "cluster is null!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_PAPRAM_INVALID);
        return false;
    }

    RWLock::ReaderLocker locker(*m_rwlock);
    std::map<RoleCluster, Quota>::iterator it = m_roles_quota.find(RoleCluster(role, cluster));
    if (it != m_roles_quota.end()) {
        quota->num_chunks = it->second.num_chunks;
        quota->num_files = it->second.num_files;
        quota->num_directories = it->second.num_directories;
        return true;
    } else {
        LOG(ERROR) << "role " << role << "does not exist!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_ROLE_NOT_EXIST_IN_MEM);
    }
    return false;
}

void QuotaManager::ListAllRolesQuota(map<RoleCluster, Quota>* roles_quota,
                                     CA_ERROR_CODE* ptr_error_code) {
    RWLock::ReaderLocker locker(*m_rwlock);
    std::map<RoleCluster, Quota>::iterator it = m_roles_quota.begin();
    for (; it != m_roles_quota.end(); ++it)
        roles_quota->insert(pair<RoleCluster, Quota>(it->first, it->second));
}

bool QuotaManager::IsValidRole(const char* role, uint32_t* id, CA_ERROR_CODE* ptr_error_code) {
    // check parameter
    uint32_t role_len = STRLEN(role);
    if (!(role_len < kMaxNameLen && role_len > 0)) {
        LOG(ERROR) << "role " << role << " is null or too long!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_PAPRAM_INVALID);
        return false;
    }

    // find role in RoleManager
    uint32_t r_id = RoleManager::GetInstance()->GetRoleIdByName(role);
    if (r_id == 0) {
        LOG(ERROR) << "role " << role << " can't find in role manager!";
        SET_ERRORCODE(ptr_error_code, ERROR_CA_ROLE_NOT_EXIST_IN_MEM);
        return false;
    }

    if (id != 0)
        *id = r_id;

    return true;
}

bool QuotaManager::DumpQuota(CA_ERROR_CODE* ptr_error_code) {
    // Fill in data
    map<RoleCluster, Quota>::iterator it = m_roles_quota.begin();
    QuotaList list;
    for ( ; it != m_roles_quota.end(); ++it) {
        uint32_t rid = RoleManager::GetInstance()->GetRoleIdByName(it->first.first.c_str());
        if (rid == 0)
            continue;
        ::ca::QuotaStorageRecord* record = list.add_quota();
        record->set_rid(rid);
        record->set_cluster(it->first.second);
        record->set_num_chunks(it->second.num_chunks);
        record->set_num_files(it->second.num_files);
        record->set_num_dirs(it->second.num_directories);
    }

    // move m_quota_filename to m_quota_filename+_time
    string last_list_name = m_quota_filename;
    last_list_name += "_" + TimeUtils::GetCurTime();
    bool success = true;
#ifdef WIN32
    if (MoveFileEx(m_quota_filename, last_list_name.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == 0)
        success = false;
#else
    if (rename(m_quota_filename, last_list_name.c_str()))
        success = false;
#endif
    if (!success)
        LOG(INFO) << "File  " << m_quota_filename << " not Exist!";

    // write to file
    scoped_ptr<RecordWriter> record_writer;
    record_writer.reset(
        new RecordWriter(
        new std::ofstream(
        m_quota_filename,
        std::ios::out | std::ios::binary),
        RecordWriterOptions(RecordWriterOptions::OWN_STREAM)));
    if (!record_writer->WriteMessage(list)) {
        LOG(ERROR) << "Write list failed : " << m_quota_filename;
        SET_ERRORCODE(ptr_error_code, ERROR_CA_WRITE_FILE);
        return false;
    }
    if (!record_writer->Flush()) {
        LOG(ERROR) << "Flush list failed : " << m_quota_filename;
        SET_ERRORCODE(ptr_error_code, ERROR_CA_FLUSH_FILE);
        return false;
    }

    return true;
}

void QuotaManager::GetQuota(
    ::google::protobuf::RpcController* controller,
    const ::ca::GetQuotaRequest* request,
    ::ca::GetQuotaResponse* response,
    ::google::protobuf::Closure* done) {
    std::string cluster_identifier = request->cluster_identifier();
    // undo : if request contains rolename, return only one record

    RWLock::ReaderLocker locker(*m_rwlock);
    std::map<RoleCluster, Quota>::iterator it = m_roles_quota.begin();
    uint32_t record_count = 0;
    for (; it != m_roles_quota.end(); ++it) {
        if (cluster_identifier == it->first.second) {
            ::ca::QuotaRecord* quota_record = response->add_quota_record();
            quota_record->set_role_name(it->first.first);
            quota_record->set_chunks_count(it->second.num_chunks);
            quota_record->set_files_count(it->second.num_files);
            quota_record->set_directories_count(it->second.num_directories);
            ++record_count;
        }
    }

    if (record_count == 0) {
        response->set_error_code(ERROR_CA_CLUSTER_NOT_EXIST);
    } else {
        response->set_error_code(ERROR_CA_OK);
    }
    
    done->Run();
}

} // namespace ca
