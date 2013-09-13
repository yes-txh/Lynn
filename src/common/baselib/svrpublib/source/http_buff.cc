//--------------------------------------------------
// http_buff.cpp
// author: wookin
// create: 2010.8
// modify: ttyyang@2010.8
// modify: bayou@2010.8.18
//
//--------------------------------------------------

#include "common/baselib/svrpublib/server_publib.h"
#include "common/baselib/svrpublib/http_buff.h"
#include "common/baselib/svrpublib/base_config.h"
#include "common/baselib/svrpublib/default_xsl.h"

DECLARE_string(xfs_cluster_name);

_START_XFS_BASE_NAMESPACE_

// 设置一个默认的home 地址
// 用户可以通过设置home=xxx 来更改这个设定
const char* DEFAULT_HOME_URI   = "http://xfs.soso.oa.com/";
const char* XFS_HOME_PAGE_NAME = "XFS";

extern CAutoLibGlobalVars g_lib_vars;

void SetModuleName(const char* module){
    if(module)
        safe_snprintf(g_lib_vars.m_http_module_name,
               sizeof(g_lib_vars.m_http_module_name),
               "%s", module);    
}


CHttpBuff::CHttpBuff() {
    m_http_head.SetExtendStep(10240);
    m_http_tail.SetExtendStep(10240);
    m_http_body.SetExtendStep(10240);
    m_http_raw_data.SetExtendStep(10240);

    m_is_raw_data = false;

    memset(m_url_proxy, 0, sizeof(m_url_proxy));
    memset(m_url_homepage, 0, sizeof(m_url_homepage));
    safe_snprintf(m_url_homepage, sizeof(m_url_homepage), "%s", DEFAULT_HOME_URI);

    memset(m_title, 0, sizeof(m_title));
    memset(m_module, 0, sizeof(m_module));
    memset(m_login_name, 0, sizeof(m_login_name));
    memset(m_login_page, 0, sizeof(m_login_page));
    memset(m_host_port, 0, sizeof(m_host_port));

    m_is_homepage = false;
    m_is_set_login_name = false;
    m_is_set_login_page = false;

    char name_module[256] = {0};
    GetModuleFileName(NULL, name_module, sizeof(name_module));

    // linux and windows
    char* p = strrchr(name_module, '/');

    if ( !p ) {
        p = strrchr(name_module, '\\');
    }

    if (p) {
        p++;

    } else {
        p = name_module;
    }

    safe_snprintf(m_module, sizeof(m_module), "%s", p);

    m_is_output_xml = false;
    m_div_idx = 0;

    m_footer_buffer = new char[MAX_FOOTER_BUFF_LENGTH];
    memset(m_footer_buffer, 0, MAX_FOOTER_BUFF_LENGTH);

    m_is_group_model = false;

    m_jasoncallback.Reset();
}

CHttpBuff::~CHttpBuff() {
    if (m_footer_buffer) {
        delete m_footer_buffer;
        m_footer_buffer = NULL;
    }
}


void CHttpBuff::ResetHttpContent() {
    m_http_head.Reset();
    m_http_tail.Reset();
    m_http_body.Reset();
    m_http_raw_data.Reset();

    m_jasoncallback.Reset();

    m_title[0] = 0;
    m_is_homepage = false;
    m_is_output_xml = false;

}

void CHttpBuff::SetContentType(int32_t content_type) {
    if ( content_type == ENUM_HTTP_XML) {
        m_is_output_xml = true;
    }
}

// &xml=true&jasoncallback 才有效
void CHttpBuff::SetJasonCallback(const char* jasoncallback){
    m_jasoncallback.Reset();
    if(jasoncallback){
        // not the string as jasoncallback=""
        if(jasoncallback[0])
            m_jasoncallback.AppendStr(jasoncallback);
    }
}

void CHttpBuff::SetAttr(const char* title,
                        const char* module,
                        bool is_home_page) {
    if (title) {
        safe_snprintf(m_title, sizeof(m_title), "%s", title);
    }

    if (module) {
        safe_snprintf(m_module, sizeof(m_module), "%s", module);
    }

    m_is_homepage = is_home_page;
}

