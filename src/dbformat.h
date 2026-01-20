#ifndef LUMINA_DBFORMAT_H
#define LUMINA_DBFORMAT_H

#include <cstdint>
#include <string>
#include <cassert>
#include "lumina/common.h"
#include "coding.h"


namespace lumina{

    // tells what is the type this key version has. Is this a delete or value operation. Useful for WAL(append only), 
    // useful for snapshots.
    enum ValueType{ kTypeDeletion = 0, kTypeValue = 1};

    // stores the max version/sequence number any key can have -- 2^56. Very large.
    static const uint64_t kMaxSequenceNumber = ((uint64_t(1) << 56)-1);

    /**
     * @brief Logic for packing a user key, sequence number, and type.
     * * Format: [ User Key ] [ Sequence Number (56 bits) | Value Type (8 bits) ]
     */
    class InternalKey{
    public:
        InternalKey(){}     
        InternalKey(const Slice& user_key, uint64_t sequence, ValueType type){
            rep_.append(user_key.data(), user_key.size());

            // storing sequence number and type in a single 64 bit integer.
            // Encoding in little-endian and storing at the path
            uint64_t packed_data = (sequence << 8) | type;
            PutFixed64(&rep_, packed_data); 
        }

        Slice user_key() const{
        }

        uint64_t sequence() const{
        }

        ValueType type() const{
        }
    
    private:
        // owns the internal key memory.
        std::string rep_;

    };
}


#endif