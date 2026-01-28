#include "coding.h"

namespace lumina {

 namespace {
    constexpr uint32_t kContinuationBit = 0x80;        // 1000 0000
    constexpr uint32_t kValueMask       = 0x7F;        // 0111 1111
    constexpr uint32_t kBitsPerByte     = 7;
    constexpr uint32_t kMaxVarintBytes  = 5;           // ceil(32 / 7)
    constexpr uint32_t kMaxShift        = 28;          // (kMaxVarintBytes - 1) * 7
    constexpr uint32_t B = 128; 
    }

/**
 * @brief Encodes a 32-bit unsigned integer into variable-length format.
 * Small values use fewer bytes (LEB128-style encoding).
 */
char* EncodeVarint32(char* dst, uint32_t v) {
    uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);

    while (v >= kContinuationBit) {
        *ptr++ = static_cast<uint8_t>((v & kValueMask) | kContinuationBit);
        v >>= kBitsPerByte;
    }
    *ptr++ = static_cast<uint8_t>(v);
    return reinterpret_cast<char*>(ptr);
}


/**
 * @brief Decodes a varint32 from [p, limit).
 * @return Pointer to the byte after the varint, or nullptr on failure.
 */
const char* GetVarint32Ptr(const char* p, const char* limit, uint32_t* v) {
    uint32_t result = 0;
    uint32_t shift  = 0;

    while (shift <= kMaxShift && p < limit) {
        uint32_t byte = static_cast<uint8_t>(*p++);
        result |= ((byte & kValueMask) << shift);

        if ((byte & kContinuationBit) == 0) {
            *v = result;
            return p;
        }
        shift += kBitsPerByte;
    }
    return nullptr; // malformed or truncated
}


/**
 * @brief Returns the number of bytes needed to encode a value as a varint.
 */
int VarintLength(uint64_t v) {
    int len = 1;
    while (v >= B) {
        v >>= kBitsPerByte;
        len++;
    }
    return len;
}

} // namespace lumina