// Copyright 2010, Tencent Inc.
// Author: Huican Zhu (huicanzhu@tencent.com)
//         Hangjun Ye (hansye@tencent.com)
//         Huan Yu (huanyu@tencent.com)

#include <istream>
#include <ostream>
#include "common/base/compatible/io.h"
#include "common/base/scoped_ptr.h"
#include "common/base/stl_container_deleter.h"
#include "common/compress/block_compression_codec.h"
#include "common/crypto/hash/crc.hpp"
#include "common/encoding/variant_integer.hpp"
#include "common/file/file.h"
#include "common/file/recordio/recordio.h"
#include "common/file/recordio/recordio_extent_header.pb.h"
#include "common/system/memory/unaligned.hpp"
#include "thirdparty/glog/logging.h"
#include "thirdparty/google/protobuf/message.h"

namespace {
static const int32_t kBlockTypeMask = 0x0F;
enum BlockType {
    BLOCK_TYPE_FIXED = 0,
    BLOCK_TYPE_VARIABLE = 1,
    BLOCK_TYPE_SINGLE = 2,
    BLOCK_TYPE_EXTENT_PROTO_HEADER = 0x80,
};

// Only used for RecordWriter::m_record_size.
// zero or positive value when all records so far are in the same size;
// Defines several negative value for special meaning.
enum RecordSize{
    RECORD_SIZE_INIT = -1,
    RECORD_SIZE_DIFFERENT = -2,
};

static const int32_t kMaxVarintSizeOfRecordSize = 4;
// <4 bytes magic> + <1 byte type> + <1 byte size> + <2 bytes checksum>
static const int32_t kMinBlockHeaderSize = 8;
// <4 bytes magic> + <1 byte type> + <4 byte size> + <2 bytes checksum>
static const int32_t kMaxBlockHeaderSize = 11;
// kMinBlockHeaderSize + <1 byte body> + <4 bytes checksum>
static const int32_t kMinBlockSize = 13;
// <4 bytes checksum>
static const int32_t kBlockFooterSize = 4;

// Use a random int as CRC32 initial value, to reduce accidental conflict
// furthermore.
static const uint8_t kRecordIOCRC32InitValue[4] = { 0xe8, 0xba, 0x49, 0x5a };

// A random int. All bytes must be different for the convenience of processing
// (to guarantee valid magic would NOT overlap).
static const uint8_t kBlockHeaderMagic[4] = { 0x6d, 0x32, 0x47, 0xc9 };

// Block size for internal local buffers.
static const int32_t kLocalBufferBlockSize = 4 * 1024;                  // 4K

// Buffer size for record sizes and recors (see type 1, variable-size records).
static const int32_t kRecordSizeBufferSize = 8 * kLocalBufferBlockSize;  // 32K
static const int32_t kRecordBufferSize = 32 * kLocalBufferBlockSize;     // 128K

static const size_t kMaxVarintEncodedSize =
    VariantInteger::MaxEncodedSizeOf<uint32_t>::Value;

// Get 2 bytes checksum from 4 bytes CRC32 checksum.
static inline uint16_t CRC32Hash32ToHash16(uint32_t hash32) {
    return (static_cast<uint16_t>(hash32 >> 16) ^
            static_cast<uint16_t>(hash32));
}

// Compute a buffer size to hold at least bytes specified by user.
static int32_t ComputeLocalBufferSize(int32_t bytes_at_least) {
    // Up round to the block size.
    int32_t local_buffer_capacity =
        (bytes_at_least + (kLocalBufferBlockSize - 1)) /
        kLocalBufferBlockSize *
        kLocalBufferBlockSize;
    // Meet the min buffer size limit.
    if (local_buffer_capacity <
        (kRecordSizeBufferSize + kRecordBufferSize)) {
        local_buffer_capacity =
            kRecordSizeBufferSize + kRecordBufferSize;
    }
    return local_buffer_capacity;
}
}  // namespace

RecordReader::RecordReader(File* input_common_file,
                           const RecordReaderOptions& options_value) {
    Initialize();
    m_input_common_file = input_common_file;
    m_options = options_value;
    CHECK_NOTNULL(m_input_common_file);
    CHECK(!CurrentBlockAvailable())
        << "Must initialize as the current block is not available.";
}

RecordReader::RecordReader(std::istream* input_stream,
                           const RecordReaderOptions& options_value) {
    Initialize();
    m_input_stream = input_stream;
    m_options = options_value;
    CHECK_NOTNULL(m_input_stream);
    CHECK(!CurrentBlockAvailable())
        << "Must initialize as the current block is not available.";
}

RecordReader::RecordReader(FILE* input_file,
                           const RecordReaderOptions& options_value) {
    Initialize();
    m_input_file = input_file;
    m_options = options_value;
    CHECK_NOTNULL(m_input_file);
    CHECK(!CurrentBlockAvailable())
        << "Must initialize as the current block is not available.";
}

RecordReader::RecordReader(int input_fd,
                           const RecordReaderOptions& options_value) {
    Initialize();
    m_input_fd = input_fd;
    m_options = options_value;
    CHECK_NE(-1, m_input_fd);
    CHECK(!CurrentBlockAvailable())
        << "Must initialize as the current block is not available.";
}

