// Copyright 2010, Tencent Inc.
// Author: Huican Zhu (huicanzhu@tencent.com)
//         Hangjun Ye (hansye@tencent.com)
//         Huan Yu (huanyu@tencent.com)
//
// Record I/O represents a stream of record logically. The record could be an
// arbitrary binary string and record I/O doesn't asuume its type. It's the
// user's responsibility to encode to or decode from the appropriate type.
//
// The motivation of record I/O is to resume the record stream even some bytes
// are corrupt. So it provides two important features: corrupt record detection
// and automatically seeking to next valid record. It doesn't "recover" the
// corrupt records as it doesn't have redundant copy.
//
// To achieve this, record I/O physically stores records in blocks as following:
// <block header>
//   <4 bytes, magic number>
//   <1 byte, block type>
//   <varint, block body size, excluding the header/footer, maximum 32M>
//   <2 bytes, checksum of header>
// <block body, the payload>
// <4 bytes, checksum of the whole block, including the header>
//
// The format of block body depends on the block type, currently record I/O
// defines following block types:
// type 0: fixed-size records.
// The block payload format is:
// <varint, record size>
// <record 1>
// ...
// <record N>
//
// or
// <varint, 0, special case for zero length record>
// <varint, number of record>
//
// type 1: variable-size records
// The block payload format is:
// <varint, size of bytes, which stores following record sizes>
// <varint, record size 1>
// ...
// <varint, record size N>
// <record 1>
// ...
// <record N>
//
// type 2: a single record
// The block payload format is:
// <record>
//
// To store more meta information in the header, we introduce a backward
// compatible optional field of extent_header in block body. If extent
// header bit in block type has been set, we extent block body format as
// following:
// <varint, extent_header size>
// <protobuf of extent_header>
// <real block payload as described above>
//
// Currently we use extent header to support block compression. See
// recordio_extent_header.proto for details.
//
// Rationales:
// As checksum has overhead, so we don't want to store a checksum for each
// reocrd if it's small. We use block as the reliable container for records
// and the basic unit for checksumming.
//
// Besides block body, we also checksum the header. The reason is we don't want
// to try to read and checksum the whole possible block when seeking next valid
// block, as we possibly get a wrong hint about the block size and introduce
// unnecessary i/o. For example, bytes in header are corrupt, or block body is
// incomplete. Having a check for the header, we just need to read and checksum
// the possible block header.
//
//
// Caveats:
// Record I/O doesn't support "nesting", i.e., if a user puts content of
// record I/O file as a record into another record I/O file, it would prevent
// the correct error handling of the second record I/O file, as record I/O might
// be fooled when seeking next valid block.
//
// A remedy is that user encodes the context of first record I/O file firstly.
// But it's not transparent then and user has to record some data for decoding
// the content.
//
// We might define a new block type for nesting record I/O, it could help user
// record data necessary for decoding, but it's not transparent to user either.
// Maybe compressed block type could help this too.

#ifndef COMMON_FILE_RECORDIO_H_
#define COMMON_FILE_RECORDIO_H_

#include <stdio.h>
#include <iosfwd>
#include "common/base/scoped_ptr.h"
#include "common/base/stdint.h"
#include "common/base/string/string_piece.hpp"
#include "common/base/platform_features.hpp"
#include "common/compress/block_compression_codec.h"

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google

namespace intern {
class BlockCompressionCodec;
}

class File; // A common file interface which supports local file and xfs file

struct RecordReaderOptions {
    enum {
        // If this option is set, record reader would take the ownership of
        // input stream and delete it in destructor. It's for the convenience of
        // maintaining the stream, and guaranteeing the sequence of destruction.
        OWN_STREAM = 0x0001,

        // If we want to read from a record i/o file when a writer is writing
        // simultaneously, we might read an incomplete block at the end of
        // stream. If this option is set, we still return false for ReadXXX
        // function when getting an incomplete block at the end of stream, but
        // would resume from it when user calls ReadXXX function again.
        //
        // NOTE: please use caution when setting this option, it introduces
        // ambiguity as we can NOT distinguish a "corrupt" block from an
        // "incomplete" block at the end of stream, so only set it when
        // necessary, i.e. a writer is writing data simultaneously.
        RESUME_LAST_INCOMPLETE_BLOCK = 0x0002,

        DEFAULT_OPTIONS = 0,
    };