void CHttpBuff::SetBanner(const char* user_name, const char* logout_page) {
    if (user_name) {
        safe_snprintf(m_login_name, sizeof(m_login_name), "%s", user_name);
        m_is_set_login_name = true;
    }

    if (logout_page) {
        safe_snprintf(m_login_page, sizeof(m_login_page), "%s", logout_page);
        m_is_set_login_page = true;
    }
}

// 设置raw data,可以通过GetRawData 获取到raw data
void CHttpBuff::SetRawData(const char* raw_data, uint32_t len){
    m_is_raw_data = true;
    m_http_raw_data.Reset();
    m_http_raw_data.AppendStr(raw_data, len);
}

bool CHttpBuff::GetRawData(const char** out_content, uint32_t* out_content_len){
    m_is_raw_data = false;
    bool is_ok = false;
    if(out_content && out_content_len){
        *out_content_len = m_http_raw_data.GetValidLen();
        *out_content = m_http_raw_data.GetString();
        is_ok = (*out_content_len > 0 && *out_content);
    }
    return is_ok;
}

void CHttpBuff::BeginGroup(const char* key, uint32_t property_val){
    char key_txt[256] = {0};
    if(m_is_output_xml)
        safe_snprintf(key_txt, sizeof(key_txt), "%s val='%u'", key ? key : "", property_val);
    else
        safe_snprintf(key_txt, sizeof(key_txt), "%s %u", key ? key : "", property_val);

    BeginGroup(key_txt);
}

void CHttpBuff::BeginGroup(const char* name) {
    if (!m_is_output_xml) {
        m_is_group_model = true;

        char buf_tmp[100] = {0};
        safe_snprintf(buf_tmp, sizeof(buf_tmp), "%s_%d", name, m_div_idx);
        ++m_div_idx;

        char buf_block_id[100] = {0};
        safe_snprintf(buf_block_id, sizeof(buf_block_id), "display_block_num_%d", m_div_idx);

        // <div class='display_block' onclick='showhidediv("xfs_build_revision_num_0",
        // "display_block_num_1"); id='display_block_num_1'>
        m_http_body.AppendStr("\r\n");
        
        m_http_body.AppendStr("\r\n<div class='display_block' id='");
        m_http_body.AppendStr(buf_block_id);
        m_http_body.AppendStr("' onclick='showhidediv(\"");
        m_http_body.AppendStr(buf_tmp);
        m_http_body.AppendStr("\",\"");
        m_http_body.AppendStr(buf_block_id);
        m_http_body.AppendStr("\");'>\r\n");

        // make strings as follow
        // <span class='display_block_btn_drop'>xfs_build_revision_num</span>
        // <span  id='xfs_build_revision_num_556-tbl-state' class='display_block_tag_plus'></span>        
        m_http_body.AppendStr("<span class='display_block_btn_drop'>");
        m_http_body.AppendStr(name);
        m_http_body.AppendStr("</span><span id='");
        m_http_body.AppendStr(buf_tmp);
        m_http_body.AppendStr("-tbl-state' class='display_block_tag_plus'>"
                              "</span>");        

        m_http_body.AppendStr("</div>\r\n");
        m_http_body.AppendStr("<div id='");
        m_http_body.AppendStr(buf_tmp);
        m_http_body.AppendStr("' class='display_block_body'>\n"
                             " <table id='display_block_info_tbl-a' summary='XFS show info...'>\n"
                             " <colgroup>\n"
                             "  <col class='id-key'/>\n"
                             "  <col class='id-val'/>\n"                             
                             " </colgroup>\n"
                              "  <thead>\n"
                              "    <tr><th scope='col' colspan='2'></th></tr>\n"
                              "  </thead>\n"
                              "  <tbody>\n");

    } else {
        m_http_body.AppendStr("<");
        m_http_body.AppendStr(name);
        m_http_body.AppendStr(">\r\n");
    }
}
void CHttpBuff::EndGroup(const char* name) {
    if (!m_is_output_xml) {
        m_is_group_model = false;

        m_http_body.AppendStr("  </tbody>\n"
                              " </table>\n"
                              "</div>\n"
                              "<div class='space_line'></div>\n");
    } else {
        m_http_body.AppendStr("</");
        m_http_body.AppendStr(name);        
        m_http_body.AppendStr(">\r\n");
    }
}

