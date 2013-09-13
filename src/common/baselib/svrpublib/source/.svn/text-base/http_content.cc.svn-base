// http_content.cc: implementation of the CHttpContent class.
//
//////////////////////////////////////////////////////////////////////

#include "thirdparty/zlib-win32/include/zlib.h"

#include "common/baselib/svrpublib/server_publib_namespace.h"
#include "common/baselib/svrpublib/http_content.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHttpContent::CHttpContent() {}

CHttpContent::~CHttpContent() {}

bool CHttpContent::Gunzip(const char* ptr_data,
                          unsigned int len, CStrBuff& obj_str_buffer) {
    CHECK(ptr_data);

    bool        is_ok = true;
#if 0
    z_stream    stream;
    int32_t     gzip_error_code = 0;
    uint32_t    zip_len = len * 25;
    char*       ptr_out_data_buffer = new char[zip_len];
    memset(ptr_out_data_buffer, 0, zip_len);

    stream.next_in = NULL;
    stream.avail_in = 0;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = 0;
    gzip_error_code = inflateInit2(&stream, 40);

    if (gzip_error_code != Z_OK) {
        LOG(ERROR) << "no built-in gzip support in zlib.";
        delete [] ptr_out_data_buffer;
        ptr_out_data_buffer = NULL;

        return false;
    }

    stream.next_in = (Bytef*)ptr_data;
    stream.avail_in = len;
    stream.next_out = (Bytef*)ptr_out_data_buffer;
    stream.avail_out = zip_len;
    gzip_error_code = inflate(&stream, Z_SYNC_FLUSH);

    if (gzip_error_code == Z_DATA_ERROR) {
        inflateEnd(&stream);
        inflateInit2(&stream, -8);

        if (gzip_error_code != Z_OK) {
            LOG(ERROR) << "can not decode gzip header.";
            delete [] ptr_out_data_buffer;
            ptr_out_data_buffer = NULL;

            return false;
        }

        stream.next_in = (Bytef*)ptr_data;
        stream.avail_in = len;
        stream.next_out = (Bytef*)ptr_out_data_buffer;
        stream.avail_out = zip_len;
        gzip_error_code = inflate(&stream, Z_SYNC_FLUSH);
    }

    while (gzip_error_code == Z_OK) {
        //append data to buffer
        obj_str_buffer.AppendStr(ptr_out_data_buffer, zip_len - stream.avail_out);
        //reset buffer pointer
        stream.next_out = (Bytef*)ptr_out_data_buffer;
        stream.avail_out = zip_len;
        gzip_error_code = inflate(&stream, Z_SYNC_FLUSH);
    }

    if (gzip_error_code == Z_STREAM_END) {
        obj_str_buffer.AppendStr(ptr_out_data_buffer, zip_len - stream.avail_out);
    } else {
        LOG(ERROR) << "gzip inflate error.";
        is_ok = false;
    }

    inflateEnd(&stream);
    delete [] ptr_out_data_buffer;
    ptr_out_data_buffer = NULL;
#endif
    return is_ok;
}

bool CHttpContent::GetTransferEncoding(const char* ptr_head, uint32_t head_len,
                                       const char** ptr_ptr_type,
                                       uint32_t* ptr_type_len) {
    bool b = false;

    if (ptr_head && head_len && ptr_ptr_type) {
        const char* key1 = "Transfer-Encoding";
        const char* key2 = "transfer-encoding";

        if (GetValue(ptr_head, head_len, key1, ptr_ptr_type, ptr_type_len)
                || GetValue(ptr_head, head_len, key2, ptr_ptr_type, ptr_type_len)) {
            b = true;
        }
    }

    return b;
}

bool CHttpContent::GetContentEncoding(const char* ptr_head, uint32_t head_len,
                                      const char** ptr_ptr_type,
                                      uint32_t* ptr_type_len) {
    bool b = false;

    if (ptr_head && head_len && ptr_ptr_type) {
        const char* key1 = "Content-Encoding";
        const char* key2 = "content-encoding";

        if (GetValue(ptr_head, head_len, key1, ptr_ptr_type, ptr_type_len)
                || GetValue(ptr_head, head_len, key2, ptr_ptr_type, ptr_type_len)) {
            b = true;
        }
    }

    return b;
}

// 获取值(头部中key:val对)
bool CHttpContent::GetValue(const char* ptr_head, uint32_t head_len,
                            const char* ptr_key_name,
                            const char** ptr_ptr_val, uint32_t* val_len) {
    bool b = false;

    if (!ptr_head || !head_len || !ptr_key_name || !ptr_ptr_val)
        return b;

    uint32_t len = (uint32_t)strlen((char*)ptr_key_name);

    if (head_len > len) {
        uint32_t urange = head_len - len;

        for (uint32_t u = 0; u < urange; u++) {
            if (ptr_head[u] == '\n') {
                if (memcmp(ptr_head + u + 1, ptr_key_name, len) == 0) {
                    const char* ptr = ptr_head + u + 1 + len;

                    while (*ptr == ' ' || *ptr == ':')
                        ptr++;

                    *ptr_ptr_val = ptr;

                    // get length
                    if (val_len) {
                        (*val_len) = 0;

                        while (!(*ptr == '\r' || *ptr == '\n')) {
                            ptr++;
                            (*val_len)++;
                        }
                    }

                    b = true;
                    break;
                }
            }
        }
    }

    return b;
}

