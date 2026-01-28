#include "memtable.h"
#include "dbformat.h"
#include "coding.h"
#include <algorithm>
#include <cstring>


namespace lumina{

    constexpr int keyLengthSize = 5;
    constexpr int tagSize = 8;

/**
 * @brief Helper to extract the InternalKey slice from a raw MemTable entry.
 * Format: [Varint32 Length][InternalKey Bytes]
 */
static Slice GetInternalKeyFromEntry(const char* entry){
    uint32_t key_length;
    const char* key_ptr = GetVarint32Ptr(entry, entry+5, &key_length);
    return Slice(key_ptr, key_length);
}

/**
 * @brief Comparator implementation for the SkipList.
 * It decodes the length-prefixed entries and delegates to InternalKeyComparator.
 */
int MemTable::KeyComparator::operator()(const char* a, const char* b) const {
    Slice internalKey_a = GetInternalKeyFromEntry(a);
    Slice internalKey_b = GetInternalKeyFromEntry(b);

    return comparator(internalKey_a, internalKey_b);
}

/**
 * @brief Logic for extracting a slice that is prefixed by a varint32 length.
 */
static Slice GetLengthPrefixedSlice(const char* data) {
    uint32_t len;
    const char* p = GetVarint32Ptr(data, data + keyLengthSize, &len);
    return Slice(p, len);
}

MemTable::MemTable(const InternalKeyComparator& comparator):
    comparator_(KeyComparator(comparator)),
    refs_(0),
    table_(comparator_), // needs a skiplist but we can do an implicit conversion. -> table_(SkipList(comparator_)) = table_(comparator_)
    usage_(0){ }


MemTable::~MemTable(){
    assert(refs_ == 0);
}


void MemTable::Add(uint64_t sequence, ValueType type, const Slice& key, const Slice& value){

    // first get all the lengths of key and value
    size_t key_size = key.size();
    size_t value_size = value.size();
    size_t internal_key_size = key_size + tagSize;   // this 8 bytes stores 7 byte sequence + 1 byte type
    
    size_t encoded_length = VarintLength(key_size) + key_size + VarintLength(value_size) + value_size;

    char* buf = new char[encoded_length]; // buffer to store the key-value pair

    // store each part of the data : [Internal Key Len] [Key | Sequence | Type] [Value Len] [Value]
    char* p = EncodeVarint32(buf, static_cast<uint32_t>(internal_key_size));
    std::memcpy(p, key.data(), key_size);
    p+=key_size;
    EncodeFixed64(p, (sequence << tagSize)|type);
    p+=8;
    p = EncodeVarint32(p, static_cast<uint32_t>(value_size));
    std::memcpy(p, value.data(), value_size);

    // add this entry to the internal skiplist -> stores it in memory in a sorted order; log(n) time
    table_.Insert(buf);
    usage_.fetch_add(encoded_length, std::memory_order_relaxed);

}


bool MemTable::Get(const LookupKey& key, std::string* value, Status* s) {
    
    Slice memkey = key.memtable_key();  // [keyLen][key | sequence | type]
    Table::Iterator iter(&table_);
    iter.Seek(memkey.data());  // move the iterator to this value - the first node with user_key


    if (iter.Valid()){
        const char* entry = iter.key();
        uint32_t key_length;
        const char* key_ptr = GetVarint32Ptr(entry, entry + keyLengthSize, &key_length);

        // extract user key without tag
        Slice user_key_in_memtable(key_ptr, key_length - tagSize);
        if (user_key_in_memtable.compare(key.user_key()) == 0){
            // user key is same: excluding tag and prefix length
            uint64_t tag = DecodeFixed64(key_ptr + key_length - tagSize);
            switch (static_cast<ValueType>(tag & 0xff)) {
                case kTypeValue: {
                    // Extract value bytes located after the InternalKey
                    Slice v = GetLengthPrefixedSlice(key_ptr + key_length);
                    value->assign(v.data(), v.size());
                    return true;
                }
                case kTypeDeletion:
                    *s = Status::NotFound("");
                    return true;
            } 
        }

    }

    return false;

}


}