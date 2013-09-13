#include "common/config/ini/ini_file.h"

using namespace std;

#ifdef _WIN32
#define snprintf _snprintf
#endif // _WIN32

IniFile::IniFile(const string &path) {
    m_path = path;
    m_ignore_case = true;
}

bool IniFile::ReadFile() {
    // Normally you would use ifstream, but the SGI CC compiler has
    // a few bugs with ifstream. So ... fstream used.
    // 打开文件
    ifstream f;
    f.open(m_path.c_str(), std::ios::in);
    if (f.fail())
        return false;

    // 读取文件
    string line;
    string key_name, value_name, value;
    string::size_type left_position, right_position;
    while (getline(f, line)) {
        // add_begin@sword 2007.6.4 当读取为空时，跳过。
        if (0 == line.length())
            continue;

        // To be compatible with Win32, check for existence of '\r'.
        // Win32 files have the '\r' and Unix files don't at the end of a line.
        // Note that the '\r' will be written to INI files from
        // Unix so that the created INI file can be read under Win32
        // without change.
        if (line[line.length() - 1] == '\r')
            line = line.substr(0, line.length() - 1);

        if (line.length()) {
            // Check that the user hasn't openned a binary file by checking
            // the first character of each line!
            if (!isprint(line[0])) {
                fprintf(stderr, "Failing on char %d\n", line[0]);
                f.close();
                return false;
            }

            if ((left_position = line.find_first_of(";#[=")) != string::npos) {
                switch (line[left_position]) {
                case '[':
                    if ((right_position = line.find_last_of("]")) !=
                      string::npos && right_position > left_position) {
                        key_name = line.substr(left_position + 1,
                                        right_position - left_position - 1);
                        AddKeyName(key_name);
                    }
                    break;

                case '=':
                    value_name = line.substr(0, left_position);
                    value = line.substr(left_position + 1);
                    SetValue(key_name, value_name, value);
                    break;

                case ';':
                case '#':
                    if (!m_fields.size())
                        AddHeaderComment(line.substr(left_position + 1));
                    else
                        AddKeyComment(key_name, line.substr(left_position + 1));
                    break;

                default:
                    break;
                }
            }
        }
    }

    f.close();

    if (m_fields.size())
        return true;

    return false;
}

// open_mode true-写 false-追加
bool IniFile::WriteFile(bool open_mode) {
    // Normally you would use ofstream, but the SGI CC compiler has
    // a few bugs with ofstream. So ... fstream used.
    ofstream f;
    if (open_mode == true) {
        f.open(m_path.c_str(), ios::out);
    } else {
        f.open(m_path.c_str(), ios::out | ios::app);
    }

    if (f.fail())
        return false;

    // Write header comments.
    for (unsigned comment_id = 0; comment_id < m_head_commonts.size();
      ++comment_id) {
        f << ';' << m_head_commonts[comment_id] << endl;
    }

    if (m_head_commonts.size())
        f << endl;

    // Write m_key_value_pair and values.
    for (unsigned key_id = 0; key_id < m_key_value_pair.size();
      ++key_id) {
        f << '[' << m_fields[key_id] << ']' << endl;

        // Comments.
        for (unsigned comment_id = 0; comment_id <
          m_key_value_pair[key_id].comments.size(); ++comment_id) {
            f << ';' << m_key_value_pair[key_id].comments[comment_id] << endl;
        }

        // Values.
        for (unsigned value_id = 0; value_id <
          m_key_value_pair[key_id].names.size(); ++value_id) {
            f << m_key_value_pair[key_id].names[value_id] << '='
              << m_key_value_pair[key_id].values[value_id] << endl;
        }

        f << endl;
    }

    f.close();
    return true;
}

int IniFile::FindKey(const string &key_name) const {
    for (unsigned key_id = 0; key_id < m_fields.size(); ++key_id)
    {
        if (CheckCase(m_fields[key_id]) == CheckCase(key_name))
            return static_cast<int>(key_id);
    }

    return kIDNotFound;
}

