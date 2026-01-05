#ifndef LUMINA_OPTIONS_H
#define LUMINA_OPTIONS_H

#include <cstddef>

namespace lumina {

/**
 * @brief Options to control the behavior of a database.
 * * Design Decision: We group these into a struct so that the API remains
 * stable even if we add new tuning parameters later.
 */
struct Options {
    // --- Performance Tuning ---

    /**
     * Amount of data to build up in memory (in bytes) before converting
     * to a sorted on-disk file. 
     * Default: 4MB. 
     * Larger values improve write performance but increase recovery time.
     */
    size_t write_buffer_size = 4 * 1024 * 1024;

    /**
     * Size of the blocks used for the WAL and SSTables.
     * Default: 32KB.
     * 32KB is a multiple of most hardware sector sizes (4KB/8KB), 
     * which helps prevent "Write Amplification" at the hardware level.
     */
    static constexpr size_t kBlockSize = 32768;

    // --- Durability & Safety ---

    /**
     * If true, the database will be created if it is missing.
     */
    bool create_if_missing = true;

    /**
     * If true, an error is raised if the database already exists.
     */
    bool error_if_exists = false;

    /**
     * If true, the write will be flushed from the operating system
     * buffer cache before the write is considered complete.
     * Setting this to true ensures 100% durability but significantly
     * slows down write throughput.
     */
    bool sync = false;
};

} // namespace lumina

#endif // LUMINA_OPTIONS_H