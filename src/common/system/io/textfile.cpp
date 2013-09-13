// Copyright (c) 2011, Tencent Inc.
// All rights reserved.
//
// Author: CHEN Feng <phongchen@tencent.com>
// Created: 05/06/11
// Description: text file utility

#include "common/system/io/textfile.hpp"
#include <string.h>
#include <errno.h>
#include <fstream>
#include "common/base/stdint.h"
#include "common/base/unique_ptr.hpp"
#include "common/base/stdext/string.hpp"
#include "common/system/io/file.hpp"

namespace io { namespace textfile {

char* RemoveLineEnding(char* line)
{
    size_t length = strlen(line);
    while(length > 0 && (line[length-1]=='\r' || line[length-1] == '\n'))
        line[--length]= '\0';
    return line;
}

void RemoveLineEnding(std::string* line)
{
    while (!line->empty())
    {
        char last = (*line)[line->length() - 1];
        if (last == '\r' || last == '\n')
            line->resize(line->length() - 1);
        else
            break;
    }
}

bool LoadToString(const std::string& filename, std::string* result)
{
    int64_t size = io::file::GetSize(filename);
    if (size < 0)
        return false;

    if (static_cast<uint64_t>(size) > SIZE_MAX) // file is larger than memory
    {
        errno = EFBIG;
        return false;
    }

    unique_ptr<FILE> fp(fopen(filename.c_str(), "rb"));
    if (!fp)
        return false;

    try
    {
        result->resize(size);
    }
    catch (std::bad_alloc& e)
    {
        return false;
    }

    char* buffer = string_as_array(result);

    const int64_t kMaxReadSize = 2 * 1024 * 1024 * 1024LL;
    int read_size;
    while ((read_size = fread(buffer, 1, std::min(size, kMaxReadSize), fp.get())) > 0)
    {
        buffer += read_size;
        size -= read_size;
    }

    if (size != 0)
    {
        result->clear();
        return false;
    }

    return true;
}

namespace {
template <typename Container>
bool ReadLinesT(const std::string& filename, Container* result)
{
    int64_t size = io::file::GetSize(filename);
    if (size < 0)
        return false;

    if (static_cast<uint64_t>(size) > SIZE_MAX) // file is larger than memory
    {
        errno = EFBIG;
        return false;
    }

    std::ifstream fs;
    fs.open(filename.c_str(), std::ios_base::in);

    if (fs.fail())
        return false;

    result->clear();

    std::string line;
    while (std::getline(fs, line))
    {
        RemoveLineEnding(&line);

        // swap with empty object to reduce copying
        result->push_back(std::string());
        std::swap(result->back(), line);
    }

    return true;
}
} // anonymous namespace

bool ReadLines(const std::string& filename, std::vector<std::string>* result)
{
    return ReadLinesT(filename, result);
}

bool ReadLines(const std::string& filename, std::deque<std::string>* result)
{
    return ReadLinesT(filename, result);
}

bool ReadLines(const std::string& filename, std::list<std::string>* result)
{
    return ReadLinesT(filename, result);
}

} } // end namespace io.textfile

