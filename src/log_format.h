#ifndef LUMINA_LOG_FORMAT_H
#define LUMINA_LOG_FORMAT_H

#include <cstdint>

namespace lumina{
namespace log{

    /**
     * @brief Record types for the Write-Ahead Log.
     * * Design Decision: Using fragments (First, Middle, Last) to allow
     * a single logical record to span across multiple 32KB physical blocks.
     */
    enum RecordType{
        kZeroType = 0, // for padding purposes. Reserved for preallocatef files
        kFullType = 1, // the full record is in the same block
        kFirstType = 2,
        kMiddleType = 3,
        kLastType = 4

        // when a record is too large to fit in the same block
        // we break it, and divide into first, middle, and last
    };

/**
 * @brief Layout Constants
 * Header: 
 * Checksum : uint32 (4 bytes)
 * Length   : uint16 (2 bytes)
 * Type     : uint8  (1 byte)
 * Total: 7 bytes
 */
static constexpr int kMaxRecordType = kLastType;
static constexpr int kBlockSize = 32768; // 32KB
static constexpr int kHeaderSize = 4 + 2 + 1;

}
}


#endif