RecordReader::~RecordReader() {
    if (m_options.m_options & RecordReaderOptions::OWN_STREAM) {
        if (m_input_common_file != NULL) {
            m_input_common_file->Close();
            delete m_input_common_file;
            // Don't cleanup the file implementation, which is process level
        } else if (m_input_stream != NULL) {
            delete m_input_stream;
        } else if (m_input_file != NULL) {
            fclose(m_input_file);
        } else if (m_input_fd != -1) {
            close(m_input_fd);
        }
    }
}

// TODO(hansye): Now we throw an exception when a block has a valid checksum but
// actually it's corrupt. Decide if we just need to skip it instead.
bool RecordReader::ReadRecord(const char** data, int32_t* size) {
    m_skipped_bytes = 0;
    uint64_t saved_accumulated_skipped_bytes = m_accumulated_skipped_bytes;

    // Check if the current block is used out.
    if (!CurrentBlockAvailable()) {
        bool available = TryReadNextBlock();
        m_skipped_bytes =
            m_accumulated_skipped_bytes - saved_accumulated_skipped_bytes;
        if (!available) {
            return false;
        }
    }

    switch (m_block_type) {
        case BLOCK_TYPE_FIXED:
            *data = m_uncompressed_body_ptr;
            *size = m_record_size;
            if (m_record_size == 0) {
                --m_records;
            }
            break;
        case BLOCK_TYPE_VARIABLE: {
            *data = m_uncompressed_body_ptr;
            int decode_length =
                DecodeLength(m_record_size_ptr,
                             m_record_size_end - m_record_size_ptr,
                             size);
            CHECK_GT(decode_length, 0) << "Not a valid record size.";
            m_record_size_ptr += decode_length;
            break;
        }
        case BLOCK_TYPE_SINGLE:
            *data = m_uncompressed_body_ptr;
            *size = m_uncompressed_body_size;
            break;
        default:
            LOG(FATAL) << "Unknown block type: " << m_block_type;
    }

    m_uncompressed_body_ptr += *size;
    return true;
}

bool RecordReader::ReadMessage(google::protobuf::Message* message) {
    m_skipped_records = 0;
    const char* data;
    int32_t size;
    while (ReadRecord(&data, &size)) {
        if (message->ParsePartialFromArray(data, size)) {
            if (message->IsInitialized()) {
                return true;
            } else {
                // Missing required fields.
                LOG(ERROR)
                    << "Missing required fields: "
                    << message->InitializationErrorString();
            }
        }
        ++m_skipped_records;
        ++m_accumulated_skipped_records;
    }
    // End of stream.
    return false;
}

void RecordReader::Initialize() {
    m_input_common_file = NULL;
    m_input_stream = NULL;
    m_input_file = NULL;
    m_input_fd = -1;
    m_state = STATE_START;
    m_block_type = BLOCK_TYPE_SINGLE;
    m_with_extent_proto_header = false;
    m_block_body_size = 0;
    m_block_body_end = NULL;
    m_record_size = 0;
    m_records = 0;
    m_record_size_end = NULL;
    m_record_size_ptr = NULL;
    m_local_buffer_capacity = 0;
    m_local_buffer_end = NULL;
    m_local_buffer_ptr = NULL;
    m_uncompressed_body_ptr = NULL;
    m_uncompressed_body_end = NULL;
    m_uncompressed_body_size = 0;
    m_checksum_state = GetUnaligned<uint32_t>(&kRecordIOCRC32InitValue);
    m_buffer_end_position = 0;
    m_current_block_position = 0;
    m_skipped_bytes = 0;
    m_accumulated_skipped_bytes = 0;
    m_skipped_records = 0;
    m_accumulated_skipped_records = 0;
    m_uncompressed_buffer_capacity = 0;
}

inline int32_t RecordReader::ReadFromStream(void* data, int32_t size) {
    if (size <= 0) {
        return 0;
    }

    if (m_input_common_file != NULL) {
        int64_t read_size = m_input_common_file->Read(
            data, static_cast<int64_t>(size));
        if (read_size <= 0) return 0;
        return static_cast<int32_t>(read_size);
    } else if (m_input_stream != NULL) {
        m_input_stream->clear();
        m_input_stream->read(reinterpret_cast<char*>(data), size);
        return m_input_stream->gcount();
    } else if (m_input_file != NULL) {
        clearerr(m_input_file);
        return fread(data, 1, size, m_input_file);
    } else if (m_input_fd != -1) {
        int read_size = read(m_input_fd, data, size);
        // read returns -1 on error, convert it to 0 here.
        if (read_size <= 0) return 0;
        return read_size;
    } else {
        LOG(FATAL) << "Missing underlying stream.";
        return 0;
    }
}

int RecordReader::DecodeLength(const void* buffer, size_t size, int32_t* result) {
    // initialize implementation of record using  unsigned integer to encode length
    return VariantInteger::Decode<uint32_t>(
        buffer, size, reinterpret_cast<uint32_t*>(result));
}