void CHttpBuff::SetProxy(const char* proxy_url) {
    safe_snprintf(m_url_proxy, sizeof(m_url_proxy), "%s", proxy_url);
}
void CHttpBuff::SetHome(const char* home_url) {
    safe_snprintf(m_url_homepage, sizeof(m_url_homepage), "%s", home_url);
}

void CHttpBuff::SetHostPort(const char* host_port) {
    safe_snprintf(m_host_port, sizeof(m_host_port), "%s", host_port);
}

void CHttpBuff::SetXslFile(const char* xsl_filename) {
    safe_snprintf(m_xsl_filename, sizeof(m_xsl_filename), "%s", xsl_filename);
}

void CHttpBuff::AddKey(const char* key, const char* value, const char* desc,
                       bool new_line,
                       const char* xml_key) {
    if (key == NULL || value == NULL || desc == NULL) {
        return;
    }

    uint32_t key_len = STRLEN(key);
    if (!m_is_output_xml) {
        // format:
        //
        // <tr>
        //	<td>key</td>
        //  <td><span class='content-val'>value ...</span><span class='comment_txt'></span></td>
        // </tr>

        // row begin
        m_http_body.AppendStr("    <tr>\n");

        // key        
        m_http_body.AppendStr("     <td class='id-key'>");
        m_http_body.AppendStr(key);
        m_http_body.AppendStr("</td>\n");

        // format: value + comments
        // <td class='id-val'>system_info.html, show system information ...
        //     <span class='comment_txt'>welcome to China...</span>
        // </td>

        // value
        m_http_body.AppendStr("     <td class='id-val'><span class='content-val'>");
        m_http_body.AppendStr(value);
        m_http_body.AppendStr("</span>");
        // 如果desc存在就输出comment
        if(desc && desc[0]){
            m_http_body.AppendStr("<span class='comment_txt'>");
            m_http_body.AppendStr(desc);
            m_http_body.AppendStr("</span>");
        }
        m_http_body.AppendStr("</td>\n");

        // row end
        m_http_body.AppendStr("    </tr>\n");
        
        // group模式下是使用的表格,换行无效
        if(!m_is_group_model){
            if(new_line)
                m_http_body.AppendStr("<br>\n");
        }

    } else {
        m_http_body.AppendStr("    <");

        // 如果xml key存在就取xml key
        const char* real_key = key;
        if(xml_key){
            real_key = xml_key;
            key_len = STRLEN(xml_key);
        }

        if (real_key[0])
            FormatXmlTag(real_key, key_len, m_http_body);
        else 
            m_http_body.AppendStr("autokey");        

        if (desc[0]) {
            m_http_body.AppendStr(" desc='");
            m_http_body.AppendStr(desc);
            m_http_body.AppendStr("'");
        }

        m_http_body.AppendStr(">");

        FormatXmlString(value, strlen(value), m_http_body);

        m_http_body.AppendStr("</");

        if (real_key[0])
            FormatXmlTag(real_key, key_len, m_http_body);
        else
            m_http_body.AppendStr("autokey");        

        m_http_body.AppendStr(">\r\n");
    }
}

//
// key-value-desc 形式 
//
// title,href组成key
// value
// desc 注释字段
//
// 将内部的href连接作为key加入
// 需要计算出真正的url后才作为key加入
//
void CHttpBuff::AddInternalHrefKey(const char* title, const char* href,
                                   const char* value ,const char* desc){

    title = title ? title : "";
    href = href ? href : "";
    desc = desc ? desc : "";    
    
    char url[1024] = {0};
    char link[1024] = {0};
    MakeRealURL(href, STRLEN(href), url, sizeof(url));
    safe_snprintf(link, sizeof(link), "<a href='%s'>%s</a>", url, title);
    const char* xml_key = title;
    AddKey(link, value, desc, true, xml_key);
}

void CHttpBuff::AddKey(const char* key, const uint16_t value, const char* desc) {
    char tmp[128];
    safe_snprintf(tmp, sizeof(tmp), "%u", value);

    AddKey(key, tmp, desc);
}

void CHttpBuff::AddKey(const char* key, const uint32_t value, const char* desc) {
    char tmp[128];
    safe_snprintf(tmp, sizeof(tmp), "%u", value);

    AddKey(key, tmp, desc);
}

