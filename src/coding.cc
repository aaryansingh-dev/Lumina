#include "coding.h"

namespace lumina {

namespace {
    constexpr uint32_t kContinuationBit = 0x80;        // 1000 0000
    constexpr uint32_t kValueMask       = 0x7F;        // 0111 1111
    constexpr uint32_t kBitsPerByte     = 7;
    constexpr uint32_t kMaxVarintBytes  = 5;           // ceil(32 / 7)
    constexpr uint32_t kMaxShift        = (kMaxVarintBytes - 1) * kBitsPerByte; 
}

char* EncodeVarint32(char* dst, uint32_t v) {
    uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);

    while (v >= kContinuationBit) {
        *ptr++ = static_cast<uint8_t>((v & kValueMask) | kContinuationBit);
        v >>= kBitsPerByte;
    }
    *ptr++ = static_cast<uint8_t>(v);
    return reinterpret_cast<char*>(ptr);
}

const char* GetVarint32Ptr(const char* ptr, const char* limit, uint32_t* v) {
    uint32_t result = 0;
    uint32_t shift  = 0;

    while (shift <= kMaxShift && ptr < limit) {
        uint32_t byte = static_cast<uint8_t>(*ptr++);
        result |= ((byte & kValueMask) << shift);

        if ((byte & kContinuationBit) == 0) {
            *v = result;
            return ptr;
        }
        shift += kBitsPerByte;
    }
    return nullptr;
}

int VarintLength(uint64_t v) {
    int len = 1;
    while (v >= kContinuationBit) {
        v >>= kBitsPerByte;
        len++;
    }
    return len;
}

} // namespace lumina