// key_value_parser.cpp: implementation of the CKeyValueParser class.
// wookin@tencent.com
// ////////////////////////////////////////////////////////////////////

#include <string.h>
#include "common/baselib/svrpublib/server_publib.h"

#include "common/baselib/svrpublib/base_config.h"

_START_XFS_BASE_NAMESPACE_

// /////////////////////////
// Construction/Destruction
// /////////////////////////
CKeyValueParser::CKeyValueParser(uint32_t init_buff_size,
                                 uint32_t init_key_size,
                                 unsigned char is_debug_output) {
    m_cache_buff_len = 0;
    m_valid_data_len = 0;
    m_cache_buff = new unsigned char[init_buff_size];
    if (m_cache_buff) {
        m_cache_buff_len = init_buff_size;
    }

    m_max_keyval_descs = 0;
    m_keyval_desc_count = 0;
    m_keyval_descs = new KeyValDesc[init_key_size];
    if (m_keyval_descs) {
        m_max_keyval_descs = init_key_size;
    }

    m_is_debug_output = is_debug_output;
    m_key_query_cursor = 0;
}

CKeyValueParser::~CKeyValueParser() {
    // release resource
    delete []m_cache_buff;
    m_cache_buff = 0;

    m_cache_buff_len = 0;
    m_valid_data_len = 0;

    delete []m_keyval_descs;
    m_keyval_descs = 0;

    m_max_keyval_descs = 0;
    m_keyval_desc_count = 0;
}

bool    CKeyValueParser::ParserFromFile(const char* filename) {
    bool b = false;
    if (filename) {
        FILE* fp = fopen(filename, "rb");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            uint32_t lsize = ftell(fp);
            if (lsize >= m_cache_buff_len || !m_cache_buff) {
                delete []m_cache_buff;
                m_cache_buff = 0;

                m_cache_buff = new unsigned char[lsize+256];
                if (m_cache_buff) {
                    m_cache_buff_len = lsize+256;
                } else {
                    m_cache_buff_len = 0;
                }
                m_valid_data_len = 0;
            }

            if (m_cache_buff && lsize>0) {
                memset(m_cache_buff, 0, m_cache_buff_len);
                m_valid_data_len = 0;
                fseek(fp, 0, SEEK_SET);
                if (fread(m_cache_buff, lsize, 1, fp) == 1) {
                    m_valid_data_len = lsize;
                    if (!(m_cache_buff[m_valid_data_len-1] == '\r' ||
                            m_cache_buff[m_valid_data_len-1] == '\n')) {
                        m_cache_buff[m_valid_data_len] = '\r';
                        m_valid_data_len++;
                    }
                    b = Parser();
                }
            }

            fclose(fp);
            fp = 0;
        }
    }
    return b;
}

bool    CKeyValueParser::ParserFromBuffer(const unsigned char* buff,
        uint32_t len) {
    bool b = false;
    if (buff && len) {
        if (len >= m_cache_buff_len || !m_cache_buff) {
            delete []m_cache_buff;
            m_cache_buff = 0;

            m_cache_buff = new unsigned char[len+256];
            if (m_cache_buff) {
                m_cache_buff_len = len+256;
            } else {
                m_cache_buff_len = 0;
            }
            m_valid_data_len = 0;
        }

        if (m_cache_buff) {
            memset(m_cache_buff, 0, m_cache_buff_len);
            memcpy(m_cache_buff, buff, len);
            m_valid_data_len = len;
            if (!(m_cache_buff[m_valid_data_len-1] == '\r' ||
                    m_cache_buff[m_valid_data_len-1] == '\n')) {
                m_cache_buff[m_valid_data_len] = '\r';
                m_valid_data_len++;
            }
            b = Parser();
        }
    }
    return b;
}