bool RecordReader::CurrentBlockAvailable() {
    if (m_state != STATE_VALID_BLOCK) {
        return false;
    }

    CHECK_LE(m_uncompressed_body_ptr, m_uncompressed_body_end);
    bool available;
    switch (m_block_type) {
        case BLOCK_TYPE_FIXED:
            if (m_record_size == 0) {
                // Special case for zero length records, check the remaining
                // number of records then.
                CHECK_GE(m_records, 0);
                available = m_records > 0;
            } else {
                available = m_uncompressed_body_ptr < m_uncompressed_body_end;
            }
            break;
        case BLOCK_TYPE_VARIABLE:
            CHECK_LE(m_record_size_ptr, m_record_size_end);
            available = m_record_size_ptr < m_record_size_end;
            break;
        case BLOCK_TYPE_SINGLE:
            available = m_uncompressed_body_ptr < m_uncompressed_body_end;
            break;
        default:
            LOG(FATAL) << "Unknown block type: " << m_block_type;
            return false;
    }

    if (!available) {
        m_state = STATE_START;
        // Skip the footer.
        m_local_buffer_ptr += kBlockFooterSize;
        // To make next call to CurrentBlockAvailable doesn't throw an
        // exception.
        m_uncompressed_body_end = m_uncompressed_body_ptr;
    }
    return available;
}

bool RecordReader::PreloadBufferAtLeast(int32_t bytes_at_least) {
    int local_buffer_available = m_local_buffer_end - m_local_buffer_ptr;
    if (local_buffer_available < bytes_at_least) {
        // Avaiable bytes are not enough, refill the buffer.
        if (m_local_buffer_capacity < bytes_at_least) {
            // Capacity is not enough, re-allocate the local buffer.
            // TODO(hansye): Currently the size of local buffer would
            // monotonously increase, investigate if we need to shrink it
            // sometimes.
            m_local_buffer_capacity = ComputeLocalBufferSize(bytes_at_least);
            char* new_buffer = new char[m_local_buffer_capacity];
            if (local_buffer_available > 0) {
                memcpy(new_buffer,
                       m_local_buffer_ptr,
                       local_buffer_available);
            }

            m_local_buffer.reset(new_buffer);
            m_local_buffer_ptr = m_local_buffer.get();
            m_local_buffer_end = m_local_buffer_ptr + local_buffer_available;
        } else {
            // Capacity is enough, re-use the current local buffer.
            // Caculate the remaining capacity from current buffer pointer.
            int remaining_capacity =
                m_local_buffer.get() + m_local_buffer_capacity -
                m_local_buffer_ptr;
            if (local_buffer_available == 0 ||
                remaining_capacity < bytes_at_least) {
                // Rewind the buffer pointer to the beginning of buffer.
                if (local_buffer_available > 0) {
                    // Move the available bytes.
                    memmove(m_local_buffer.get(),
                            m_local_buffer_ptr,
                            local_buffer_available);
                }
                m_local_buffer_ptr = m_local_buffer.get();
                m_local_buffer_end =
                    m_local_buffer_ptr + local_buffer_available;
            }
        }

        // TODO(hansye): Currently we try to totally refill the buffer,
        // investigate if we just need to read bytes to fulfill bytes_at_least.
        int32_t read_size =
            ReadFromStream(m_local_buffer_end,
                           m_local_buffer.get() + m_local_buffer_capacity -
                           m_local_buffer_end);
        IncreaseBufferEnd(read_size);
        return (m_local_buffer_end - m_local_buffer_ptr) >= bytes_at_least;
    }
    return true;
}