const char* CHttpContent::StrStr(const char* ptr_src, uint32_t len, const char* ptr_sub_str) {
    if (!ptr_src || !len || !ptr_sub_str)
        return NULL;

    uint32_t sub_len = (uint32_t)strlen(ptr_sub_str);
    uint32_t range = len - sub_len;

    for (uint32_t u = 0; u <= range; u++) {
        if (ptr_src[u] == ptr_sub_str[0]) {
            if (memcmp(ptr_src + u, ptr_sub_str, sub_len) == 0) {
                return ptr_src + u;
            }
        }
    }

    return NULL;
}

// 解析获得的内容
// 内容获取过程由回调函数完成
bool CHttpContent::Parser(pFunGetNexchar fun_get_next, void* ptr_in_param,
                          pFunOutputPureBody fun_output, void* ptr_out_param) {
    m_head_buff.Reset();
    unsigned char ch = 0;

    // get header
    // head和body之间一个空行分隔
    bool is_got_head = false;

    while (fun_get_next(&ch, ptr_in_param)) {
        bool is_crlf = (m_head_buff.GetLastChar() == '\n' && (ch == '\r' || ch == '\n'));
        m_head_buff.AppendStr(ch);

        if (is_crlf) {
            is_got_head = true;
            break;
        }
    }

    if (!is_got_head)
        return false;

    // invalid head buffer
    if (!m_head_buff.GetValidLen())
        return false;

    // get Transfer-encoding
    bool is_chunked = false;
    bool is_encoding = false;
    const char*  ptr_type = NULL;
    uint32_t    type_len = 0;

    if (GetTransferEncoding(m_head_buff.GetString(), m_head_buff.GetValidLen(), &ptr_type, &type_len)
            && StrStr(ptr_type, type_len, "chunked")) {
        is_chunked = true;
    }

    //? Encoding
    if (GetContentEncoding(m_head_buff.GetString(), m_head_buff.GetValidLen(), &ptr_type, &type_len)
            && StrStr(ptr_type, type_len, "gzip")) {
        is_encoding = true;
    }

    // seek to valid head
    ch = 0;
    fun_get_next(&ch, ptr_in_param);

    if (ch != '\n')
        return false;

    fun_get_next(&ch, ptr_in_param);

    // read next char fail
    if (!ch)
        return false;

    bool is_got_body = false;

    if (is_chunked) {
        while (1) {
            // get row buffer(chunked length in hex)
            m_tmp_row.Reset();
            m_tmp_row.AppendStr(ch);

            while (fun_get_next(&ch, ptr_in_param)) {
                if (ch == '\r' || ch == '\n')
                    break;

                m_tmp_row.AppendStr(ch);
            }

            uint32_t chunk_len = 0;
            sscanf(m_tmp_row.GetString(), "%x", &chunk_len);

            // ? last chunk
            if (chunk_len == 0) {
                fun_output(0, true, ptr_out_param);
                is_got_body = true;
                break;
            }

            // seek to valid entity
            ch = 0;

            if (!fun_get_next(&ch, ptr_in_param) && ch != '\n')
                break;

            if (!fun_get_next(&ch, ptr_in_param))
                break;

            // read chunk data
            unsigned char* ptr_buff = NULL;

            if (GetNextPtrFromBuff(&ptr_buff, chunk_len - 1, ptr_in_param)) {
                if (is_encoding) {
                    m_obj_eoncoding_buff.AppendStr(reinterpret_cast<char*>(ptr_buff) - 1, chunk_len);
                } else {
                    OutPureBuffFromBuff((const char*)(ptr_buff - 1), chunk_len, ptr_out_param);
                }
            } else {
                // read data fail
                return false;
            }

            // seek to next valid entity(maybe chunked length)
            ch = 0;

            while (fun_get_next(&ch, ptr_in_param)) {
                if (!(ch == '\r' || ch == '\n'))
                    break;
            }

            // read next char fail
            if (!ch)
                return false;
        }

        if (is_encoding) {
            m_obj_uneoncoding_buff.Reset();
            Gunzip(m_obj_eoncoding_buff.GetString(),
                   m_obj_eoncoding_buff.GetValidLen(),
                   m_obj_uneoncoding_buff);

            OutPureBuffFromBuff(m_obj_uneoncoding_buff.GetString(),
                                m_obj_uneoncoding_buff.GetValidLen(),
                                ptr_out_param);
        }
    } else {
        // content-length
        const char* ptr_dest1 = strstr(m_head_buff.GetString(), "Content-Length:");
        const char* ptr_dest2 = strstr(m_head_buff.GetString(), "Content-length:");
        const char* ptr_dest3 = strstr(m_head_buff.GetString(), "content-length:");
        const char* ptr_dest = ptr_dest1 ? ptr_dest1 : (ptr_dest2 ? ptr_dest2 : ptr_dest3);
        uint32_t max_content_len = ptr_dest ? ATOI(ptr_dest + strlen("content-length:")) : 0;

        // normal http body
        unsigned char* ptr_buff = NULL;

        // next max_content_len-1 http body
        if (GetNextPtrFromBuff(&ptr_buff, max_content_len - 1, ptr_in_param)) {
            if (is_encoding) {
                m_obj_uneoncoding_buff.Reset();
                Gunzip(reinterpret_cast<char*>(ptr_buff) - 1, max_content_len,
                       m_obj_uneoncoding_buff);
                OutPureBuffFromBuff(m_obj_uneoncoding_buff.GetString(),
                                    m_obj_uneoncoding_buff.GetValidLen(),
                                    ptr_out_param);
            } else {
                OutPureBuffFromBuff((const char*)(ptr_buff - 1), max_content_len, ptr_out_param);
            }

            is_got_body = true;
            fun_output(0, true, ptr_out_param);
        } else {
            return is_got_body;
        }
    }

    return is_got_body;
}