void CHttpBuff::AddKey(const char* key, const uint64_t value, const char* desc) {
    char tmp[128];
    safe_snprintf(tmp, sizeof(tmp), "%"FMTu64, value);

    AddKey(key, tmp, desc);
}

void CHttpBuff::AddKey(const char* key, const float value, const char* desc) {
    char tmp[128];
    safe_snprintf(tmp, sizeof(tmp), "%.2f", value);

    AddKey(key, tmp, desc);
}

// 增加一条彩色线
void CHttpBuff::AddLineKey(bool new_line) {
    const char* line = "<hr class='xfs_lite_line' align='left'>\n";
    AddKey("", line, "", new_line);
}

void CHttpBuff::AddHref(const char* title, const char* href, const char* desc, bool is_xml,
                        bool new_line) {
    if (title == NULL || href == NULL) {
        return;
    }

    if (title[0]  == 0 || href[0]  == 0) {
        return;
    }

    bool is_home_page = (strcmp(XFS_HOME_PAGE_NAME, title)  == 0);

    const uint32_t kMaxUrlLen = 1024 + 4096;
    char buf_url[kMaxUrlLen] = {0};

    if( !MakeRealURL(href,STRLEN(href), buf_url, kMaxUrlLen, is_xml, is_home_page)) {
        return;
    }

    if (!m_is_output_xml) {
        char buf_html[1500];
        uint32_t len = safe_snprintf(buf_html, sizeof(buf_html),
                                     "<a href=\"%s\">%s</a>",
                                     buf_url, title);
        buf_html[len] = 0;

        m_http_body.AppendStr(buf_html);

        // 不管有没有desc,至少补充一个空格
        m_http_body.AppendStr("&nbsp;");
        if (desc[0]  != 0) {
            int32_t blank_len = 20 - static_cast<int32_t>(strlen(title)) - 1;
            for (int32_t i = 0; i < blank_len; i++) {
                m_http_body.AppendStr("&nbsp;");
            }
            m_http_body.AppendStr(desc);
        }

        // 添加换行标志
        if(new_line)
            m_http_body.AppendStr("<br>\r\n");

    } else {

        m_http_body.AppendStr("<Link>\r\n");
        m_http_body.AppendStr("    <Title>");

        FormatXmlString(title, strlen(title), m_http_body);


        m_http_body.AppendStr("</Title>\r\n");
        m_http_body.AppendStr("    <Url>");

        FormatXmlString(buf_url, strlen(buf_url), m_http_body);

        m_http_body.AppendStr("</Url>\r\n");

        if (desc) {
            m_http_body.AppendStr("    <Desc>");

            FormatXmlString(desc, strlen(desc), m_http_body);

            m_http_body.AppendStr("</Desc>\r\n");
        }

        m_http_body.AppendStr("</Link>\r\n");
    }
}