bool RecordReader::TryReadNextBlockHeader() {
    static const uint32_t kBlockHeaderMagicAsInt =
        GetUnaligned<uint32_t>(&kBlockHeaderMagic);
    CHECK(m_state == STATE_START);
    while (true) {
        // Seek for the block header magic number.
        if (!PreloadBufferAtLeast(kMinBlockSize)) {
            return false;
        }
        bool found_header_magic = false;
        while (m_local_buffer_end >
               (m_local_buffer_ptr + sizeof(kBlockHeaderMagic))) {
            const uint32_t magic = GetUnaligned<uint32_t>(m_local_buffer_ptr);
            if (magic == kBlockHeaderMagicAsInt) {
                if (!PreloadBufferAtLeast(kMinBlockSize)) {
                    return false;
                }
                m_current_block_position = BufferPointerPosition();
                m_local_buffer_ptr += sizeof(kBlockHeaderMagic);
                found_header_magic = true;
                break;
            }
            ++m_local_buffer_ptr;
            ++m_accumulated_skipped_bytes;
        }
        if (!found_header_magic) {
            // Buffer used up, start over.
            continue;
        }
        m_checksum_state = GetUnaligned<uint32_t>(&kRecordIOCRC32InitValue);
        m_checksum_state = UpdateCRC32(&kBlockHeaderMagic,
                                       sizeof(kBlockHeaderMagic),
                                       m_checksum_state);

        // Note the kMaxBlockHeaderSize is less than kMinBlockSize, so the
        // buffer must have enough bytes right now, we don't have to check the
        // size and read from underlying stream.

        // Get block type.
        // We define a temporary buffer position here. We don't really consume
        // the bytes if any of following steps fails, as they are still possibly
        // bytes of the block header magic.
        char* temp_buffer_ptr = m_local_buffer_ptr;
        m_block_type = *(temp_buffer_ptr++);
        m_with_extent_proto_header =
            m_block_type & BLOCK_TYPE_EXTENT_PROTO_HEADER;
        m_block_type &= kBlockTypeMask;
        if (m_block_type != BLOCK_TYPE_FIXED &&
            m_block_type != BLOCK_TYPE_VARIABLE &&
            m_block_type != BLOCK_TYPE_SINGLE) {
            // Not a valid block type, start over.
            m_accumulated_skipped_bytes += sizeof(kBlockHeaderMagic);
            continue;
        }

        // Get block size.
        int decode_length = DecodeLength(
            temp_buffer_ptr,
            m_local_buffer_end - temp_buffer_ptr,
            &m_block_body_size);

        if ((decode_length <= 0) ||
            (m_block_body_size > kMaxRecordSize)) {
            // Not a valid block size, start over.
            m_accumulated_skipped_bytes += sizeof(kBlockHeaderMagic);
            continue;
        }
        temp_buffer_ptr += decode_length;

        // Get checksum.
        m_checksum_state = UpdateCRC32(m_local_buffer_ptr,
                                       temp_buffer_ptr - m_local_buffer_ptr,
                                       m_checksum_state);
        uint16_t header_checksum = GetUnaligned<uint16_t>(temp_buffer_ptr);
        temp_buffer_ptr += sizeof(header_checksum);
        if (header_checksum != CRC32Hash32ToHash16(m_checksum_state)) {
            // Not a valid checksum, start over.
            m_accumulated_skipped_bytes += sizeof(kBlockHeaderMagic);
            continue;
        }

        // Done.
        m_block_header_size = sizeof(kBlockHeaderMagic) +
            temp_buffer_ptr - m_local_buffer_ptr;
        m_local_buffer_ptr = temp_buffer_ptr;
        m_state = STATE_VALID_HEADER;
        return true;
    }
}

bool RecordReader::TryReadNextBlock() {
    if (m_options.m_options &
        RecordReaderOptions::RESUME_LAST_INCOMPLETE_BLOCK) {
        // m_state is possibly STATE_VALID_HEADER only when
        // RESUME_LAST_INCOMPLETE_BLOCK option is set.
        CHECK(m_state == STATE_START || m_state == STATE_VALID_HEADER);
    } else {
        CHECK(m_state == STATE_START);
    }

    while (true) {
        if (m_state != STATE_VALID_HEADER) {
            if (!TryReadNextBlockHeader()) {
                if (!(m_options.m_options &
                      RecordReaderOptions::RESUME_LAST_INCOMPLETE_BLOCK)) {
                    // The user doesn't want to resume from last "incomplete"
                    // block, all bytes until buffer end are consumed and
                    // skipped.
                    m_accumulated_skipped_bytes +=
                        BufferEndPosition() - BufferPointerPosition();
                    m_local_buffer_ptr = m_local_buffer_end;
                }
                return false;
            }
        }

        // Try to read the block body and footer (the checksum).
        if (!PreloadBufferAtLeast(m_block_body_size + kBlockFooterSize)) {
            // Fail to read the block body, possibly the header is corrupt, or
            // the block is incomplete.
            if (m_options.m_options &
                RecordReaderOptions::RESUME_LAST_INCOMPLETE_BLOCK) {
                // This is the last "incomplete" block and user wants to resume
                // from it. Don't try to seek for next valid block and just wait
                // for more data coming.
                return false;
            } else {
                // The user doesn't want to resume from last "incomplete" block,
                // so just start over to seek for next valid block.
                m_accumulated_skipped_bytes +=
                    BufferPointerPosition() - m_current_block_position;
                m_state = STATE_START;
                continue;
            }
        }

        m_checksum_state = UpdateCRC32(m_local_buffer_ptr,
                                       m_block_body_size,
                                       m_checksum_state);
        m_block_body_end = m_local_buffer_ptr + m_block_body_size;
        uint32_t checksum = GetUnaligned<uint32_t>(m_block_body_end);
        if (checksum != m_checksum_state) {
            // Not a valid checksum, start over.
            m_accumulated_skipped_bytes +=
                BufferPointerPosition() - m_current_block_position;
            m_state = STATE_START;
            continue;
        }

        ProcessExtentHeader();

        // Done.
        switch (m_block_type) {
            case BLOCK_TYPE_FIXED: {
                int decode_length =
                    DecodeLength(m_uncompressed_body_ptr,
                                 (m_uncompressed_body_end -
                                 m_uncompressed_body_ptr),
                                 &m_record_size);
                CHECK_GT(decode_length, 0) << "Not a valid record size.";
                m_uncompressed_body_ptr += decode_length;
                if (m_record_size == 0) {
                    // Special case for zero length records, we need to read the
                    // number of records then.
                    decode_length =
                        DecodeLength(m_uncompressed_body_ptr,
                                     (m_uncompressed_body_end -
                                     m_uncompressed_body_ptr),
                                     &m_records);
                    CHECK_GT(decode_length, 0)
                        << "Not a valid number of records.";
                    m_uncompressed_body_ptr += decode_length;
                }
                break;
            }
            case BLOCK_TYPE_VARIABLE: {
                int32_t record_offset = 0;
                int decode_length =
                    DecodeLength(m_uncompressed_body_ptr,
                                 (m_uncompressed_body_end -
                                 m_uncompressed_body_ptr),
                                 &record_offset);
                CHECK_GT(decode_length, 0) << "Not a valid record offset.";
                m_uncompressed_body_ptr += decode_length;
                m_record_size_ptr = m_uncompressed_body_ptr;
                m_uncompressed_body_ptr += record_offset;
                m_record_size_end = m_uncompressed_body_ptr;
                break;
            }
            case BLOCK_TYPE_SINGLE:
                break;
            default:
                LOG(FATAL) << "Unknown block type: " << m_block_type;
                break;
        }

        m_state = STATE_VALID_BLOCK;
        return true;
    }
}


