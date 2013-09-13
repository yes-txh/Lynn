// Copyright 2010, Tencent Inc.
// Author: Huican Zhu (huicanzhu@tencent.com)
//         Hangjun Ye (hansye@tencent.com)
//
// The C style wrapper API for Record I/O, mainly used for calling from Java via
// JNA.

#ifndef COMMON_FILE_RECORDIO_JNA_H_
#define COMMON_FILE_RECORDIO_JNA_H_

#include "common/base/stdint.h"
#include "common/system/dynamic_library.hpp"

#ifndef __cplusplus
struct RecordReader;
struct RecordWriter;
#else
class RecordReader;
class RecordWriter;
#endif

#ifdef  __cplusplus
extern "C" {
#endif

// Wrapper API for RecordReader.
DLL_EXPORT RecordReader* OpenRecordReader(const char* filename,
                                          uint32_t options);
DLL_EXPORT bool ReadRecord(RecordReader* record_reader,
                           const char** data,
                           int32_t* size);
DLL_EXPORT void CloseRecordReader(RecordReader* record_reader);

// Wrapper API for RecordWriter.
DLL_EXPORT RecordWriter* OpenRecordWriter(const char* filename,
                                          uint32_t options,
                                          uint32_t compression_codec);
DLL_EXPORT bool WriteRecord(RecordWriter* record_writer,
                            const char* data,
                            int32_t size);
DLL_EXPORT bool FlushRecordWriter(RecordWriter* record_writer);
DLL_EXPORT void CloseRecordWriter(RecordWriter* record_writer);

#ifdef  __cplusplus
}
#endif

#endif  // COMMON_FILE_RECORDIO_JNA_H_