    explicit RecordReaderOptions(uint32_t options = DEFAULT_OPTIONS)
        : m_options(options) {
    }

    uint32_t m_options;
};

struct RecordWriterOptions {
    enum {
        // If this option is set, record writer would take the ownership of
        // output stream and delete it in destructor. It's for the convenience
        // of maintaining the stream, and guaranteeing the sequence of
        // destruction.
        OWN_STREAM = 0x0001,

        DEFAULT_OPTIONS = 0,
    };

    explicit RecordWriterOptions(
        uint32_t options = DEFAULT_OPTIONS,
        uint32_t compression_codec = intern::BlockCompressionCodec::NONE)
        : m_options(options),
          m_compression_codec(compression_codec) {
    }

    uint32_t m_options;

    // See compression_type in compress/block_compression_codec.h
    uint32_t m_compression_codec;
};

class RecordReader {
public:
    static const int32_t kMaxRecordSize = 32 * 1024 * 1024 - 1;  // 32M

    // We provide several constructor to fit different file interfaces.

    explicit RecordReader(
        File* input_common_file,
        const RecordReaderOptions& options_value = RecordReaderOptions());
    explicit RecordReader(
        std::istream* input_stream,
        const RecordReaderOptions& options_value = RecordReaderOptions());
    explicit RecordReader(
        FILE* input_file,
        const RecordReaderOptions& options_value = RecordReaderOptions());
    explicit RecordReader(
        int input_fd,
        const RecordReaderOptions& options_value = RecordReaderOptions());
    ~RecordReader();

    const RecordReaderOptions& options() const { return m_options; }
    void set_options(const RecordReaderOptions& options_value) {
        m_options = options_value;
    }

    // Read next record from record I/O stream. Return true if a record is read
    // successfully, false if no more valid record is found until end of stream.
    //
    // The parameter data would be pointed to an internal buffer which contains
    // the read record. The buffer possible becomes invalid after next call of
    // ReadRecord, so it's user's responsibility to keep a copy if necessary.
    bool ReadRecord(const char** data, int32_t* size);

    // Compatible with old interface
    bool ReadRecord(const char** data, uint32_t* size) {
        int32_t ssize;
        bool result = ReadRecord(data, &ssize);
        *size = ssize;
        return result;
    }

    // A convenient wrapper of ReadRecord that uses StringPiece to store buffer.
    bool ReadRecord(StringPiece* data) {
        const char* buffer;
        int32_t size;
        if (!ReadRecord(&buffer, &size)) return false;
        data->set(buffer, size);
        return true;
    }

    // It assumes the read record is a protocol buffer message and try to parse
    // it. This interface is just for convenience. Return false if no more valid
    // protocol buffer message is found until end of stream.
    // A message missing required fields IS considered as INVALID.
    //
    // Note: GetSkippedBytes() might return an inaccurate value as internally
    // we need to try to call ReadRecord until finding a valid protocol buffer
    // message. And protocol buffer doesn't validate the input very strictly,
    // so it's application's responsibilty to ensure that.
    bool ReadMessage(google::protobuf::Message* message);

    // Call this function after a call of ReadXXX function, it returns skipped
    // bytes due to corrupt record detection. A value larger than 0 means some
    // corrupt records are detected. Some applications might want to stop
    // processing when an error is detected.
    int64_t GetSkippedBytes() const { return m_skipped_bytes; }

    // The accumualted one.
    int64_t GetAccumulatedSkippedBytes() const {
        return m_accumulated_skipped_bytes;
    }

    // Call this function after a call of ReadMessage function, it returns
    // skipped records because protocol buffer parsing fails.
    int64_t GetSkippedRecords() const { return m_skipped_records; }

    // The accumualted one.
    int64_t GetAccumulatedSkippedRecords() const {
        return m_accumulated_skipped_records;
    }

    // Return available bytes in local buffer. Call this function after a
    // ReadXXX function returns false (reach the end of stream) to show how many
    // bytes are still unconsumed. It's used when user wants to resume from last
    // incomplete block.
    int64_t GetUnconsumedBytes() const {
        return m_local_buffer_end - m_local_buffer_ptr;
    }

    int64_t GetCurrentBlockPosition() const {
        return BlockEndPosition() -
                (m_block_header_size + m_block_body_size);
    }

private:
    enum State {
        STATE_START = 0x0000,
        STATE_VALID_HEADER = 0x0001,
        STATE_VALID_BLOCK = 0x0002,
    };