void RecordReader::ProcessExtentHeader() {
    common::file::RecordIOExtentHeader extent_header_proto;
    int32_t extent_header_buffer_size = 0;
    if (m_with_extent_proto_header) {
        int32_t extent_header_size = 0;
        int decode_length =
            DecodeLength(m_local_buffer_ptr,
                         (m_local_buffer_end -
                         m_local_buffer_ptr),
                         &extent_header_size);
        CHECK_GT(decode_length, 0) << "Not a valid ext header size.";
        extent_header_buffer_size += decode_length;
        m_local_buffer_ptr += decode_length;
        CHECK(extent_header_proto.ParseFromArray(
                m_local_buffer_ptr, extent_header_size))
            << "Not a valid extent header";
        extent_header_buffer_size += extent_header_size;
        m_local_buffer_ptr += extent_header_size;
    }
    if (extent_header_proto.has_compression_header()) {
        size_t uncompressed_size =
            extent_header_proto.compression_header().uncompressed_size();
        if (m_uncompressed_buffer_capacity < uncompressed_size) {
            // Capacity is not enough, re-allocate the local buffer.
            // TODO(hansye): Currently the size of local buffer would
            // monotonously increase, investigate if we need to shrink it
            // sometimes.
            m_uncompressed_buffer_capacity = uncompressed_size;
            m_uncompressed_buffer.reset(new char[uncompressed_size]);
        }
        // Lazily initialize compress codec.
        int compression_codec =
            extent_header_proto.compression_header().compression_codec();
        if (m_compression_codec.get() == NULL ||
            m_compression_codec->GetType() != compression_codec) {
            m_compression_codec.reset(
                intern::BlockCompressionCodec::CreateCodec(compression_codec));
        }
        CHECK_EQ(intern::BlockCompressionCodec::COMPRESSION_E_OK,
                 m_compression_codec->Inflate(m_local_buffer_ptr,
                                              m_block_body_size -
                                              extent_header_buffer_size,
                                              m_uncompressed_buffer.get(),
                                              &uncompressed_size));
        m_uncompressed_body_ptr = m_uncompressed_buffer.get();
        m_uncompressed_body_end = m_uncompressed_body_ptr + uncompressed_size;
        m_uncompressed_body_size = uncompressed_size;
    } else {
        m_uncompressed_body_ptr = m_local_buffer_ptr;
        m_uncompressed_body_end = m_local_buffer_ptr + m_block_body_size;
        m_uncompressed_body_size = m_block_body_size;
    }
    m_local_buffer_ptr += m_block_body_size - extent_header_buffer_size;
}


RecordWriter::RecordWriter(File* output_common_file,
                           const RecordWriterOptions& options_value)
    : m_options(options_value) {
    Initialize();
    m_output_common_file = output_common_file;
    CHECK_NOTNULL(m_output_common_file);
}


RecordWriter::RecordWriter(std::ostream* output_stream,
                           const RecordWriterOptions& options_value)
    : m_options(options_value) {
    Initialize();
    m_output_stream = output_stream;
    CHECK_NOTNULL(m_output_stream);
}

RecordWriter::RecordWriter(FILE* output_file,
                           const RecordWriterOptions& options_value)
    : m_options(options_value) {
    Initialize();
    m_output_file = output_file;
    CHECK_NOTNULL(m_output_file);
}

RecordWriter::RecordWriter(int output_fd,
                           const RecordWriterOptions& options_value)
    : m_options(options_value) {
    Initialize();
    m_output_fd = output_fd;
    CHECK_NE(-1, m_output_fd);
}

RecordWriter::~RecordWriter() {
    Flush();
    if (m_options.m_options & RecordWriterOptions::OWN_STREAM) {
        if (m_output_common_file != NULL) {
            m_output_common_file->Close();
            delete m_output_common_file;
            // Don't Cleanup file implementation
        } else if (m_output_stream != NULL) {
            delete m_output_stream;
        } else if (m_output_file != NULL) {
            fclose(m_output_file);
        } else if (m_output_fd != -1) {
            close(m_output_fd);
        }
    }
}