bool CHttpBuff::MakeRealURL(const char* in_url, uint32_t in_url_len,
                            char* out_url, uint32_t out_url_len,
                            bool is_xml, bool is_homepage) {
    if( !in_url || !out_url || out_url_len == 0) {
        return false;
    }

    uint32_t buf_url_max_len = out_url_len;
    uint32_t real_len = 0;

    char sign = '?';

    // <proxy> realurl ?(&) randkey home <key=value>

    // ? add proxy
    if (m_url_proxy[0] != 0 && is_homepage == false) {
        real_len = safe_snprintf(out_url + real_len,
                                 buf_url_max_len - real_len,
                                 "%s?realserver=",
                                 m_url_proxy);
        sign = '&';
    }

    // randkey
    char rand_buf[128];
    uint32_t rand_buf_len = safe_snprintf(rand_buf,
                                          sizeof(rand_buf),
                                          "randkey=%d",
                                          safe_rand() % 100);

    uint32_t copy_len = in_url_len;

    if (copy_len > (buf_url_max_len - real_len - rand_buf_len)) {
        LOG(WARNING) << "url too long:" << in_url;
        return false;
    }

    const char* param_vals = strstr(in_url, "?");

    if (param_vals) {
        copy_len = param_vals - in_url;
    }

    // copy realurl
    memcpy(out_url + real_len, in_url, copy_len);
    real_len += copy_len;

    // copy &
    out_url[real_len] = sign;
    real_len++;

    // copy randkey
    memcpy(out_url + real_len, rand_buf, rand_buf_len);
    real_len += rand_buf_len;

    // copy home
    // 不输出home(m_url_homepage),不会影响结果
    // 只输出调试信息    
    // real_len += safe_snprintf(out_url + real_len, buf_url_max_len - real_len - 1,
    //                          "&have_home=%s",  m_url_homepage[0] ? "yes" : "no");

    // 不输出proxy值,只输出一个供调试用的信息
    // 如果要输出proxy,输出形式为&proxy=value of m_url_proxy
    // 输出proxy会让URL太长,比较难看,输出不输出都不影响结果    
    real_len += safe_snprintf(out_url + real_len, buf_url_max_len - real_len - 1,
                              "&have_proxy=%s", m_url_proxy[0] ? "yes" : "no");


    if (out_url[real_len-1] == '?') {
        out_url[real_len-1] = 0;
        real_len --;
    }

    if (param_vals) {
        param_vals++;

        m_cgi_parser.ResetContent();
        m_cgi_parser.AttachEnvironmentString(param_vals);
        int32_t keys = m_cgi_parser.GetParameterCount();

        for (int32_t i = 0; i < keys; i++) {
            if (strcmp(m_cgi_parser.GetParameterName(i), "randkey")  != 0 &&
                    strcmp(m_cgi_parser.GetParameterName(i), "home")  != 0 &&
                    strcmp(m_cgi_parser.GetParameterName(i), "realserver")  != 0) {
                real_len += safe_snprintf(out_url + real_len,
                                          buf_url_max_len - real_len - 1,
                                          "&%s=%s",
                                          m_cgi_parser.GetParameterName(i),
                                          m_cgi_parser.GetParameterVal(i));
            }
        }
    }

    // copy is_xml
    if (is_xml == true) {
        real_len += safe_snprintf(out_url + real_len,
                                  buf_url_max_len - real_len - 1,
                                  "%s", "&xml=true");
    }

    out_url[real_len] = 0;

    return true;
}