// parser data
bool CHttpContent::ParserFromFile(char* ptr_file_name) {
    FILE* fp = fopen(ptr_file_name, "rb");
    bool b = Parser(GetNexcharFromFile, (void*)fp, OutputPureBody, (void*)stdout);
    fclose(fp);

    return b;
}

bool CHttpContent::ParserFromBuff(const char* ptr_input_buff, uint32_t buff_len,
                                  const char** ptr_ptr_pure_body, uint32_t* body_len) {
    bool b = false;

    // parser from buffer
    ParserFromBuffInfo obj_buff_info;
    obj_buff_info.ptr_in_buff = (unsigned char*)ptr_input_buff;
    obj_buff_info.in_buff_len = buff_len;
    obj_buff_info.pos = 0;

    m_out_buff.Reset();
    b = Parser(GetNexcharFromBuff, &obj_buff_info, OutPureBuffFromBuff, &m_out_buff);

    if (b) {
        *ptr_ptr_pure_body = m_out_buff.GetString();
        *body_len = m_out_buff.GetValidLen();
    }

    return b;
}

bool CHttpContent::GetNexcharFromBuff(unsigned char* ptr_char, void* ptr_in_param) {
    ParserFromBuffInfo* ptr_from_buff = (ParserFromBuffInfo*)ptr_in_param;

    if (ptr_from_buff->pos < ptr_from_buff->in_buff_len) {
        *ptr_char = ptr_from_buff->ptr_in_buff[ptr_from_buff->pos];
        ptr_from_buff->pos++;

        return true;
    }

    return false;
}

bool CHttpContent::GetNextContentFromBuff(unsigned char* ptr_content,
                                          uint32_t size,void* ptr_in_param) {
    ParserFromBuffInfo* ptr_from_buff = (ParserFromBuffInfo*)ptr_in_param;

    if (ptr_from_buff->pos + size <= ptr_from_buff->in_buff_len) {
        memcpy(ptr_content, ptr_from_buff->ptr_in_buff + ptr_from_buff->pos, size);
        ptr_from_buff->pos += size;

        return true;
    }

    return false;
}

bool CHttpContent::GetNextPtrFromBuff(unsigned char** ptr_content, uint32_t size,
                                      void* ptr_type_len) {
    ParserFromBuffInfo* ptr_from_buff = (ParserFromBuffInfo*)ptr_type_len;

    if (ptr_from_buff->pos + size <= ptr_from_buff->in_buff_len) {
        *ptr_content = ptr_from_buff->ptr_in_buff + ptr_from_buff->pos;
        ptr_from_buff->pos += size;

        return true;
    }

    return false;
}

void CHttpContent::OutPureBuffFromBuff(unsigned char ch, bool is_end_flag, void* ptr_out_param) {
    if (!is_end_flag) {
        CStrBuff* ptr_buff = (CStrBuff*)ptr_out_param;
        ptr_buff->AppendStr(ch, 1);
    }
}

void CHttpContent::OutPureBuffFromBuff(const char* ptr_content, uint32_t size,
                                       void* ptr_out_param) {
    CStrBuff* ptr_buff = (CStrBuff*)ptr_out_param;
    ptr_buff->AppendStr(ptr_content, size);
}

bool CHttpContent::GetNexcharFromFile(unsigned char* ptr_char, void* ptr_in_param) {
    FILE* fp = (FILE*)ptr_in_param;
    return fread(ptr_char, 1, 1, fp) == 1 ? true : false;
}

void CHttpContent::OutputPureBody(unsigned char ch, bool is_end_flag, void* ptr_out_param) {
    FILE* fp_out = (FILE*)ptr_out_param;

    if (is_end_flag) {
        if (fp_out == stdout)
            fprintf(fp_out, "\r\n-----\r\n");
    } else {
        fwrite(&ch, 1, 1, fp_out);
    }
}