bool RecordWriter::WriteRecord(const char* data, int32_t size) {
    CHECK_GE(size, 0);
    CHECK_LE(size, kMaxRecordSize);

    // Lazily allocate internal buffers.
    if (m_record_size_buffer.get() == NULL) {
        m_record_size_buffer.reset(new char[kRecordSizeBufferSize]);
        m_record_size_buffer_ptr = m_record_size_buffer.get();
    }
    if (m_record_buffer.get() == NULL) {
        m_record_buffer.reset(new char[kRecordBufferSize]);
        m_record_buffer_ptr = m_record_buffer.get();
    }

    // Decide whether to flush.
    if (m_records > 0) {
        // Buffer not empty.
        if (((m_record_buffer_ptr + size) >=
             (m_record_buffer.get() + kRecordBufferSize)) ||
            ((m_record_size_buffer_ptr + EncodedSizeOf(size)) >=
              (m_record_size_buffer.get() + kRecordSizeBufferSize))) {
            // Record buffer or record size buffer overflow, flush now.
            if (!FlushFromExternalBuffer(
                m_record_buffer.get(),
                m_record_buffer_ptr - m_record_buffer.get())) {
                return false;
            }
        } else {
            if ((m_record_size >= 0) &&
                (m_record_size != static_cast<int32_t>(size))) {
                // Record size doesn't match with previous one or ones, decide
                // whether to flush previous records using type 0.
                if ((m_record_size_buffer_ptr - m_record_size_buffer.get()) >
                    2 * (kMaxBlockHeaderSize + kBlockFooterSize)) {
                    // We have pretty a few records with the same size, flush
                    // using type 0.
                    if (!FlushFromExternalBuffer(
                        m_record_buffer.get(),
                        m_record_buffer_ptr - m_record_buffer.get())) {
                        return false;
                    }
                }
            }
        }
    }

    m_record_size_buffer_ptr += EncodeLength(size, m_record_size_buffer_ptr);
    ++m_records;

    if ((m_record_buffer_ptr + size) >=
        (m_record_buffer.get() + kRecordBufferSize)) {
        // Record buffer still overflows. It implies the record size is larger
        // than the capacity of internal record buffer, so we write a single
        // record block.
        CHECK_EQ(m_record_buffer_ptr, m_record_buffer.get())
            << "Don't correctly flush before writing large record.";
        CHECK_EQ(1, m_records);
        FlushFromExternalBuffer(data, size);
    } else {
        // Write to internal buffers.
        memcpy(m_record_buffer_ptr, data, size);
        m_record_buffer_ptr += size;

        if (m_record_size == RECORD_SIZE_INIT) {
            m_record_size = size;
        } else if (m_record_size != static_cast<int32_t>(size)) {
            m_record_size = RECORD_SIZE_DIFFERENT;
        }
    }
    return true;
}

bool RecordWriter::WriteMessage(const google::protobuf::Message& message) {
    std::string output;
    if (!message.IsInitialized()) {
        // Missing required fields.
        LOG(ERROR)
            << "Missing required fields: "
            << message.InitializationErrorString();
        return false;
    }
    if (!message.AppendToString(&output)) {
        // It shouldn't happen per current protobuf message implementation.
        return false;
    }
    if (output.size() > static_cast<uint32_t>(kMaxRecordSize)) {
        return false;
    }
    return WriteRecord(output.data(), output.size());
}

bool RecordWriter::Flush() {
    if (m_records > 0) {
        if (!FlushFromExternalBuffer(
            m_record_buffer.get(),
            m_record_buffer_ptr - m_record_buffer.get())) {
            return false;
        }
        return FlushStream();
    }
    return true;
}


void RecordWriter::Initialize() {
    m_output_common_file = NULL;
    m_output_stream = NULL;
    m_output_file = NULL;
    m_output_fd = -1;
    m_record_size = RECORD_SIZE_INIT;
    m_records = 0;
    m_record_size_buffer_ptr = NULL;
    m_record_buffer_ptr = NULL;
    m_checksum_state = GetUnaligned<uint32_t>(&kRecordIOCRC32InitValue);
    m_uncompressed_buffer_capacity = 0;
    m_compressed_buffer_capacity = 0;
    if (m_options.m_compression_codec > 0) {
        m_compression_codec.reset(intern::BlockCompressionCodec::CreateCodec(
                m_options.m_compression_codec));
    }
}

inline bool RecordWriter::WriteToStream(const void* data, int32_t size) {
    if (size <= 0) {
        return true;
    }

    if (m_output_common_file != NULL) {
        int64_t write_size = m_output_common_file->Write(
            data, static_cast<int64_t>(size));
        return write_size >= 0;
    } else if (m_output_stream != NULL) {
        m_output_stream->write(reinterpret_cast<const char*>(data), size);
        return m_output_stream->good();
    } else if (m_output_file != NULL) {
        uint32_t real_size = static_cast<uint32_t>(size);
        return fwrite(data, 1, real_size, m_output_file) == real_size;
    } else if (m_output_fd != -1) {
        uint32_t real_size = static_cast<uint32_t>(size);
        // write return -1 for error
        return write(m_output_fd, data, real_size) == size;
    } else {
        LOG(FATAL) << "Missing underlying stream.";
        return false;
    }
}