bool CHttpBuff::GetHead(const char** out_content, uint32_t* out_content_len) {
    char process[256] = {0};
    GetModuleFileName(NULL, process, sizeof(process));

    if (!m_is_output_xml) {
        const char* k_http_tmp1 =
            "<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN'"
            " 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd'>\n"
            "<html xmlns='http://www.w3.org/1999/xhtml'>\n"            
            "<head>\n"
            "<meta http-equiv=\"Content-Type\" "
            "content='text/html; charset=utf-8'>\n"
            "<meta http-equiv='Content-Language' "
            "content='zh-cn'>\r\n"
            "<link href='http://xfs.soso.oa.com/xfs/css/xfs_style.css' "
            "type='text/css' rel='stylesheet'/>\n"
            "<script src='http://xfs.soso.oa.com/xfs/js/xfs.js'></script>\n"
            "<title>";

        const char* k_http_tmp2 =
            "</title>\n"
            "<script language='javascript'>\n";

        const char* k_http_tmp4 =            
            "</script>\n"       
            "</head>\n"
            "<body>";

        const char* k_http_tmp5 = "<div align='right'>Welcome&nbsp;";

        const char* k_http_tmp6 = "&nbsp; | <a onClick = 'Logout()' href ='#'>Logout</a></div>";

        const char* k_http_tmp7 = "<HR class='line_3' align='left'>\n"
                                  "<span class='h1_text'>XFS., "
                                  "  DREAM STARTS HERE . . .</span><br>\n"
                                  "<HR class='line_1' align='left'>\n"
                                  "<br>\n"
                                  "<div><strong><span class='module_title'>XFS Module</span>"
                                  "<span class='module_gt'>&gt;</span><span class='module'>";


        const char* k_http_tmp8 = "</span></strong>\n";
        const char* k_http_tmp9 = "<br><br>\n";

        m_http_head.Reset();

        m_http_head.AppendStr(k_http_tmp1);
        m_http_head.AppendStr(m_title);
        m_http_head.AppendStr(k_http_tmp2);

        // 用户注销后再登陆时跳转到的页面，如果不设置，则跳转到注销前的当前页面
        if (m_is_set_login_page) {
            m_http_head.AppendStr("var g_ca_user_set_redirect_uri='");
            m_http_head.AppendStr(m_login_page);
            m_http_head.AppendStr("';");
        } else {
            m_http_head.AppendStr("var g_ca_user_set_redirect_uri='';");            
        }

        m_http_head.AppendStr(k_http_tmp4);

        if (m_is_set_login_name) {
            m_http_head.AppendStr(k_http_tmp5);
            m_http_head.AppendStr(m_login_name);
            m_http_head.AppendStr(k_http_tmp6);
        }

        m_http_head.AppendStr(k_http_tmp7);
        m_http_head.AppendStr(g_lib_vars.m_http_module_name);
        m_http_head.AppendStr(k_http_tmp8);

        // cluster name
        char sz[1024] = {0};
        safe_snprintf(sz, sizeof(sz),
                     "<span class='cluster_info' id='id_cluster_name'>cluster name | "
                     "<a href='http://xfs.soso.oa.com/cgi-bin/XFSEntry.cgi?cluster=%s'>%s</a>"
                     "</span>\n", FLAGS_xfs_cluster_name.c_str(), FLAGS_xfs_cluster_name.c_str());
        m_http_head.AppendStr(sz);

        // process      
        safe_snprintf(sz, sizeof(sz),
                      "<span class='cluster_info' id='id_process_cmd'>cmd | %s </span>\n</div>\n",
                      process);
        m_http_head.AppendStr(sz);
        
        m_http_head.AppendStr(k_http_tmp9);

        *out_content_len = m_http_head.GetValidLen();
        *out_content = m_http_head.GetString();

    } else { // output xml style

        // ? have set jasoncallback
        if(HaveJasonCallback()){
            m_http_head.AppendStr(m_jasoncallback.GetString());
            m_http_head.AppendStr("(\"");
        }
        
        m_http_head.AppendStr("<?xml version='1.0' encoding='utf-8'?>\r\n");

        m_http_head.AppendStr("<?xml-stylesheet type='text/xsl' href='");
        m_http_head.AppendStr(m_url_proxy);
        m_http_head.AppendStr((STRLEN(m_url_proxy) ? "?realserver=" : ""));
        m_http_head.AppendStr("http://");
        m_http_head.AppendStr(m_host_port);
        m_http_head.AppendStr("/");

        if (m_xsl_filename[0]) {
            m_http_head.AppendStr(m_xsl_filename);
            m_http_head.AppendStr("'?>");

        } else {
            m_http_head.AppendStr("default.xsl' ?>");
        }

        m_http_head.AppendStr("\n<Content>\r\n");
        m_http_head.AppendStr("<Module>");
        FormatXmlString(m_module, strlen(m_module), m_http_head);
        m_http_head.AppendStr("</Module>\r\n");
        m_http_head.AppendStr("<Process>");
        FormatXmlString(process, STRLEN(process), m_http_head);
        m_http_head.AppendStr("</Process>\r\n");

        if(HaveJasonCallback())
            FormatToJavascriptString(m_http_head);
    }

    *out_content_len = m_http_head.GetValidLen();
    *out_content = m_http_head.GetString();
    return true;
}