bool    CKeyValueParser::Parser() {
    bool b = false;
    if (m_cache_buff && m_valid_data_len>0) {
        uint32_t i = 0;
        int32_t old_flag = -1;
        int32_t flag = -1;
        uint32_t lines = 0;
        for (i = 0; i < m_valid_data_len; i++) {
            flag = (m_cache_buff[i] == '\r' || m_cache_buff[i] == '\n') ? 0:1;
            if (old_flag != flag) {
                lines++;
                old_flag = flag;
            }
        }
        lines++;

        if (lines >= m_max_keyval_descs) {
            delete []m_keyval_descs;
            m_keyval_descs = 0;

            m_max_keyval_descs = 0;
            m_keyval_desc_count = 0;
            m_keyval_descs = new KeyValDesc[lines+2];
            if (m_keyval_descs) {
                m_max_keyval_descs = lines+2;
            }
        }

        //  Parser
        if (m_keyval_descs) {
            memset((unsigned char*)m_keyval_descs, 0,
                   sizeof(KeyValDesc)*m_max_keyval_descs);
            m_keyval_desc_count = 0;

            int32_t    key_start = -1;
            int32_t    equal_pos = -1;
            int32_t    val_end = -1;
            m_keyval_desc_count = 0;
            old_flag = -1;
            for (i = 0; i < m_valid_data_len; i++) {
                flag = (m_cache_buff[i] == '\r' || m_cache_buff[i] == '\n') ?
                       0 :
                       1;
                if (old_flag != flag && flag == 1) { //  0-->1
                    //  Start of new line
                    if (key_start < 0 &&
                            !(m_cache_buff[i] == '\r' ||
                              m_cache_buff[i] == '\n' ||
                              m_cache_buff[i] == '='  ||
                              m_cache_buff[i] == ' ' ||
                              m_cache_buff[i] == '\t')) {
                        key_start = i;
                        old_flag = flag;
                    }
                }

                if (key_start >= 0 && equal_pos < 0 &&
                        m_cache_buff[i] == '=')
                    equal_pos = i;

                if (old_flag != flag && flag == 0) { //  1-->0
                    //  End of the line
                    val_end = i-1;
                    if (key_start >= 0 && equal_pos > 0 && val_end > 0 &&
                            equal_pos > key_start && val_end > equal_pos) {
                        m_keyval_descs[m_keyval_desc_count].valid = true;
                        m_keyval_descs[m_keyval_desc_count].key_start =
                            key_start;

                        m_keyval_descs[m_keyval_desc_count].key_end =
                            equal_pos - 1;

                        m_keyval_descs[m_keyval_desc_count].val_start =
                            equal_pos + 1;

                        m_keyval_descs[m_keyval_desc_count].val_end = val_end;
                        m_keyval_desc_count++;
                    }
                    key_start = -1;
                    equal_pos = -1;
                    val_end = -1;
                    old_flag = flag;
                }
            }
            b = m_keyval_desc_count > 0;
        }
    }

    return b;
}

bool    CKeyValueParser::GetValue(const char* key,
                                  unsigned char* buff, int32_t buff_len) {
    return GetValue(key, buff, buff_len, 0, false);
}

bool    CKeyValueParser::GetValue(const char* key,
                                  unsigned char* buff, int32_t buff_len,
                                  uint32_t key_index) {
    return GetValue(key, buff, buff_len, key_index, true);
}

