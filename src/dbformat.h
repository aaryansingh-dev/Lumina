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
    static const int packedSize = 8;

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
            uint64_t packed_data = (sequence << packedSize) | type;
            PutFixed64(&rep_, packed_data); 
        }

        Slice user_key() const{
            assert(rep_.size() >= 8);
            return Slice(rep_.data(), rep_.size()-packedSize);     // remove meta-data from the end
        }

        uint64_t sequence() const{
            assert(rep_.size() >= packedSize);
            uint64_t packed = DecodeFixed64(rep_.data() + rep_.size() - packedSize);
            return packed >> packedSize;
        }

        ValueType type() const{
            assert(rep_.size() >= packedSize);
            uint64_t packed = DecodeFixed64(rep_.data() + rep_.size() - packedSize);
            return static_cast<ValueType>(packed & (0xff));
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
            Slice user_a(a.data(), a.size() - packedSize);
            Slice user_b(b.data(), b.size() - packedSize);
            int r = user_a.compare(user_b);
            
            if (r == 0) {
                // 2. Compare sequence numbers (Descending for newest first)
                uint64_t num_a = DecodeFixed64(a.data() + a.size() - packedSize);
                uint64_t num_b = DecodeFixed64(b.data() + b.size() - packedSize);
                if (num_a > num_b) r = -1;
                else if (num_a < num_b) r = +1;
            }
            return r;
        }
    };


    /**
     * @brief Helper for lookups to avoid unnecessary memory allocations.
     * * Design Decision: LookupKey constructs a single contiguous buffer on 
     * the stack that includes the length-prefix. This allows the MemTable 
     * to use a very fast memcmp-based search.
     */
    class LookupKey {
    public:
        // Initialize a helper for looking up user_key at a specific sequence.
        LookupKey(const Slice& user_key, uint64_t sequence) {
            size_t usize = user_key.size();
            size_t needed = usize + 5 + packedSize;  // 5 (max varint32) + usize + 8 (fixed64)
            char* dst;
            if (needed <= sizeof(space_)) {
                dst = space_;
            } else {
                dst = new char[needed];
            }
            start_ = dst;
            // 1. Store length of [user_key + 8 bytes for seq/type]
            kstart_ = EncodeVarint32(dst, static_cast<uint32_t>(usize + packedSize));
            // 2. Store user key
            memcpy(const_cast<char*>(kstart_), user_key.data(), usize);
            // 3. Store packed seq/type (using kTypeValue as it's the search boundary)
            char* end_ptr = const_cast<char*>(kstart_) + usize;
            EncodeFixed64(end_ptr, (sequence << packedSize) | kTypeValue);
            end_ = end_ptr + packedSize;
        }

        ~LookupKey() {
            if (start_ != space_) delete[] start_;
        }

        // Returns a slice suitable for MemTable lookups (includes length prefix)
        Slice memtable_key() const { return Slice(start_, end_ - start_); }

        // Returns a slice of the internal key (no length prefix)
        Slice internal_key() const { return Slice(kstart_, end_ - kstart_); }

        // Returns the user key
        Slice user_key() const { return Slice(kstart_, end_ - kstart_ - packedSize); }

    private:
        const char* start_;
        const char* kstart_;
        const char* end_;
        char space_[200]; // Optimization: avoid heap for most keys

        LookupKey(const LookupKey&) = delete;
        LookupKey& operator=(const LookupKey&) = delete;
    };

}


#endif