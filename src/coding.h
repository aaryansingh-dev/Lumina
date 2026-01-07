#ifndef LUMINA_CODING_H
#define LUMINA_CODING_H

#include <cstdint>
#include <cstring>

namespace lumina{

    inline void EncodeFixed32(char* dst, uint32_t value){
        uint8_t* const buffer = reinterpret_cast<uint8_t*>(dst);    // forcing the compiler to read memory as bytes and not chars for proper operations in 0-255.
        buffer[0] = static_cast<uint8_t>(value);    // store the lowest 8 bits(1 byte).
        buffer[1] = static_cast<uint8_t>(value>>8);
        buffer[2] = static_cast<uint8_t>(value>>16);
        buffer[3] = static_cast<uint8_t>(value>>24);
    }


    inline uint32_t DecodeFixed32(const char* ptr){
        const uint8_t* const buffer = reinterpret_cast<const uint8_t*>(ptr);
        return static_cast<uint32_t>(buffer[0]) | 
        (static_cast<uint32_t>(buffer[1])<<8) |
        (static_cast<uint32_t>(buffer[2])<<16) | 
        (static_cast<uint32_t>(buffer[3])<<24);

    }

    inline void EncodeFixed16(char* dst, uint32_t value){
        uint8_t* const buffer = reinterpret_cast<uint8_t*>(dst);
        buffer[0] = static_cast<uint8_t>(value);    // store the lowest 8 bits(1 byte).
        buffer[1] = static_cast<uint8_t>(value>>8);
    }

    inline uint16_t DecodeFixed16(const char* ptr){
        const uint8_t* const buffer = reinterpret_cast<const uint8_t*>(ptr);
        return static_cast<uint16_t>(buffer[0]) | 
        (static_cast<uint16_t>(buffer[1])<<8);
    }

}

#endif