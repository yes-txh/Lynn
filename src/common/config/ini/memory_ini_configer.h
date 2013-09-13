/************************************************************
  Copyright (C), 1998-2006, Tencent Technology Commpany Limited

  文件名称: tse_mem_configer.h
  作者: swordshao
  日期: 2006.9.17
  版本: 1.0
  模块描述: 内存形式的配置信息读写模块
  组成类:
      MemoryIniConfiger
  修改历史:
      <author>    <time>   <version >   <desc>
      swordshao  2006.9.17    1.0        创建
*************************************************************/

//////////////////////////////////////////////////////////////////////////
// modified by ivanhuang at 20101115
// 1.修改代码风格、增加注释、修改部分代码
// 2.增加UnitTest
//////////////////////////////////////////////////////////////////////////

#ifndef COMMON_CONFFIG_MEMORY_INI_CONFIGER_H_ // NOLINT
#define COMMON_CONFFIG_MEMORY_INI_CONFIGER_H_

#include <string>
#include <map>
#include "common/config/ini/ini_configer.h"

/********************************************************************
 功能描述: 内存形式的配置信息读写类,是IniConfiger的内存形式的实现.
 主要对外接口:
     见IniConfiger
*********************************************************************/
class MemoryIniConfiger : public IniConfiger
{
private:
    virtual bool GetStringValue(const std::string& field, const std::string& key, std::string* value) const;
    virtual bool SetStringValue(const std::string& field, const std::string& key, const std::string& value);
private:
    /** 内存配置信息影射表, key为std::pair<Field, Key>, value为string */
    typedef std::map<std::pair<std::string, std::string>, std::string> FieldKeyValueMap;
    FieldKeyValueMap m_key_value_map;
};

#endif // COMMON_CONFFIG_MEMORY_INI_CONFIGER_H_ // NOLINT
