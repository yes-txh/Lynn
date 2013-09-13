// lite_mysql_wrapper.cpp

#include "common/baselib/svrpublib/server_publib.h"

#if defined(_MYSQL_SUPPORT)
// ////////////////////////////////////////////////////
// ////////////////////////////////////////////////////
CLiteMySQLWrapper:: CLiteMySQLWrapper() {
    memset(m_fileds_cols_name, 0, sizeof(m_fileds_cols_name));

    mysql_init(&m_mysql_connection);

    m_reconn_count = 0;

    m_db_port = 3306;
    m_is_connected = false;
    m_fields_count = 0;
    m_mysql_result = NULL;
    m_mysql_rows = 0;

    memset(m_default_charset, 0, sizeof(m_default_charset));
    safe_snprintf(m_default_charset, sizeof(m_default_charset), "%s", "utf8");
    memset(m_host, 0, sizeof(m_host));
    memset(m_user, 0, sizeof(m_user));
    memset(m_password, 0, sizeof(m_password));
    memset(m_db_name, 0, sizeof(m_db_name));
    m_is_field_index_initialized = false;
}

CLiteMySQLWrapper:: ~CLiteMySQLWrapper() {
    if (m_is_connected) {
        FreeResult();
        mysql_close(&m_mysql_connection);
        m_is_connected = false;
    }
}

bool CLiteMySQLWrapper::StripField(unsigned char* to_buff, uint32_t buff_len,
                                   const char* from, uint32_t len) {
    bool b = false;
    if (from) {
        char*    tostr  = new char[2*len+1];
        if (tostr) {
            memset(tostr, 0, 2*len+1);
            mysql_real_escape_string(&m_mysql_connection, tostr, from, len);
            uint32_t uRetStrLen = STRLEN(tostr);
            if (to_buff && uRetStrLen && buff_len >= uRetStrLen) {
                memcpy(to_buff, tostr, uRetStrLen);
                b = true;
                if (buff_len>uRetStrLen)
                    to_buff[uRetStrLen] = 0;
            }
            delete []tostr;
            tostr = 0;
        }
    }
    return b;
}

bool CLiteMySQLWrapper:: Connect(const char* host, const char* user,
                                 const char* password,
                                 const char* database ,
                                 uint32_t flag ,
                                 int32_t db_port) {
    safe_snprintf(m_host, sizeof(m_host), "%s", host);
    safe_snprintf(m_user, sizeof(m_user), "%s" user);
    safe_snprintf(m_password, sizeof(m_password), "%s", password);
    safe_snprintf(m_db_name, sizeof(m_db_name), "%s", database);
    m_client_flag = flag;
    m_db_port = db_port;
    return EstablishConnect();
}

void CLiteMySQLWrapper::SetDefaultCharset(const char* charset) {
    if (charset) {
        memset(m_default_charset, 0, sizeof(m_default_charset));
        safe_snprintf(m_default_charset, sizeof(m_default_charset),
                      "%s", charset);
    }
}

bool CLiteMySQLWrapper::EstablishConnect() {
    if (!m_is_connected) {
        // close connection
        mysql_close(&m_mysql_connection);
        m_is_connected = false;

        uint32_t time_out = 28800*3*365; // default Value(365*24 hours)
        char setting_sz[128];
        safe_snprintf(setting_sz, sizeof(setting_sz),
                      "set @@interactive_timeout = %u;", time_out);

        mysql_options(&m_mysql_connection, MYSQL_INIT_COMMAND , setting_sz);

        // set reconnect
        char value = 1;
        mysql_options(&m_mysql_connection, MYSQL_OPT_RECONNECT,
                      reinterpret_cast<char *>(&value));

        char* pdbname = 0;
        if (m_db_name[0] != '\0')
            pdbname = m_db_name;

        // connect
        if (mysql_real_connect(&m_mysql_connection,
                               m_host, m_user, m_password, pdbname, m_db_port,
                               NULL, m_client_flag) == NULL) {
            VLOG(3) << "<MySQL-WARNING> DB Connection fail:h:" <<
                      m_host << ", db:" << pdbname << ", u:" << m_user << ", " <<
                      mysql_error(&m_mysql_connection) << "</MySQL-WARNING>";
        } else {
            char sz[256];
            safe_snprintf(sz, sizeof(sz), "set names '%s'",
                          m_default_charset);

            mysql_query(&m_mysql_connection, reinterpret_cast<char*>(sz));

            m_is_connected = true;
            m_reconn_count++;
        }
    }
    return m_is_connected;
}