bool CHttpBuff::GetBody(const char** out_content, uint32_t* out_content_len) {
    // footer line
    const char* split_line = "\n<br><hr class='xfs_line' align='left'>\n";

    if (!m_is_output_xml) {
        m_http_body.AppendStr(split_line);
    }

    if ( !m_is_homepage ) {
        char link[1024];
        safe_snprintf(link, sizeof(link), "http://%s/index.html", m_host_port);
        AddHref(m_module, link);
    }

    if(m_is_output_xml) {
        if (m_url_homepage[0]) {
            AddHref(XFS_HOME_PAGE_NAME, m_url_homepage, XFS_HOME_PAGE_NAME);
        }
    } else {
        // 输出footer
        const uint32_t kMaxUrlLen = 1024;
        char link[kMaxUrlLen];
        char about_url[kMaxUrlLen] = {0};
        char sys_url[kMaxUrlLen] = {0};        

        uint32_t len = safe_snprintf(link, sizeof(link), "http://%s/about.html", m_host_port);
        MakeRealURL(link,len, about_url, kMaxUrlLen);

        len = safe_snprintf(link, sizeof(link), "http://%s/system_info.html", m_host_port);
        MakeRealURL(link, len, sys_url, kMaxUrlLen);

        char host[64] = {0};
        strcpy(host, m_host_port);
        char* ptr = strstr(host, ":");
        if(ptr) *ptr = 0;        

        len = safe_snprintf(m_footer_buffer, MAX_FOOTER_BUFF_LENGTH,
                                    "\n<div class='xfs_footer'>\n"
                                    "<span class='xfs_copyright_txt'>"
                                    "Copyright &copy; 2011 Tencent XFS Team.,</span><br>\n"
                                    "<span class='xfs_footer_about'>\n"
                                    "<a href='%s'>XFS</a>&nbsp;|&nbsp;\n"
                                    "<a href='%s'>Server state</a>&nbsp;|&nbsp;\n"
                                    "<a href='http://tnm2.oa.com/host/home/%s'>Machine info on TNM</a> | \n"
                                    "<a href='%s'>About</a>\n"
                                    "</span>\n"
                                    "</div>\n"
                                    "</body>\n", m_url_homepage, sys_url, host, about_url);
        m_http_body.AppendStr(m_footer_buffer, static_cast<uint32_t>(len));

    }

    if(HaveJasonCallback())
        FormatToJavascriptString(m_http_body);

    *out_content_len = m_http_body.GetValidLen();
    *out_content = m_http_body.GetString();
    return true;
}

bool CHttpBuff::GetTail(const char** out_content, uint32_t* out_content_len) {
    if ( !m_is_output_xml) {
        m_http_tail.AppendStr("\r\n</html>\r\n\r\n");
    } else { // output xml format
        m_http_tail.AppendStr("</Content>");

        // ? have set jasoncallback
        if(HaveJasonCallback()){            
            m_http_tail.AppendStr("\");");
        }

        if(HaveJasonCallback())
            FormatToJavascriptString(m_http_tail);

        m_http_tail.AppendStr("\r\n\r\n");
    }

    *out_content_len = m_http_tail.GetValidLen();
    *out_content = m_http_tail.GetString();

    return true;
}

// 将中间的\r\n换着空格
void CHttpBuff::FormatToJavascriptString(CStrBuff& buff){
    buff.ChangeNLRT2Space();
}

void CHttpBuff::FormatXmlString(const char* in_buff, uint32_t in_buff_len,
                                CStrBuff& out_strbuff) {
    const char* p = in_buff;

    while ( in_buff_len > 0) {
        if ( *p == '>') {
            out_strbuff.AppendStr("&gt;");

        } else if ( *p == '<') {
            out_strbuff.AppendStr("&lt;");

        } else if ( *p == '&') {
            out_strbuff.AppendStr("&amp;");

        } else if ( *p == '\"') {
            out_strbuff.AppendStr("&quot;");

        } else if ( *p == '\'') {
            out_strbuff.AppendStr("&apos;");

        } else {
            out_strbuff.AppendStr(*p);
        }

        in_buff_len--;
        p++;
    }
}

void CHttpBuff::FormatXmlTag(const char* in_buff,
                             uint32_t    in_buff_len,
                             CStrBuff&   out_strbuff) {
    const char* p = in_buff;

    if( isdigit(*p) ) {
        out_strbuff.AppendStr((unsigned char)'_');
    }

    while ( in_buff_len > 0) {
        const char v = *p;

        //
        // 增加 (){}? 过滤
        // wookin
        //
        if ( v == ' ' || v == ':' || v == '<'  || v == '#'  || v == '/' || v == '+' ||
                v == '-' || v == '>' || v == '=' || v == '\"' || v == '\'' || v == ',' ||
                v == '(' || v == ')' || v == '{' || v == '}'  || v == '?'  || v == ';' ||
                v == '.' || v == '&') {
            out_strbuff.AppendStr((unsigned char)'_');

        } else {
            out_strbuff.AppendStr(v);
        }

        in_buff_len--;
        p++;
    }
}

_END_XFS_BASE_NAMESPACE_
