// Copyright 2010, Tencent Inc.
// Author: Huican Zhu (huicanzhu@tencent.com)
//         Hangjun Ye (hansye@tencent.com)

#include "common/file/recordio/recordio_jna.h"
#include <fstream>
#include <new>
#include "common/base/scoped_ptr.h"
#include "common/file/recordio/recordio.h"
// includes from thirdparty
#include "glog/logging.h"

RecordReader* OpenRecordReader(const char* filename, uint32_t options) {
    scoped_ptr<std::ifstream> input_stream(
        new (std::nothrow) std::ifstream(
            filename, std::ios::in | std::ios::binary));
    if ((input_stream.get() == NULL) || !input_stream->good()) {
        return NULL;
    }

    options |= RecordReaderOptions::OWN_STREAM;
    RecordReader* record_reader =
        new (std::nothrow) RecordReader(
            input_stream.release(), RecordReaderOptions(options));
    return record_reader;
}

bool ReadRecord(RecordReader* record_reader,
                const char** data,
                int32_t* size) {
    CHECK_NOTNULL(record_reader);
    return record_reader->ReadRecord(data, size);
}

void CloseRecordReader(RecordReader* record_reader) {
    CHECK_NOTNULL(record_reader);
    delete record_reader;
}

RecordWriter* OpenRecordWriter(const char* filename,
                               uint32_t options,
                               uint32_t compression_codec) {
    scoped_ptr<std::ofstream> output_stream(
        new (std::nothrow) std::ofstream(
            filename, std::ios::out | std::ios::binary | std::ios::app));
    if ((output_stream.get() == NULL) || !output_stream->good()) {
        return NULL;
    }

    options |= RecordWriterOptions::OWN_STREAM;
    RecordWriter* record_writer =
        new (std::nothrow) RecordWriter(
            output_stream.release(),
            RecordWriterOptions(options, compression_codec));
    return record_writer;
}

bool WriteRecord(RecordWriter* record_writer, const char* data, int32_t size) {
    CHECK_NOTNULL(record_writer);
    return record_writer->WriteRecord(data, size);
}

bool FlushRecordWriter(RecordWriter* record_writer) {
    CHECK_NOTNULL(record_writer);
    return record_writer->Flush();
}

void CloseRecordWriter(RecordWriter* record_writer) {
    CHECK_NOTNULL(record_writer);
    delete record_writer;
}