    void Initialize();

    // Wrapper functions for underlying input stream, as we support stl stream,
    // FILE*, and fd currently.
    // It returns number of bytes read, returns 0 for end of file or error.
    // TODO(aaronzou): return 0 for end of file and return -1 for error.
    int32_t ReadFromStream(void* data, int32_t size);

    // Decord a variant integer encoded length field from buffer
    static int DecodeLength(const void* buffer, size_t size, int32_t* result);

    // Return true if current block has some avaiable records.
    // Note it would change the internal state to STATE_START when we just
    // consume all bytes of current block.
    bool CurrentBlockAvailable();

    // Preload from underlying stream to the local buffer until it has at least
    // bytes specified by the parameter bytes_at_least.
    // It might read more bytes than bytes_at_least for optimization.
    // Return false if underlying stream doesn't have that many bytes.
    bool PreloadBufferAtLeast(int32_t bytes_at_least);

    // Try to read next valid block header. Return false if it doesn't find a
    // valid one until end of the stream.
    bool TryReadNextBlockHeader();

    // Try to read next valid block. Return false if it doesn't find a valid one
    // until end of the stream.
    bool TryReadNextBlock();

    // Parse extent header from block body, then do extra block body decoding if
    // necessary. For example, if there is a compression codec set in the extent
    // header, decompress block content by the codec.
    void ProcessExtentHeader();

    // Return the position of the "end" of current block (the m_block_body_end)
    // in the input stream.
    int64_t BlockEndPosition() const {
        return (BufferEndPosition() -
                (m_local_buffer_end - m_block_body_end));
    }

    // Return the position of the "end" of local buffer (the m_local_buffer_end)
    // in the input stream.
    int64_t BufferEndPosition() const {
        return m_buffer_end_position;
    }

    // Return the position of the current pointer of local buffer (the
    // m_local_buffer_ptr) in the input stream. It's inferred from the buffer
    // end position.
    int64_t BufferPointerPosition() const {
        return (BufferEndPosition() -
                (m_local_buffer_end - m_local_buffer_ptr));
    }

    void IncreaseBufferEnd(const int64_t count) {
        m_local_buffer_end += count;
        m_buffer_end_position += count;
    }

    // Only one of the following would be used.
    // TODO(hansye): switch to File interface once it's ready.
    File* m_input_common_file;
    std::istream* m_input_stream;
    FILE* m_input_file;
    int m_input_fd;

    RecordReaderOptions m_options;
    State m_state;

    // For all block types.
    int32_t m_block_type;
    int32_t m_block_header_size;
    bool m_with_extent_proto_header;
    int32_t m_block_body_size;
    const char* m_block_body_end;

    // For fixed-size record type.
    int32_t m_record_size;
    int32_t m_records;  // Only for zero length records.

    // For variable-size record type.
    const char* m_record_size_end;
    const char* m_record_size_ptr;

    // For single record type.

    // Local buffer for the input stream. It's to optimize when reading and
    // seeking the valid block header, by reducing calls to underlying stream.
    // TODO(hansye): It introduces one more memory copy, considering using a
    // method similar to ZeroCopyInputStream when we switch input stream to File
    // in future.
    scoped_array<char> m_local_buffer;
    int32_t m_local_buffer_capacity;
    char* m_local_buffer_end;
    char* m_local_buffer_ptr;

    scoped_array<char> m_uncompressed_buffer;
    size_t m_uncompressed_buffer_capacity;
    char* m_uncompressed_body_ptr;
    char* m_uncompressed_body_end;
    int32_t m_uncompressed_body_size;

    // The checksum state.
    uint32_t m_checksum_state;

    // The position of the "end" of local buffer (the m_local_buffer_end) in
    // the input stream. We use "end" instead of "beginning" as we don't want to
    // update this field frequently.
    int64_t m_buffer_end_position;
    // The position of current block in the input stream.
    int64_t m_current_block_position;

    // Skipped bytes due to corrupt record detection. It indicates the skipped
    // bytes from last attempt to seek a valid block.
    int64_t m_skipped_bytes;
    // The accumulated one.
    int64_t m_accumulated_skipped_bytes;

    // Skipped records because protocol buffer parsing fails. It indicates the
    // skipped records from last call to ReadMessage.
    int64_t m_skipped_records;
    // The accumulated one.
    int64_t m_accumulated_skipped_records;