//
// CR_COMMANDS_OUT_OF_SYNC: Commands were executed in an improper order.
// CR_SERVER_GONE_ERROR:    The MySQL server has gone away.
// CR_SERVER_LOST:          The connection to the server
//                          was lost during the query.
// CR_UNKNOWN_ERROR:        An unknown error occurred.
//
bool CLiteMySQLWrapper::Query(const char* sql_string) {
    bool b = false;
    if (sql_string) {
        EstablishConnect();
        int32_t ret = mysql_query(&m_mysql_connection, sql_string);
        if (ret!= 0) {
            ret = mysql_errno(&m_mysql_connection);
            VLOG(3) << "<MySQL-WARNING> * DB Query fail:" << mysql_error(&m_mysql_connection) <<
                      "MySQL Error Number:" << ret <<
                      "Maybe no such record[QUERY], "
                      "or record already exist[INSERT].</MySQL-WARNING>";

            if (CR_SERVER_GONE_ERROR == ret ||
                    CR_SERVER_LOST == ret ||
                    CR_UNKNOWN_ERROR == ret ) {
                VLOG(3) << "<MySQL-WARNING> try reconnect to database:" <<
                          m_host << ":" << m_db_name << "</MySQL-WARNING>";

                m_is_connected = false;
                EstablishConnect();
                ret = mysql_query(&m_mysql_connection, sql_string);
                if ( ret != 0 ) {
                    VLOG(3) << "<MySQL-WARNING> * DB query 2nd fail:" <<
                              mysql_error(&m_mysql_connection) << "</MySQL-WARNING>";
                    XSleep(1);
                } else
                    b = true;
            }
        } else
            b = true;
    }
    return b;
}

bool CLiteMySQLWrapper::Query(const char* sql_string, uint32_t query_len) {
    bool b = false;
    if (sql_string) {
        EstablishConnect();
        int32_t ret = mysql_real_query(&m_mysql_connection,
                                       sql_string, query_len);
        if ( ret!= 0) {
            VLOG(3) << "<MySQL-WARNING> DB real query fail:" <<
                      mysql_error(&m_mysql_connection) << "</MySQL-WARNING>";
            ret = mysql_errno(&m_mysql_connection);
            if ( CR_SERVER_GONE_ERROR == ret ||
                    CR_SERVER_LOST == ret ||
                    CR_UNKNOWN_ERROR == ret ) {
                m_is_connected = false;
                EstablishConnect();
                ret = mysql_real_query(&m_mysql_connection,
                                       sql_string, query_len);
            }

            if ( ret != 0 ) {
                VLOG(3) << "<MySQL-WARNING> DB real query 2nd fail:" <<
                          mysql_error(&m_mysql_connection) << "</MySQL-WARNING>";
            } else
                b = true;
        } else
            b = true;
    }
    return b;
}

bool CLiteMySQLWrapper:: FreeResult() {
    if (m_mysql_result != NULL) {
        mysql_free_result(m_mysql_result);
        m_mysql_result = NULL;
    }

    m_is_field_index_initialized = false;
    m_fields_count = 0;
    return true;
}

bool CLiteMySQLWrapper:: StoreResult() {
    bool b = false;
    FreeResult();
    m_mysql_result = mysql_store_result(&m_mysql_connection);
    if (m_mysql_result == NULL) {
        VLOG(3) << "<***MySQL ERROR***> DBSaveResult ERROR:" << mysql_error(&m_mysql_connection);
        m_fields_count = 0;
    } else {
        m_fields_count = mysql_num_fields(m_mysql_result);
        b = true;
    }
    return b;
}

char** CLiteMySQLWrapper:: FetchRow() {
    if (m_mysql_result == NULL)
        StoreResult();

    m_mysql_rows = 0;
    if (m_mysql_result)
        m_mysql_rows = mysql_fetch_row(m_mysql_result);
    return m_mysql_rows;
}

bool CLiteMySQLWrapper::InitFieldName() {
    if (!m_is_field_index_initialized && m_mysql_result) {
        uint32_t    i = 0;
        MYSQL_FIELD* fields = mysql_fetch_fields(m_mysql_result);
        for (i = 0;
                i < m_fields_count && i < MYSQL_QQ_MAX_COLUMNS && fields;
                i++) {
            if (MYSQL_QQ_MAX_COL_NAME_LEN>fields[i].name_length) {
                memcpy(m_fields_cols_name[i],
                       fields[i].name,
                       fields[i].name_length);
                (m_fields_cols_name[i])[fields[i].name_length] = 0;
            } else
                (m_fields_cols_name[i])[0] = 0;
        }
        m_is_field_index_initialized = true;
    }
    return true;
}

//
//  返回结果的行数
//
uint32_t CLiteMySQLWrapper:: GetAffectedRows() {
    my_ulonglong iNumRows = mysql_affected_rows(&m_mysql_connection);
    return (uint32_t)iNumRows;
}

