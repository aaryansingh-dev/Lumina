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
            assert(rep_.size() >= 8);
            return Slice(rep_.data(), rep_.size()-8);     // remove meta-data from the end
        }

        uint64_t sequence() const{
            assert(rep_.size() >= 8);
            uint64_t packed = DecodeFixed64(rep_.data() + rep_.size() - 8);
            return packed >> 8;
        }

        ValueType type() const{
            assert(rep_.size() >= 8);
            uint64_t packed = DecodeFixed64(rep_.data() + rep_.size() - 8);
            return static_cast<ValueType>(packed && (0xff));
        }

        Slice Encode() const{
            return Slice(rep_);
        }

        void DecodeFrom(const Slice& s) { rep_.assign(s.data(), s.size()); }

        void Clear() { rep_.clear(); }
    
    private:
        // owns the internal key memory.
        std::string rep_;

    };

    /**
    * @brief Comparator for InternalKeys.
    */
    struct InternalKeyComparator {
        int operator()(const Slice& a, const Slice& b) const {
            // 1. Compare user keys
            Slice user_a(a.data(), a.size() - 8);
            Slice user_b(b.data(), b.size() - 8);
            int r = user_a.compare(user_b);
            
            if (r == 0) {
                // 2. Compare sequence numbers (Descending for newest first)
                uint64_t num_a = DecodeFixed64(a.data() + a.size() - 8);
                uint64_t num_b = DecodeFixed64(b.data() + b.size() - 8);
                if (num_a > num_b) r = -1;
                else if (num_a < num_b) r = +1;
            }
            return r;
        }
    };
}


#endif