inline bool RecordWriter::FlushStream() {
    if (m_output_common_file != NULL) {
        int32_t ret_code = m_output_common_file->Flush();
        return ret_code == 0;
    } else if (m_output_stream != NULL) {
        m_output_stream->flush();
        return m_output_stream->good();
    } else if (m_output_file != NULL) {
        return fflush(m_output_file) == 0;
    } else if (m_output_fd != -1) {
        return fsync(m_output_fd) == 0;
    } else {
        LOG(FATAL) << "Missing underlying stream.";
        return false;
    }
}

int RecordWriter::EncodedSizeOf(int32_t length) {
    // initialize implementation of record using  unsigned integer to encode length
    return VariantInteger::EncodedSize<uint32_t>(length);
}

int RecordWriter::EncodeLength(int32_t length, void* buffer) {
    // initialize implementation of record using  unsigned integer to encode length
    return VariantInteger::UncheckedEncode<uint32_t>(length, buffer);
}


inline bool RecordWriter::WriteAndUpdateChecksum(const void* data,
                                                 int32_t size) {
    if (size == 0) {
        return true;
    }
    m_checksum_state = UpdateCRC32(data, size, m_checksum_state);
    return WriteToStream(data, size);
}

inline bool RecordWriter::WriteVarintAndUpdateChecksum(int32_t value) {
    char varint_buffer[kMaxVarintEncodedSize];
    int varint_size = EncodeLength(value, varint_buffer);
    CHECK_GT(varint_size, 0);
    return WriteAndUpdateChecksum(varint_buffer, varint_size);
}

inline bool RecordWriter::WriteToBuffer(const void* data, int32_t size) {
    if (size == 0) {
        return true;
    }
    memcpy(m_uncompressed_buffer_ptr, data, size);
    m_uncompressed_buffer_ptr += size;
    return true;
}

inline bool RecordWriter::WriteVarintToBuffer(int32_t value) {
    char varint_buffer[kMaxVarintEncodedSize];
    int varint_size = EncodeLength(value, varint_buffer);
    CHECK_GT(varint_size, 0);
    return WriteToBuffer(varint_buffer, varint_size);
}

bool RecordWriter::FlushFromExternalBufferWithCompression(const char* data,
                                                          int32_t size) {
    CHECK_GT(m_records, 0);
    int32_t recode_size_buffer_size =
        m_record_size_buffer_ptr - m_record_size_buffer.get();
    CHECK_GT(recode_size_buffer_size, 0);

    char block_type = -1;
    int32_t block_body_size;
    if (m_records == 1) {
        // Single record type.
        block_type = BLOCK_TYPE_SINGLE;
        block_body_size = size;
    } else if (m_record_size >= 0) {
        // Fixed-size record type.
        block_type = BLOCK_TYPE_FIXED;
        block_body_size = EncodedSizeOf(m_record_size) + size;
        if (m_record_size == 0) {
            // Special case for zero length records, we need to write number of
            // records then.
            block_body_size += EncodedSizeOf(m_records);
        }
    } else {
        // Variable-size record type.
        block_type = BLOCK_TYPE_VARIABLE;
        block_body_size = EncodedSizeOf(recode_size_buffer_size) +
            recode_size_buffer_size + size;
    }


    if (block_body_size > m_uncompressed_buffer_capacity) {
        m_uncompressed_buffer_capacity = block_body_size;
        m_uncompressed_buffer.reset(new char[block_body_size]);
    }
    m_uncompressed_buffer_ptr = m_uncompressed_buffer.get();

    switch (block_type) {
        case BLOCK_TYPE_FIXED:
            if (!WriteVarintToBuffer(m_record_size)) {
                return false;
            }
            if (m_record_size == 0) {
                // Special case for zero length records, we need to write number
                // of records then.
                if (!WriteVarintToBuffer(m_records)) {
                    return false;
                }
            }
            break;
        case BLOCK_TYPE_VARIABLE:
            if (!WriteVarintToBuffer(recode_size_buffer_size)) {
                return false;
            }
            if (!WriteToBuffer(m_record_size_buffer.get(),
                               recode_size_buffer_size)) {
                return false;
            }
            break;
        case BLOCK_TYPE_SINGLE:
            break;
        default:
            LOG(FATAL) << "Unknown block type: " << block_type;
            break;
    }

    WriteToBuffer(data, size);

    if (m_compressed_buffer.get() == NULL ||
        m_compressed_buffer_capacity < block_body_size) {
        m_compressed_buffer.reset(new char[block_body_size]);
    }
    common::file::RecordIOExtentHeader extent_header_proto;
    char* output_buffer_ptr;
    size_t output_buffer_size = block_body_size;
    // Sometime compression will fail because the content has been
    // compressed, so that compressed content size may be larger than the
    // input. In such case we will output uncompressed body directly. We
    // still need to write an empty RecordIOExtentHeader because the bit
    // in blocktype has been set and written to stream.
    if (intern::BlockCompressionCodec::COMPRESSION_E_OK ==
        m_compression_codec->Deflate(m_uncompressed_buffer.get(),
                                     block_body_size,
                                     m_compressed_buffer.get(),
                                     &output_buffer_size)) {
        common::file::CompressionHeader* compression_header =
            extent_header_proto.mutable_compression_header();
        compression_header->set_compression_codec(
            m_options.m_compression_codec);
        compression_header->set_uncompressed_size(block_body_size);
        output_buffer_ptr = m_compressed_buffer.get();
    } else {
        output_buffer_ptr = m_uncompressed_buffer.get();
        output_buffer_size = block_body_size;
    }

    std::string extent_header_string;
    extent_header_proto.SerializeToString(&extent_header_string);

    // Write block header.
    block_type |= BLOCK_TYPE_EXTENT_PROTO_HEADER;
    m_checksum_state = GetUnaligned<uint32_t>(&kRecordIOCRC32InitValue);
    if (!WriteAndUpdateChecksum(&kBlockHeaderMagic,
                                sizeof(kBlockHeaderMagic)) ||
        !WriteAndUpdateChecksum(&block_type, sizeof(block_type)) ||
        !WriteVarintAndUpdateChecksum(
            output_buffer_size + EncodedSizeOf(extent_header_string.size()) +
            extent_header_string.size())) {
        return false;
    }

    // We don't checksum the checksum itself.
    uint16_t header_checksum = CRC32Hash32ToHash16(m_checksum_state);
    if (!WriteToStream(&header_checksum, sizeof(header_checksum))) {
        return false;
    }

    if (!WriteVarintAndUpdateChecksum(extent_header_string.size()) ||
        !WriteAndUpdateChecksum(extent_header_string.data(),
                                extent_header_string.size()) ||
        !WriteAndUpdateChecksum(output_buffer_ptr, output_buffer_size)) {
        return false;
    }

    // Write block footer, the block checksum.
    if (!WriteToStream(&m_checksum_state, sizeof(m_checksum_state))) {
        return false;
    }

    // Reset states.
    m_record_size = RECORD_SIZE_INIT;
    m_records = 0;
    m_record_size_buffer_ptr = m_record_size_buffer.get();
    m_record_buffer_ptr = m_record_buffer.get();
    return true;
}