int IniFile::FindValue(uint32_t key_id, const string &value_name) const {
    if (!m_key_value_pair.size() || key_id >= m_key_value_pair.size())
        return kIDNotFound;

    for (unsigned value_id = 0; value_id <
      m_key_value_pair[key_id].names.size(); ++value_id) {
        if (CheckCase(m_key_value_pair[key_id].names[value_id]) ==
          CheckCase(value_name)) {
            return static_cast<int>(value_id);
        }
    }

    return kIDNotFound;
}

unsigned IniFile::AddKeyName(const string &key_name) {
    m_fields.resize(m_fields.size() + 1, key_name);
    m_key_value_pair.resize(m_key_value_pair.size() + 1);

    return m_fields.size() - 1;
}

string IniFile::KeyName(uint32_t key_id) const {
    if (key_id < m_fields.size())
        return m_fields[key_id];

    return "";
}

unsigned IniFile::NumValues(uint32_t key_id) const {
    if (key_id < m_key_value_pair.size())
        return m_key_value_pair[key_id].names.size();

    return 0;
}

unsigned IniFile::NumValues(const string &key_name) const {
    int key_id = FindKey(key_name);
    if (key_id == kIDNotFound)
        return 0;

    return NumValues(key_id);
}

string IniFile::ValueName(uint32_t key_id, uint32_t value_id) const {
    if (key_id < m_key_value_pair.size() &&
      value_id < m_key_value_pair[key_id].names.size())
        return m_key_value_pair[key_id].names[value_id];

    return "";
}

string IniFile::ValueName(const string &key_name, uint32_t value_id) const {
    int key_id = FindKey(key_name);
    if (key_id == kIDNotFound)
        return "";

    return ValueName(key_id, value_id);
}

bool IniFile::SetValue(uint32_t key_id, uint32_t value_id, const string &value) {
    if (key_id < m_key_value_pair.size() &&
      value_id < m_key_value_pair[key_id].names.size()) {
        m_key_value_pair[key_id].values[value_id] = value;
        return true;
    }

    return false;
}

bool IniFile::SetValue(const string &key_name, const string &value_name,
                       const string &value, bool create) {
    int key_id = FindKey(key_name);
    if (key_id == kIDNotFound) {
        if (!create)
            return false;

        key_id = static_cast<int>(AddKeyName(key_name));
    }

    int value_id = FindValue(static_cast<unsigned>(key_id), value_name);
    if (value_id == kIDNotFound) {
        if (!create)
            return false;

        m_key_value_pair[key_id].names.resize(
            m_key_value_pair[key_id].names.size() + 1, value_name);
        m_key_value_pair[key_id].values.resize(
            m_key_value_pair[key_id].values.size() + 1, value);
    } else {
        m_key_value_pair[key_id].values[value_id] = value;
    }

    return true;
}

bool IniFile::SetValueI(const string &key_name, const string &value_name,
                        int value, bool create) {
    char svalue[kValueData];
    snprintf(svalue, sizeof(svalue), "%d", value);

    return SetValue(key_name, value_name, svalue);
}

bool IniFile::SetValueB(const string &key_name, const string &valuename,
               bool value, bool create) {
    return SetValueI(key_name, valuename, static_cast<int>(value), create);
}

bool IniFile::SetValueF(const string &key_name, const string &value_name,
                        double value, bool create) {
    char svalue[kValueData];
    snprintf(svalue, sizeof(svalue), "%f", value);

    return SetValue(key_name, value_name, svalue);
}

bool IniFile::SetValueV(const string &key_name, const string &value_name,
                        char *format, ...) {
    va_list args;
    va_start(args, format);

    char value[kValueData];
    vsprintf(value, format, args);

    va_end(args);

    return SetValue( key_name, value_name, value);
}

string IniFile::GetValue(uint32_t key_id, uint32_t value_id,
                         const string &default_value) const {
    if (key_id < m_key_value_pair.size() &&
      value_id < m_key_value_pair[key_id].names.size())
        return m_key_value_pair[key_id].values[value_id];

    return default_value;
}