//
//  按照字段名取回当前行的结果
//
char* CLiteMySQLWrapper::GetFieldByColName(const char* field_name, bool case ) {
    InitFieldName();
    uint32_t i = 0;
    for (i = 0; i < m_fields_count; i++) {
        if (case) {
            if (strcmp(field_name, m_fields_cols_name[i]) == 0)
                return GetFieldByIndex(i);
        } else {
            if (stricmp(field_name, m_fields_cols_name[i]) == 0)
                return GetFieldByIndex(i);
        }
    }

    VLOG(3) << "<***MySQL ERROR***> no such field:" << m_fields_cols_name[i];
    return NULL;
}

//
//  按照字段索引取回当前行的结果
//  iField:zero based
//
char* CLiteMySQLWrapper::GetFieldByIndex(uint32_t field_index) {
    // you must call FetchRow before get field value
    if (field_index >= m_fields_count ||
            !m_mysql_rows)
        return NULL;

    return m_mysql_rows[field_index];
}

//
//  返回结果的列数
//
uint32_t CLiteMySQLWrapper::GetFieldsCount() const {
    return m_fields_count;
}

//
//  Class CLiteMySQLManage
//
CLiteMySQLManage::CLiteMySQLManage() {
    metset(m_long_sql, 0, sizeof(m_long_sql));
    memset(m_sql_query, 0, sizeof(m_sql_query));

    m_lite_mysql_obj = 0;
    m_is_inside_create_lite_mysql_obj = false;
}

CLiteMySQLManage::CLiteMySQLManage(CLiteMySQLWrapper* pLiteMySQL) {
    metset(m_long_sql, 0, sizeof(m_long_sql));
    memset(m_sql_query, 0, sizeof(m_sql_query));

    m_lite_mysql_obj = pLiteMySQL; //  Attach lite MySQL object
    m_is_inside_create_lite_mysql_obj = false;
}

CLiteMySQLManage::CLiteMySQLManage(MySQLAccountInfo* sql_account_info) {
    metset(m_long_sql, 0, sizeof(m_long_sql));
    memset(m_sql_query, 0, sizeof(m_sql_query));

    m_lite_mysql_obj = 0;
    if (sql_account_info) {
        memcpy((unsigned char*)&m_mysql_account_info,
               (unsigned char*)sql_account_info,
               sizeof(MySQLAccountInfo));
    }
    m_is_inside_create_lite_mysql_obj = false;
}

CLiteMySQLManage::CLiteMySQLManage(MySQLAccountInfo sql_account_info) {
    metset(m_long_sql, 0, sizeof(m_long_sql));
    memset(m_sql_query, 0, sizeof(m_sql_query));

    m_lite_mysql_obj = 0;
    memcpy((unsigned char*)&m_mysql_account_info,
           (unsigned char*)&sql_account_info,
           sizeof(MySQLAccountInfo));

    m_is_inside_create_lite_mysql_obj = false;
}

bool    CLiteMySQLManage::InitLiteMySQL(MySQLAccountInfo* sql_account_info) {
    if (sql_account_info) {
        memcpy((unsigned char*)&m_mysql_account_info,
               (unsigned char*)sql_account_info,
               sizeof(MySQLAccountInfo));
        return InitLiteMySQL();
    }
    return false;
}

bool    CLiteMySQLManage::InitLiteMySQL() {
    bool b = false;
    if (!m_lite_mysql_obj) {
        m_lite_mysql_obj = new CLiteMySQLWrapper;
        if (m_lite_mysql_obj) { //  Maybe out of memory
            b = m_lite_mysql_obj->Connect(m_mysql_account_info._host,
                                          m_mysql_account_info._user,
                                          m_mysql_account_info._password,
                                          m_mysql_account_info._database,
                                          m_mysql_account_info._flag,
                                          m_mysql_account_info._port);
            if (b) {
                m_is_inside_create_lite_mysql_obj = true;
            } else {
                delete m_lite_mysql_obj;
                m_lite_mysql_obj = 0;
                m_is_inside_create_lite_mysql_obj = false;
            }
        }
    }
    return b;
}

void    CLiteMySQLManage::UnInitLiteMySQL() {
    if (m_is_inside_create_lite_mysql_obj) {
        delete m_lite_mysql_obj;
        m_lite_mysql_obj = 0;

        m_is_inside_create_lite_mysql_obj = false;
    }
}

void CLiteMySQLManage::ResetSQLQueryString() {
    memset(m_sql_query, 0, MAX_QUERY_SQL_LENGTH);
}

void CLiteMySQLManage::ResetLongSQLString() {
    memset(m_long_sql, 0, MAX_LONG_SQL_LENGTH);
}

CLiteMySQLWrapper*    CLiteMySQLManage::GetMySQLWrapperObject() {
    return m_lite_mysql_obj;
}

#else
#endif // defined(_MYSQL_SUPPORT)