bool RecordWriter::FlushFromExternalBufferWithoutCompression(const char* data,
                                                             int32_t size) {
    CHECK_GT(m_records, 0);
    int32_t recode_size_buffer_size =
        m_record_size_buffer_ptr - m_record_size_buffer.get();
    CHECK_GT(recode_size_buffer_size, 0);

    char block_type = -1;
    int32_t block_body_size;
    if (m_records == 1) {
        // Single record type.
        block_type = BLOCK_TYPE_SINGLE;
        block_body_size = size;
    } else if (m_record_size >= 0) {
        // Fixed-size record type.
        block_type = BLOCK_TYPE_FIXED;
        block_body_size = EncodedSizeOf(m_record_size) + size;
        if (m_record_size == 0) {
            // Special case for zero length records, we need to write number of
            // records then.
            block_body_size += EncodedSizeOf(m_records);
        }
    } else {
        // Variable-size record type.
        block_type = BLOCK_TYPE_VARIABLE;
        block_body_size = EncodedSizeOf(recode_size_buffer_size) +
            recode_size_buffer_size + size;
    }

    // Write block header.
    m_checksum_state = GetUnaligned<uint32_t>(&kRecordIOCRC32InitValue);
    if (!WriteAndUpdateChecksum(&kBlockHeaderMagic,
                                sizeof(kBlockHeaderMagic)) ||
        !WriteAndUpdateChecksum(&block_type, sizeof(block_type)) ||
        !WriteVarintAndUpdateChecksum(block_body_size)) {
        return false;
    }
    // We don't checksum the checksum itself.
    uint16_t header_checksum = CRC32Hash32ToHash16(m_checksum_state);
    if (!WriteToStream(&header_checksum, sizeof(header_checksum))) {
        return false;
    }

    // Write block body.
    switch (block_type & kBlockTypeMask) {
        case BLOCK_TYPE_FIXED:
            if (!WriteVarintAndUpdateChecksum(m_record_size)) {
                return false;
            }
            if (m_record_size == 0) {
                // Special case for zero length records, we need to write number
                // of records then.
                if (!WriteVarintAndUpdateChecksum(m_records)) {
                    return false;
                }
            }
            break;
        case BLOCK_TYPE_VARIABLE:
            if (!WriteVarintAndUpdateChecksum(recode_size_buffer_size)) {
                return false;
            }
            if (!WriteAndUpdateChecksum(m_record_size_buffer.get(),
                                        recode_size_buffer_size)) {
                return false;
            }
            break;
        case BLOCK_TYPE_SINGLE:
            break;
        default:
            LOG(FATAL) << "Unknown block type: " << block_type;
            break;
    }

    if (!WriteAndUpdateChecksum(data, size)) {
        return false;
    }


    // Write block footer, the block checksum.
    if (!WriteToStream(&m_checksum_state, sizeof(m_checksum_state))) {
        return false;
    }

    // Reset states.
    m_record_size = RECORD_SIZE_INIT;
    m_records = 0;
    m_record_size_buffer_ptr = m_record_size_buffer.get();
    m_record_buffer_ptr = m_record_buffer.get();
    return true;
}

bool RecordWriter::FlushFromExternalBuffer(const char* data, int32_t size) {
    if (m_compression_codec.get() != NULL) {
        return FlushFromExternalBufferWithCompression(data, size);
    } else {
        return FlushFromExternalBufferWithoutCompression(data, size);
    }
}