    // Lazily allocated compression codec. It may be different among blocks
    // because we may append more records into existing recordio file with
    // different codec.
    scoped_ptr<intern::BlockCompressionCodec> m_compression_codec;
};

class RecordWriter {
public:
    static const int32_t kMaxRecordSize = RecordReader::kMaxRecordSize;  // 32M

    // We provide several constructor to fit different file interfaces.

    explicit RecordWriter(
        File* output_common_file,
        const RecordWriterOptions& options_value = RecordWriterOptions());
    explicit RecordWriter(
        std::ostream* output_stream,
        const RecordWriterOptions& options_value = RecordWriterOptions());
    explicit RecordWriter(
        FILE* output_file,
        const RecordWriterOptions& options_value = RecordWriterOptions());
    explicit RecordWriter(
        int output_fd,
        const RecordWriterOptions& options_value = RecordWriterOptions());
    ~RecordWriter();

    const RecordWriterOptions& options() const { return m_options; }
    void set_options(const RecordWriterOptions& options_value) {
        m_options = options_value;
    }

    // Write a record to record I/O stream. The size could be zero.
    // Return false if failed, which only happens due to underlying I/O failure.
    bool WriteRecord(const char* data, int32_t size);

    // A convenient wrapper of WriteRecord that uses StringPiece to store data.
    bool WriteRecord(const StringPiece& data) {
        return WriteRecord(data.data(), data.size());
    }

    // This interface is just for convenience.
    // Return false if failed, which happens when protocol buffer message is
    // invalid or underlying I/O fails.
    // A message missing required fields IS considered as INVALID.
    bool WriteMessage(const google::protobuf::Message& message);

    // It's called internally when internal buffers overflow and we need to
    // write a block. But user could also call it to wrap up a block earlier.
    bool Flush();

private:
    void Initialize();

    // Wrapper functions for underlying input stream, as we support stl stream,
    // FILE*, and fd currently.
    // It returns true if all bytes are written successfully.
    bool WriteToStream(const void* data, int32_t size);
    bool FlushStream();

    // Calculate encoded bytes length of a length value
    static int EncodedSizeOf(int32_t length);

    // encode a length value into buffer
    static int EncodeLength(int32_t length, void* buffer);

    // Write to output stream and update the internal checksum state.
    bool WriteAndUpdateChecksum(const void* data, int32_t size);
    bool WriteVarintAndUpdateChecksum(int32_t value);

    // Write to output stream and update the internal checksum state if no
    // compression, or output to internal buffer.
    bool WriteToBuffer(const void* data, int32_t size);
    bool WriteVarintToBuffer(int32_t value);

    // Sometimes we need to flush an external buffer (when user writes a record
    // larger than internal buffer size, we write a single record block directly
    // from user's buffer).
    bool FlushFromExternalBuffer(const char* data, int32_t size);

    // Here is the major difference between following two functions:
    // It is straightforward to calculate header checksum for uncompressed body
    // because the body size in header is known beforehand. But for compressed
    // body, we must compress the body first to obtain the size of compressed
    // body, then set it in header and calculate the checksum.
    bool FlushFromExternalBufferWithoutCompression(const char* data,
                                                   int32_t size);
    bool FlushFromExternalBufferWithCompression(const char* data,
                                                int32_t size);

    // Only one of the following would be used.
    // TODO(hansye): switch to File interface once it's ready.
    File* m_output_common_file;
    std::ostream* m_output_stream;
    FILE* m_output_file;
    int m_output_fd;

    RecordWriterOptions m_options;

    // Keep the track of record size in current buffer.
    // Must be a signed integer, we use negative values for special meaning.
    int32_t m_record_size;
    // Number of records in current buffer.
    int32_t m_records;

    scoped_array<char> m_record_size_buffer;
    scoped_array<char> m_record_buffer;
    char* m_record_size_buffer_ptr;
    char* m_record_buffer_ptr;

    // The checksum state.
    uint32_t m_checksum_state;

    // If compression codec is set in option, initialize codec and use it
    // during the whole writing process.
    scoped_ptr<intern::BlockCompressionCodec> m_compression_codec;
    scoped_array<char> m_uncompressed_buffer;
    int32_t m_uncompressed_buffer_capacity;
    char* m_uncompressed_buffer_ptr;
    scoped_array<char> m_compressed_buffer;
    int32_t m_compressed_buffer_capacity;
};

#endif  // COMMON_FILE_RECORDIO_H_