string IniFile::GetValue(const string &key_name, const string &value_name,
                         const string &default_value) const {
    int key_id = FindKey(key_name);
    if (key_id == kIDNotFound)
        return default_value;

    int value_id = FindValue(static_cast<unsigned>(key_id), value_name);
    if (value_id == kIDNotFound)
        return default_value;

    return m_key_value_pair[key_id].values[value_id];
}

int IniFile::GetValueI(const string &key_name, const string &value_name,
                       int default_value) const {
    char svalue[kValueData];
    snprintf(svalue, sizeof(svalue), "%d", default_value);

    return atoi(GetValue(key_name, value_name, svalue).c_str());
}

bool IniFile::GetValueB(const string &key_name, const string &valuename,
                        bool default_value) const {
    int result = GetValueI(key_name, valuename,
                           static_cast<int>(default_value));
    return static_cast<bool>(result);
}

double IniFile::GetValueF(const string &key_name, const string &value_name,
                          double default_value) const {
    char svalue[kValueData];
    snprintf(svalue, sizeof(svalue), "%f", default_value);

    return atof(GetValue(key_name, value_name, svalue).c_str());
}

// 16 variables may be a bit of over kill, but hey, it's only code.
template <typename T1, typename T2, typename T3, typename T4,
        typename T5, typename T6, typename T7, typename T8,
        typename T9, typename T10, typename T11, typename T12,
        typename T13, typename T14, typename T15, typename T16>
unsigned IniFile::GetValueV(const string &key_name, const string &value_name,
                       const char *format,
                       T1 *v1, T2 *v2, T3 *v3, T4 *v4,
                       T5 *v5, T6 *v6, T7 *v7, T8 *v8,
                       T9 *v9, T10 *v10, T11 *v11,
                       T12 *v12, T13 *v13, T14 *v14,
                            T15 *v15, T16 *v16) const
{
    string value = GetValue(key_name, value_name);
    if (!value.length())
        return false;

    // Why is there not vsscanf() function. Linux man pages say that there is
    // but no compiler I've seen has it defined. Bummer!
    //
    // va_start( args, format);
    // nVals = vsscanf( value.c_str(), format, args);
    // va_end( args);

    unsigned nVals = sscanf(value.c_str(), format,
                            v1, v2, v3, v4, v5, v6, v7, v8,
                            v9, v10, v11, v12, v13, v14, v15, v16);

    return nVals;
}

bool IniFile::DeleteValue(const string &key_name, const string &value_name) {
    int key_id = FindKey(key_name);
    if (key_id == kIDNotFound)
        return false;

    int value_id = FindValue(static_cast<unsigned>(key_id), value_name);
    if (value_id == kIDNotFound)
        return false;

    // This looks strange, but is neccessary.
    vector<string>::iterator npos =
        m_key_value_pair[key_id].names.begin() + value_id;
    vector<string>::iterator vpos =
        m_key_value_pair[key_id].values.begin() + value_id;

    m_key_value_pair[key_id].names.erase(npos, npos + 1);
    m_key_value_pair[key_id].values.erase(vpos, vpos + 1);

    return true;
}

bool IniFile::DeleteKey(const string &key_name) {
    int key_id = FindKey(key_name);
    if (key_id == kIDNotFound)
        return false;

    // Now hopefully this destroys the vector lists within m_key_value_pair.
    // Looking at <vector> source, this should be the case using the destructor.
    // If not, I may have to do it explicitly. Memory leak check should tell.
    // memleak_test.cpp shows that the following not required.
    // m_key_value_pair[key_id].m_fields.clear();
    // m_key_value_pair[key_id].values.clear();

    vector<string>::iterator       npos = m_fields.begin() + key_id;
    vector<KeyValuePair>::iterator kpos = m_key_value_pair.begin() + key_id;

    m_fields.erase(npos, npos + 1);
    m_key_value_pair.erase(kpos, kpos + 1);

    return true;
}