bool    CKeyValueParser::GetValue(const char* key,
                                  unsigned char* val_buff,
                                  int32_t val_buff_len,
                                  uint32_t key_index,
                                  bool use_index){
    bool b = false;
    if (m_keyval_desc_count > 0 &&
            key && val_buff && val_buff_len &&
            m_keyval_descs) {
        uint32_t i = 0;
        uint32_t same_key_count = 0;

        // Invalid cursor
        if (m_key_query_cursor >= m_keyval_desc_count)
            m_key_query_cursor = 0;

        // ? can use cursor
        if (use_index)
            m_key_query_cursor = 0;

        // from cursor to end
        uint32_t key_len = STRLEN(key);
        for (i = m_key_query_cursor; i < m_keyval_desc_count; i++) {
            if (m_keyval_descs[i].valid &&
                    key_len ==
                    m_keyval_descs[i].key_end - m_keyval_descs[i].key_start + 1 &&
                    memcmp(m_cache_buff +
                           m_keyval_descs[i].key_start, key,
                           key_len) == 0) {
                if (same_key_count == key_index) {
                    int32_t val_len = m_keyval_descs[i].val_end -
                                      m_keyval_descs[i].val_start + 1;

                    if (val_buff_len >= val_len) {
                        memcpy(val_buff,
                               m_cache_buff + m_keyval_descs[i].val_start,
                               val_len);

                        val_buff[val_len] = 0;
                        b = true;
                        m_key_query_cursor = i;
                    }
                    break;
                }
                same_key_count++;
            }
        }

        // From 0 to cursor
        if (!b) {
            same_key_count = 0;
            for (i = 0; i < m_key_query_cursor; i++) {
                if (m_keyval_descs[i].valid &&
                        key_len ==
                        m_keyval_descs[i].key_end-m_keyval_descs[i].key_start + 1 &&
                        memcmp(m_cache_buff + m_keyval_descs[i].key_start,
                               key,
                               key_len) == 0) {
                    if (same_key_count == key_index) {
                        int32_t v_len =
                            m_keyval_descs[i].val_end-m_keyval_descs[i].val_start
                            + 1;

                        if (val_buff_len >= v_len) {
                            memcpy(val_buff,
                                   m_cache_buff + m_keyval_descs[i].val_start,
                                   v_len);

                            val_buff[v_len] = 0;
                            b = true;
                            m_key_query_cursor = i;
                        }
                        break;
                    }
                    same_key_count++;
                }
            }
        }
    }

    if (m_is_debug_output && !b) {
        VLOG(3) << "get value of key:" << key << " fail";
    }
    return b;
}

int32_t  CKeyValueParser::GetSameKeyCount(const char* key_name) {
    int32_t same_key_count = 0;
    if (m_keyval_desc_count > 0 &&
            key_name &&
            m_keyval_descs) {
        uint32_t i = 0;
        uint32_t name_len = STRLEN(key_name);
        for (i = 0; i < m_keyval_desc_count; i++) {
            if (m_keyval_descs[i].valid &&
                    name_len ==
                    m_keyval_descs[i].key_end-m_keyval_descs[i].key_start+1 &&
                    memcmp(m_cache_buff + m_keyval_descs[i].key_start,
                           key_name,
                           name_len) == 0) {
                same_key_count++;
            }
        }
    }
    return same_key_count;
}

#ifdef _DEBUG
void    CKeyValueParser::ListKeyVal(FILE* fp_log) {
    uint32_t i = 0;
    for (i = 0; i < m_keyval_desc_count && fp_log; i++) {
        int32_t key_len =
            m_keyval_descs[i].key_end - m_keyval_descs[i].key_start + 1;
        fwrite(m_cache_buff + m_keyval_descs[i].key_start, key_len, 1, fp_log);
        fprintf(fp_log, "=");
        int32_t val_len =
            m_keyval_descs[i].val_end - m_keyval_descs[i].val_start + 1;
        fwrite(m_cache_buff + m_keyval_descs[i].val_start, val_len, 1, fp_log);
        fprintf(fp_log, "\r\n");
    }

    VLOG(3) << "Total records:" << static_cast<uint32_t>(m_keyval_desc_count);
}
#endif // _DEBUG

//
// int32_t main(int32_t argc, char** argv)
// {
// CKeyValueParser obj(1024, 32);
// if (obj.ParserFromFile("c:\\test.txt"))
// {
// obj.ListKeyVal(stdout);
// }
//
//  printf("-------------------------\r\n");
//  char val[256];
//  obj.GetValue("URL", val, 256);
//  printf("url is = %s\r\n", val);
//  obj.GetValue("URL", val, 256, 1);
//  printf("url is = %s\r\n", val);
//
//
//      printf("--------------------\r\nfrom buffer...\r\n");
//      char* psz="key1=boy\rkey2=girl\r"
//                "URL=http://www.girlclick.com.cn/forumdisplay.php?fid=283\r"
//                "URL=http://www.playboyqq.com";
//      if (obj.ParserFromBuffer(psz, STRLEN(psz)))
//      {
//      obj.GetValue("URL", val, 256);
//      printf("url is=%s\r\n", val);
//      obj.GetValue("URL", val, 256, 1);
//      printf("url is=%s\r\n", val);
//      }
//
//        return 0;
// }
//

_END_XFS_BASE_NAMESPACE_
