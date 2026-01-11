#ifndef LUMINA_CRC32_H
#define LUMINA_CRC32_H

#include <cstdint>
#include <cstddef>
#include "lumina/common.h"


namespace lumina{

namespace crc32{


/**
 * @brief Mask a CRC32 value before storing it in the WAL or SSTable.
 *
 * Masking adds a simple transformation to the CRC32 value so that
 * special values (like all zeros or the CRC of an empty string) 
 * are less likely to appear. This helps detect torn writes or
 * partial records.
 *
 * @param crc The original CRC32 value.
 * @return A masked CRC32 value suitable for storage.
 */
uint32_t Mask(uint32_t crc);

/**
 * @brief Reverse the effect of Mask().
 *
 * When reading a CRC32 from WAL or SSTable, we must unmask it
 * before comparing it to a newly computed CRC32 for validation.
 *
 * @param masked_crc The masked CRC32 value read from storage.
 * @return The original CRC32 value before masking.
 */
uint32_t Unmask(uint32_t masked_crc);

/**
 * @brief Compute the CRC32 of a block of data.
 *
 * CRC32 is a checksum algorithm that produces a 32-bit value
 * representing the contents of a byte array. It is used to
 * detect corruption in WAL records or SSTable blocks.
 *
 * @param data Pointer to the start of the data.
 * @param n Length of the data in bytes.
 * @return The CRC32 checksum of the provided data.
 *
 * @note This function does not maintain any state — it computes
 * the CRC over the given buffer only.
 */
uint32_t Value(const char* data, size_t n);


/**
 * @brief Extend an existing CRC32 with additional data.
 *
 * Often, we compute the CRC incrementally: for example, for a
 * WAL record, we first compute the CRC of the header, then
 * extend it with the record body. Extend allows you to continue
 * updating an existing CRC without starting from scratch.
 *
 * @param crc The current CRC32 value.
 * @param data Pointer to additional data to include.
 * @param n Length of the additional data in bytes.
 * @return Updated CRC32 value including the new data.
 */
uint32_t Extend(uint32_t crc, const char* data, size_t n);


}
}

#endif