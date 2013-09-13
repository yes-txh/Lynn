// IniFile.cpp:  Implementation of the IniFile class.
// Written by:   Adam Clauss
// Email: cabadam@tamu.edu
// You may use this class/code as you wish in your programs.
// Feel free to distribute it, and
// email suggested changes to me.
//
// Rewritten by: Shane Hill
// Date:         21/08/2001
// Email:        Shane.Hill@dsto.defence.gov.au
// Reason:        Remove dependancy on MFC. Code should compile on any
//               platform. Tested on Windows/Linux/Irix
//////////////////////////////////////////////////////////////////////

/************************************************************
  Copyright (C), 1998-2006, Tencent Technology Commpany Limited

  文件名称: tseinifile.h
  作者: swordshao
  日期: 2006.9.19
  版本: 1.0
  模块描述: INI文件读写模块. 整理Shane Hill的实现.
  组成类:
      IniFile
  修改历史:
      <author>    <time>   <version >   <desc>
      swordshao  2006.9.19    1.0        创建
*************************************************************/

//////////////////////////////////////////////////////////////////////////
// modified by ivanhuang at 20101115
// 1.修改代码风格、增加注释、修改部分代码
// 2.增加UnitTest
//////////////////////////////////////////////////////////////////////////

#ifndef COMMON_CONFFIG_INI_INI_FILE_H_ // NOLINT
#define COMMON_CONFFIG_INI_INI_FILE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "common/base/inttypes.h"

enum IniFileErrorCode {
    kIDNotFound = -1,
};

enum IniFileStringLength {
    kKeyName   = 128,
    kValueName = 128,
    kValueData = 2048,
};

class IniFile {
public:
    explicit IniFile(const std::string &path = "");

    // Sets whether or not keynames and valuenames should be case sensitive.
    // The default is case insensitive.
    bool IgnoreCase() const              {return m_ignore_case;}
    void SetIgnoreCase(bool ignore_case) {m_ignore_case = ignore_case;}

    // Sets path of ini file to read and write from.
    std::string Path() const                {return m_path;}
    void   SetPath(const std::string &path) {m_path = path;}

    // Reads ini file specified using path.
    // Returns true if successful, false otherwise.
    bool ReadFile();

    // Writes data stored in class to ini file.
    // bOpenMode true-写 false-追加
    bool WriteFile(bool open_mode);

    // Deletes all stored ini data.
    void Erase();
    void Clear();
    void Reset();

    // Returns index of specified key, or kIDNotFound if not found.
    int FindKey(const std::string &key_name) const;

    // Returns index of specified value, in the specified key,
    // or kIDNotFound if not found.
    int FindValue(uint32_t key_id, const std::string &valuename) const;

    // Returns number of m_key_value_pair currently in the ini.
    unsigned NumKeys() const {return m_fields.size();}

    // Add a key name.
    unsigned AddKeyName(const std::string &key_name);

    // Returns key m_fields by index.
    std::string KeyName(uint32_t key_id) const;

    // Returns number of values stored for specified key.
    unsigned NumValues(uint32_t key_id) const;
    unsigned NumValues(const std::string &key_name) const;

    // Returns value name by index for a given key_name or key_id.
    std::string ValueName(uint32_t key_id, uint32_t value_id) const;
    std::string ValueName(const std::string &key_name, uint32_t value_id) const;

    // Gets value of [key_name] valuename =.
    // Overloaded to return std::string, int, and double.
    // Returns default_value if key/value not found.
    std::string  GetValue(uint32_t key_id, uint32_t value_id,
                    const std::string &default_value = "") const;
    std::string  GetValue(const std::string &key_name, const std::string &valuename,
                    const std::string &default_value = "") const;
    int     GetValueI(const std::string &key_name, const std::string &valuename,
                     int default_value = 0) const;
    bool    GetValueB(const std::string &key_name, const std::string &valuename,
                     bool default_value = false) const;
    double  GetValueF(const std::string &key_name, const std::string &valuename,
                       double default_value = 0.0) const;
    // This is a variable length formatted GetValue routine. All these voids
    // are required because there is no vsscanf() like there is a vsprintf().
    // Only a maximum of 8 variable can be read.
    template <typename T1, typename T2, typename T3, typename T4,
        typename T5, typename T6, typename T7, typename T8,
        typename T9, typename T10, typename T11, typename T12,
        typename T13, typename T14, typename T15, typename T16>
    unsigned GetValueV(const std::string &key_name, const std::string &value_name,
                       const char *format,
                       T1 *v1 = 0, T2 *v2 = 0, T3 *v3 = 0, T4 *v4 = 0,
                       T5 *v5 = 0, T6 *v6 = 0, T7 *v7 = 0, T8 *v8 = 0,
                       T9 *v9 = 0, T10 *v10 = 0, T11 *v11 = 0,
                       T12 *v12 = 0, T13 *v13 = 0, T14 *v14 = 0,
                       T15 *v15 = 0, T16 *v16 = 0) const;