void IniFile::Erase() {
    // This loop not needed. The vector<> destructor seems to do
    // all the work itself. memleak_test.cpp shows this.
    // for ( unsigned i = 0; i < m_key_value_pair.size(); ++i) {
    // m_key_value_pair[i].m_fields.clear();
    // m_key_value_pair[i].values.clear();
    // }
    m_fields.clear();
    m_key_value_pair.clear();
    m_head_commonts.clear();
}

void IniFile::Clear() {
    Erase();
}

void IniFile::Reset() {
    Erase();
}

void IniFile::AddHeaderComment(const string &comment) {
    m_head_commonts.resize(m_head_commonts.size() + 1, comment);
}

string IniFile::HeaderComment(uint32_t comment_id) const {
    if (comment_id < m_head_commonts.size())
        return m_head_commonts[comment_id];

    return "";
}

bool IniFile::DeleteHeaderComment(unsigned comment_id) {
    if (comment_id < m_head_commonts.size()) {
      vector<string>::iterator cpos = m_head_commonts.begin() + comment_id;
        m_head_commonts.erase(cpos, cpos + 1);
        return true;
    }

    return false;
}

unsigned IniFile::NumKeyComments(uint32_t key_id) const {
    if (key_id < m_key_value_pair.size())
        return m_key_value_pair[key_id].comments.size();

    return 0;
}

unsigned IniFile::NumKeyComments(const string &key_name) const {
    int key_id = FindKey(key_name);
    if (key_id == kIDNotFound)
        return 0;

    return m_key_value_pair[key_id].comments.size();
}

bool IniFile::AddKeyComment(uint32_t key_id, const string &comment) {
    if (key_id < m_key_value_pair.size()) {
        m_key_value_pair[key_id].comments.resize(m_key_value_pair[key_id].comments.size() + 1, comment);
        return true;
    }

    return false;
}

bool IniFile::AddKeyComment(const string &key_name, const string &comment) {
    int key_id = FindKey(key_name);
    if (key_id == kIDNotFound)
        return false;

    return AddKeyComment(static_cast<unsigned>(key_id), comment);
}

string IniFile::KeyComment(uint32_t key_id, uint32_t comment_id) const {
    if (key_id < m_key_value_pair.size() && comment_id < m_key_value_pair[key_id].comments.size())
        return m_key_value_pair[key_id].comments[comment_id];

    return "";
}

string IniFile::KeyComment(const string &key_name, uint32_t comment_id) const {
    int key_id = FindKey(key_name);
    if (key_id == kIDNotFound)
        return "";

    return KeyComment(static_cast<unsigned>(key_id), comment_id);
}

bool IniFile::DeleteKeyComment(uint32_t key_id, uint32_t comment_id) {
    if (key_id < m_key_value_pair.size() &&
      comment_id < m_key_value_pair[key_id].comments.size()) {
        vector<string>::iterator cpos = m_key_value_pair[key_id].comments.begin() + comment_id;
        m_key_value_pair[key_id].comments.erase(cpos, cpos + 1);
        return true;
    }

    return false;
}

bool IniFile::DeleteKeyComment(const string &key_name, uint32_t comment_id) {
    int key_id = FindKey(key_name);
    if (key_id == kIDNotFound)
        return false;

    return DeleteKeyComment(static_cast<unsigned>(key_id), comment_id);
}

bool IniFile::DeleteKeyComments(uint32_t key_id) {
    if (key_id < m_key_value_pair.size()) {
        m_key_value_pair[key_id].comments.clear();
        return true;
    }

    return false;
}

bool IniFile::DeleteKeyComments(const string &key_name) {
    int key_id = FindKey(key_name);
    if (key_id == kIDNotFound)
        return false;

    return DeleteKeyComments(static_cast<unsigned>(key_id));
}

string IniFile::CheckCase(string s) const {
    if (m_ignore_case)
        for (string::size_type i = 0; i < s.length(); ++i)
            s[i] = tolower(s[i]);

    return s;
}
