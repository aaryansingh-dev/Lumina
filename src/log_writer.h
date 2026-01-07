#ifndef LUMINA_LOG_WRITER_H
#define LUMINA_LOG_WRITER_H

#include <fstream>
#include <string>
#include "lumina/common.h"
#include "log_format.h"

namespace lumina {
namespace log {

/**
 * @brief Writer handles the sequential appending of records to a log file.
 * * It manages the physical 32KB block boundaries, ensuring that records
 * are fragmented correctly if they don't fit in the remaining space of a block.
 */
class Writer{

public:
    // Takes ownership of the file stream or manages a reference.
    // For simplicity in this stage, assuming an open std::ofstream.
    // Also, avoid implicit constructor conversions.
    explicit Writer(std::ofstream* dest);
    ~Writer();

    /**
     * @brief Add a record to the log.
     * This will calculate the checksum and handle block fragmentation.
     */
    Status AddRecord(const Slice& slice);


private:

    std::ofstream* dest_;     // destination to write data to
    int block_offset_;  // current offset in the 32KB file

    //internal helper to write each fragment to the file/disk
    Status EmitPhysicalRecord(RecordType type, const char* ptr, size_t length);

    // restrict copy functions for the writer objects
    Writer(const Writer&) = delete;
    Writer& operator=(const Writer&) = delete;

};

}
}

#endif