    // Sets value of [key_name] valuename =.
    // Specify the optional paramter as false(0) if you don't want it to create
    // the key if it doesn't exist.Returns true if data enter, false otherwise
    // Overloaded to accept std::string, int, and double.
    bool SetValue(uint32_t key_id, uint32_t value_id, const std::string &value);
    bool SetValue(const std::string &key_name, const std::string &valuename,
                  const std::string &value, bool create = true);
    bool SetValueI(const std::string &key_name, const std::string &valuename,
                   int value, bool create = true);
    bool SetValueB(const std::string &key_name, const std::string &valuename,
                   bool value, bool create = true);
    bool SetValueF(const std::string &key_name, const std::string &valuename,
                   double value, bool create = true);
    bool SetValueV(const std::string &key_name, const std::string &valuename,
                   char *format, ...);

    // Deletes specified value.
    // Returns true if value existed and deleted, false otherwise.
    bool DeleteValue(const std::string &key_name, const std::string &valuename);

    // Deletes specified key and all values contained within.
    // Returns true if key existed and deleted, false otherwise.
    bool DeleteKey(const std::string &key_name);

    // Header comment functions.
    // Header m_head_commonts are those m_head_commonts before the first key.
    //
    // Number of header m_head_commonts.
    unsigned NumHeaderComments() const  {return m_head_commonts.size();}
    // Add a header comment.
    void     AddHeaderComment(const std::string &comment);
    // Return a header comment.
    std::string   HeaderComment(uint32_t comment_id) const;
    // Delete a header comment.
    bool     DeleteHeaderComment(unsigned comment_id);
    // Delete all header m_head_commonts.
    void     DeleteHeaderComments()      {m_head_commonts.clear();}

    // Key comment functions.
    // Key m_head_commonts are those m_head_commonts within a key.
    // Any m_head_commonts defined within value m_fields will be added
    // to this list. Therefore, these m_head_commonts will be moved to
    // the top of the key definition when the IniFile::WriteFile() is called.
    //
    // Add a key comment.
    bool     AddKeyComment(uint32_t key_id, const std::string &comment);
    bool     AddKeyComment(const std::string &key_name, const std::string &comment);

    // Delete a key comment.
    bool     DeleteKeyComment(uint32_t key_id, uint32_t comment_id);
    bool     DeleteKeyComment(const std::string &key_name, const uint32_t comment_id);

    // Delete all m_head_commonts for a key.
    bool     DeleteKeyComments(uint32_t key_id);
    bool     DeleteKeyComments(const std::string &key_name);

    // Return a key comment.
    std::string   KeyComment(uint32_t key_id, uint32_t comment_id) const;
    std::string   KeyComment(const std::string &key_name, uint32_t comment_id) const;

    // Number of key m_head_commonts.
    unsigned NumKeyComments(uint32_t key_id) const;
    unsigned NumKeyComments(const std::string &key_name) const;

private:
    std::string CheckCase(std::string s) const;

private:
    struct KeyValuePair {
        std::vector<std::string> names;
        std::vector<std::string> values;
        std::vector<std::string> comments;
    };

    bool                    m_ignore_case;
    std::string                  m_path;
    std::vector<KeyValuePair>    m_key_value_pair;
    std::vector<std::string>          m_fields;
    std::vector<std::string>          m_head_commonts;
};

#endif // COMMON_CONFFIG_INI_INI_FILE_H_ // NOLINT
