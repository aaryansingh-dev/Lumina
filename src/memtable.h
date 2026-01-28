#ifndef LUMINA_MEMTABLE_H
#define LUMINA_MEMTABLE_H

#include <string>
#include "lumina/db.h"
#include "dbformat.h"
#include "skiplist.h"

namespace lumina {

/**
 * @brief MemTable wraps the SkipList to store InternalKeys.
 * * Design Decision: The SkipList stores raw "char*" entries that are 
 * length-prefixed. This class handles the conversion between user-facing 
 * Slices and the internal memory format.
 */
class MemTable {
public:
    struct KeyComparator {
        const InternalKeyComparator comparator;
        explicit KeyComparator(InternalKeyComparator c) : comparator(c) {}
        
        // This operator is used by the SkipList to compare two entries
        int operator()(const char* a, const char* b) const;
    };

    explicit MemTable(const InternalKeyComparator& comparator);
    ~MemTable();

    // Increase reference count (for future Immutable MemTable support)
    void Ref() { ++refs_; }
    // Decrease reference count; delete if it drops to zero
    void Unref() {
        --refs_;
        assert(refs_ >= 0);
        if (refs_ <= 0) delete this;
    }

    /**
     * @brief Adds an entry to the MemTable.
     * Format: [Varint32 InternalKey Size][InternalKey Bytes]
     */
    void Add(uint64_t sequence, ValueType type, const Slice& key, const Slice& value);

    /**
     * @brief Search for a key. Returns true and populates *value if found.
     */
    bool Get(const LookupKey& key, std::string* value, Status* s);

    // Approximate memory usage of the MemTable
    size_t ApproximateMemoryUsage() const { return usage_.load(std::memory_order_relaxed); }

private:
    typedef SkipList<const char*, KeyComparator> Table;

    KeyComparator comparator_;
    int refs_;
    Table table_;
    std::atomic<size_t> usage_;

    MemTable(const MemTable&) = delete;
    void operator=(const MemTable&) = delete;
};

} // namespace lumina

#endif // LUMINA_MEMTABLE_H