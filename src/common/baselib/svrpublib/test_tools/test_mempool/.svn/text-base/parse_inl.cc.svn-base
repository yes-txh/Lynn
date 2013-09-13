#include "common/baselib/svrpublib/test_tools/test_mempool/parse_inl.h"

void ParseProtocolInl::Parse(std::string file) {
    std::ifstream input(file.c_str());
    std::string line;
    int32_t val_default = 0;
    int32_t real_start = 0;
    int32_t cur_st = -1;
    bool is_first_st = true;
    while (getline(input,line)) {
        int32_t real_end = 0;
        while((real_end = line.find_first_of("//")) != std::string::npos) {
            line = line.erase(real_end);
        }

        for (int32_t i = 0; i != line.length();) {
            if (line[i] == ' ' || line[i] == '\t' || line[i] == ',') {
                line.erase(i,1);
            } else
                ++i;
        }

        if (line.empty() || line.size() == 0) {
            continue;
        }

        if (line.find_first_of("ST_") != 0 && 
            line.find_first_of("KEY_") != 0) {
            continue;
        }     

        // 开始定义枚举变量
        if (line.find("=") == std::string::npos) {         
            // 该枚举类型为
            if (line.find_first_of("ST_") == 0) {
                if (is_first_st) {
                    val_default = 0;
                    is_first_st = false;
                }
                st[val_default++] = line;   
            } else if (line.find_first_of("KEY_") == 0) {
                st_key[cur_st][val_default] = line;
                if (!is_first_st) {
                    is_first_st = true;
                }
            } else {
                LOG(ERROR) << "enum definition format is error";
            }
        } else {
            // 该行标识该enum中随后的key对应的ST(service_type)
            if (line.find_first_of("ST_") == 0 &&
                line.find("-1") != std::string::npos) {
                int32_t pos = line.find('=');
                line = line.erase(pos);

                std::map<int32_t,std::string>::iterator iter;
                for (iter = st.begin(); iter != st.end(); ++iter) {
                    if (iter->second == line) {
                        cur_st = iter->first;
                        break;
                    }
                }
                if (iter == st.end())
                    LOG(ERROR) << "enum definition format is error";
                val_default = 0;
            } else if (line.find_first_of("ST_") == 0) { // 该enum定义ST(service type)
                is_first_st = false;
                int32_t pos = line.find('=');
                std::string val = line.substr(0, pos);
                std::string key = line.substr(pos + 1);
                bool is_key_digit = true;
                for (int32_t i = 0; i != key.length(); ++i) {
                    if (isdigit(key[i]) == 0) {
                        is_key_digit = false;
                        break;
                    }
                }
                // st结构的key
                int32_t int_key;
                if (is_key_digit) {
                    int_key = atoi(key.c_str());
                } else {
                    std::map<int32_t,std::string>::iterator iter;
                    for (iter = st.begin(); iter != st.end(); ++iter) {
                        if (iter->second == key) {
                            int_key = iter->first;
                            break;
                        }
                    }
                    if (iter == st.end())
                        LOG(ERROR) << "enum definition format is error";
                }
                st[int_key] = val;
                val_default = int_key + 1;
            } else if (line.find_first_of("KEY_") == 0) { // 该enum定义某个ST中的key
                int32_t pos = line.find('=');
                std::string val = line.substr(0, pos);
                std::string key = line.substr(pos + 1);
                bool is_key_digit = true;
                for (int32_t i = 0; i != key.length(); ++i) {
                    if (isdigit(key[i]) == 0) {
                        is_key_digit = false;
                        break;
                    }
                }
                // st_key结构中cur_st对应的map的key
                int32_t int_key;
                if (is_key_digit) {
                    int_key = atoi(key.c_str());
                } else {
                    std::map<int32_t,std::string>::iterator iter;
                    for (iter = st.begin(); iter != st.end(); ++iter) {
                        if (iter->second == key) {
                            int_key = iter->first;
                            break;
                        }
                    }
                    if (iter == st.end())
                        LOG(ERROR) << "enum definition format is error";
                }
                st_key[cur_st][int_key] = val;
                val_default = int_key + 1;
            }
        }
    }
}

std::string ParseProtocolInl::GetServiceType(int32_t type) {
    std::map<int32_t, std::string>::iterator iter = st.find(type);
    if (iter == st.end()){
        return "";
    } else {
        return iter->second;
    }
}

std::string ParseProtocolInl::GetKey(int32_t type, int16_t key) {
    std::map<int32_t, std::map<uint16_t, std::string> >::iterator iter1 = st_key.find(type);
    if (iter1 == st_key.end()) {
        return "";
    } else {
        std::map<uint16_t, std::string>::iterator iter2 = iter1->second.find(type);
        if (iter2 == iter1->second.end()) {
            return "";
        } else {
            return iter2->second;
        }
    }
}

void ParseProtocolInl::Debug() {
    LOG(INFO) << "--------service type---start---";
    std::map<int32_t, std::string>::iterator iter1 = st.begin();
    for (; iter1 != st.end(); ++iter1)
    {
        std::cout<<iter1->first<<" : "<<iter1->second<<std::endl;
    }
    LOG(INFO) << "--------service type---end---";
    std::map<int32_t, std::map<uint16_t, std::string> >::iterator iter2 = st_key.begin();
    for (; iter2 != st_key.end(); ++iter2) {
        LOG(INFO) << "---------" << st[iter2->first] << "-------start-------";
        std::map<uint16_t, std::string>::iterator iter3 = iter2->second.begin();
        for (; iter3 != iter2->second.end(); ++iter3) {
            LOG(INFO) << iter3->first << " : " << iter3->second;
        }
        LOG(INFO) << "---------" << st[iter2->first] << "-------end-------";
    }
}