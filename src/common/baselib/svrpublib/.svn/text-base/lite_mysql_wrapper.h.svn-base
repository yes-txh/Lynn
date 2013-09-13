// lite_mysql_wrapper.h
// wookin 修改于2006.06.23
//
// 进行了如下优化:
// 1：提高了性能，只缓冲一次列名
//
// 2：尽量不要用字段名查询，直接用字段index效率会高
//
// 3：连接超时后重连
//
// //////////////////////////////////////////////////

#ifndef COMMON_BASELIB_SVRPUBLIB_LITE_MYSQL_WRAPPER_H_
#define COMMON_BASELIB_SVRPUBLIB_LITE_MYSQL_WRAPPER_H_

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

#if defined(_MYSQL_SUPPORT)
class CLiteMySQLWrapper {
#define MYSQL_QQ_MAX_COLUMNS         64
#define MYSQL_QQ_MAX_COL_NAME_LEN    48

public:
    //  default constructor
    CLiteMySQLWrapper();

    //  default destructor
    virtual ~CLiteMySQLWrapper();

    bool Connect(const char* host,
                 const char* user,
                 const char* passwd,
                 const char* db,
                 uint32_t flag = 0 ,
                 int32_t port = 3306);

    //
    //  Used to execute db sql query.
    //  @param szSqlString indicates sql string .
    //
    bool  Query(const char* sql_string);

    //
    //  Used to execute db sql query.
    //  @param szSqlString indicates sql string
    //   which should include binary data.
    //
    //  @param size indicates sql size of szSqlString.
    //  real query
    //
    bool  Query(const char* sql_string, uint32_t size);

    //
    //  Used to store query result.
    //
    bool StoreResult();

    //
    //  Used to Free Query result.
    //
    bool FreeResult();

    //
    //  Used to get result row .
    //  @return NULL on failure
    //  else result row of query.
    //
    char** FetchRow();

    //
    //  返回结果的列数
    //
    uint32_t GetFieldsCount() const;

    //
    //  Used to strip abnormal character .
    //  @param to indictes striped stirng.
    //  @param from indicates original string.
    //  @param len indicates from string length.
    //
    bool StripField(unsigned char* pToBuf, uint32_t uToBufLen, const char* from,
                    uint32_t len);

    //
    //  Used to get field result by ordinal number.
    //  @param iFiled indicates field ordinal number.
    //  @return NULL on failure.
    //  else field result.
    //  尽量使用这个接口
    //
    char* GetFieldByIndex(uint32_t field);

    //
    //  Used to get field result by field name.
    //  @param szFieldName indicates field name.
    //  @return NULL on failure.
    //  else field result.
    //  尽量少用这个接口，效率不如直接用index高
    //
char* GetFieldByColName(const char* field_name, bool case = false);

    //
    //  Used to get query affected rows.
    //  @return affected row count.
    //
    uint32_t GetAffectedRows();

    //
    //  Used to get mysql sock handle.
    //  @return sock handle.
    //
    inline   MYSQL* GetConnectHandle() {
        return &m_mysql_connection;
    }

    bool IsConnected() const {
        return m_is_connected;
    }

    void SetDefaultCharset(const char* charset);

private:
    char        m_default_charset[32];
    char        m_host[64];        // 数据库主机名
    char        m_user[32];        // 数据库用户名
    char        m_password[32];    // 数据库用户密码
    uint32_t    m_client_flag;

    uint16_t    m_db_port;         //  Port default:3306
    char        m_db_name[128];    //  db name.
    uint32_t    m_fields_count;
    MYSQL       m_mysql_connection;
    MYSQL_RES*  m_mysql_result;
    MYSQL_ROW   m_mysql_rows;
    uint32_t    m_reconn_count;     // reconnect count
    char        m_fields_cols_name
    [MYSQL_QQ_MAX_COLUMNS][MYSQL_QQ_MAX_COL_NAME_LEN];

    bool        m_is_field_index_initialized;
    bool        m_is_connected;

    bool        InitFieldName();

    //  Used to establish db connection.
    bool        EstablishConnect();
};

struct MySQLAccountInfo {
    char        _host[24];
    char        _user[32];
    char        _password[32];
    char        _database[32];
    uint16_t    _port;
    uint32_t    _flag;
    MySQLAccountInfo(char* host, char* user, char* password, char* database) {
        _port = 3306;
        _flag = 0;

        memset(_host, 0, sizeof(_host));
        memset(_user, 0, sizeof(_user));
        memset(_password, 0, sizeof(_password));
        memset(_database, 0, sizeof(_database));
        int32_t n = safe_snprintf(_host, sizeof(_host), "%s", host);
        _host[n] = 0;

        n = safe_snprintf(_user, sizeof(_user), "%s", user);
        _user[n] = 0;

        n = safe_snprintf(_password, sizeof(_password), "%s", password);
        _password[n] = 0;

        n = safe_snprintf(_database, sizeof(_database), "%s", database);
        _database[n] = 0;
    }

    MySQLAccountInfo(char* host, char* user, char* password, char* database,
                     uint16_t    port, uint32_t flag_val) {
        _port = port;
        _flag = flag_val;

        memset(_host, 0, sizeof(_host));
        memset(_user, 0, sizeof(_user));
        memset(_password, 0, sizeof(_password));
        memset(_database, 0, sizeof(_database));
        int32_t n = safe_snprintf(_host, sizeof(_host), "%s", host);
        _host[n] = 0;

        int32_t n = safe_snprintf(_user, sizeof(_user), "%s", user);
        _user[n] = 0;

        n = safe_snprintf(_password, sizeof(_password), "%s", password);
        _password[n] = 0;

        n = safe_snprintf(_database, sizeof(_database), "%s", database);
        _database[n] = 0;
    }

    MySQLAccountInfo() {
        _port = 3306;
        _flag = 0;

        memset(_host, 0, sizeof(_host));
        memset(_user, 0, sizeof(_user));
        memset(_password, 0, sizeof(_password));
        memset(_database, 0, sizeof(_database));
    }
};

class CLiteMySQLManage {
public:
#define MAX_QUERY_SQL_LENGTH       1024
#define MAX_LONG_SQL_LENGTH        (50*1024)

public:
    CLiteMySQLManage();
    explicit CLiteMySQLManage(CLiteMySQLWrapper* lite_mysql);
    explicit CLiteMySQLManage(MySQLAccountInfo* mysql_account_info);
    explicit CLiteMySQLManage(MySQLAccountInfo mysql_account_info);

    bool    InitLiteMySQL(MySQLAccountInfo* mysql_account_info);
    bool    InitLiteMySQL();
    void    UnInitLiteMySQL();
    void    ResetSQLQueryString();
    void    ResetLongSQLString();

    CLiteMySQLWrapper*  GetMySQLWrapperObject();

private:
    MySQLAccountInfo    m_mysql_account_info;
    bool                m_is_inside_create_lite_mysql_obj;
protected:
    CLiteMySQLWrapper*  m_lite_mysql_obj;
    char                m_sql_query[MAX_QUERY_SQL_LENGTH];
    char                m_long_sql[MAX_LONG_SQL_LENGTH];
};
#endif // _MYSQL_SUPPORT

_END_XFS_BASE_NAMESPACE_

#endif // COMMON_BASELIB_SVRPUBLIB_LITE_MYSQL_WRAPPER_H_
