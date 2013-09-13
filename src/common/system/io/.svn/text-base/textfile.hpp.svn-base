// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/06/11
// Description: test file utility

#ifndef COMMON_SYSTEM_IO_TEXTFILE_HPP
#define COMMON_SYSTEM_IO_TEXTFILE_HPP
#pragma once

#include <string>
#include <vector>
#include <deque>
#include <list>

namespace io { namespace textfile {

bool LoadToString(const std::string& filename, std::string* result);
bool ReadLines(const std::string& filename, std::vector<std::string>* result);
bool ReadLines(const std::string& filename, std::deque<std::string>* result);
bool ReadLines(const std::string& filename, std::list<std::string>* result);

} }

#endif // COMMON_SYSTEM_IO_TEXTFILE_HPP
