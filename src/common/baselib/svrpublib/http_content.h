//////////////////////////////////////////////////////////////////////////
// http_content.h
// @brief:   ������cgi���ص���Ӧ���ݵ���
// @author:  jackyzhao@tencent
// @time:    2010-8-27
// @version: 1.0
//////////////////////////////////////////////////////////////////////////

#if !defined(XFS_SCHEDULER_MASTER_HTTP_CONTENT_H_)
#define XFS_SCHEDULER_MASTER_HTTP_CONTENT_H_

// ��ȡԭʼ��������ص�����
typedef bool (*pFunGetNexchar)(unsigned char* ptr_char, void* ptr_in_param);
// �����������Ļص�����
typedef void (*pFunOutputPureBody)(unsigned char ch, bool is_end_flag, void* ptr_out_param);
typedef void (*pFunOutputContent)(unsigned char* ptr_data, uint32_t len, bool is_close);

// name:        CHttpContent
// description: ����http����
//              ֧�ֱ�����߽���,�����ر߽���
//              client����web server��������
class CHttpContent {
public:
    CHttpContent();
    ~CHttpContent();

    // ��ȡֵ(ͷ����key:val��)
    bool GetValue(const char* ptr_head, uint32_t head_len,
                  const char* ptr_key_name,
                  const char** ptr_ptr_val, uint32_t* val_len);

    // ��ȡtransfer-encoding����
    bool GetTransferEncoding(const char* ptr_head, uint32_t head_len,
                             const char** ptr_ptr_type, uint32_t* ptr_type_len);

    // ��ȡcontent-encoding����
    bool GetContentEncoding(const char* ptr_head, uint32_t head_len,
                            const char** ptr_ptr_type,
                            uint32_t* ptr_type_len);

    // ������õ�����
    // ���ݻ�ȡ�����ɻص��������,�ص���������false����������
    // ���ʹ��chunked����,����Ҫ�⿪,Ŀǰ��֧��gzip
    bool Parser(pFunGetNexchar fun_get_next, void* ptr_in_param,
                pFunOutputPureBody fun_output, void* ptr_out_param);

    bool ParserFromBuff(const char* ptr_input_buff, uint32_t buff_len,
                        const char** ptr_ptr_pure_body, uint32_t* body_len);

    // parser data
    bool ParserFromFile(char* ptr_file_name);

private:
    struct ParserFromBuffInfo {
        unsigned char* ptr_in_buff;
        uint32_t in_buff_len;
        uint32_t pos;
        ParserFromBuffInfo() {
            ptr_in_buff = 0;
            in_buff_len = 0;
            pos = 0;
        }
    };

    static bool GetNexcharFromFile(unsigned char* ptr_char, void* ptr_in_param);

    static void OutputPureBody(unsigned char ch, bool is_end_flag, void* ptr_out_param);

    const char* StrStr(const char* ptr_src, uint32_t len, const char* ptr_sub_str);

    static bool GetNexcharFromBuff(unsigned char* ptr_char, void* ptr_in_param);

    static bool GetNextContentFromBuff(unsigned char* ptr_content, uint32_t size,
                                       void* ptr_in_param);

    static bool GetNextPtrFromBuff(unsigned char** ptr_content, uint32_t size, void* ptr_type_len);

    static void OutPureBuffFromBuff(unsigned char ch, bool is_end_flag, void* ptr_out_param);

    static void OutPureBuffFromBuff(const char* ptr_content, uint32_t size, void* ptr_out_param);

    bool Gunzip(const char* ptr_data, unsigned int len, CStrBuff& obj_str_buffer);

    // http head
    CStrBuff    m_head_buff;

    // temp row buffer
    CStrBuff    m_tmp_row;

    // temp parser from buffer output buffer
    CStrBuff m_out_buff;

    // temp zip buffer
    CStrBuff m_obj_eoncoding_buff;

    // temp unzip buffer
    CStrBuff m_obj_uneoncoding_buff;
};

#endif // !defined(XFS_SCHEDULER_MASTER_HTTP_CONTENT_H_)
