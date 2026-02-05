#include <iostream>
#include <mutex>
#include <fstream>

#include "lumina/db.h"
#include "lumina/options.h"
#include "log_writer.h"
#include "dbformat.h"
#include "memtable.h"

namespace lumina{

/**
 * @brief Private implementation of the DB interface.
 * * We use this internal class to manage the lifecycle of the WAL, 
 * MemTable, and SSTables without exposing those complexities in the 
 * public include/lumina/db.h header.
 */
class DatabaseImpl: public DB{

public:

    DatabaseImpl(const Options& options, const std::string& name) : 
    options_(options), 
    name_(name),
    mem_(nullptr),
    log_(nullptr)   {
        
        InternalKeyComparator cmp;
        mem_ = new MemTable(cmp);
    }

    virtual ~DatabaseImpl() override{
        if (log_) delete log_;
        if (mem_) {
            mem_->Unref(); // Drop our reference
        }
    }

    Status Put(const Slice& key, const Slice& value) override{
        std::lock_guard<std::mutex> lock(mutex_);
        
        // 1. Assign a Sequence Number
        // In a real engine, this increments. For this phase, we use 1 
        // until we implement the VersionSet.
        uint64_t seq = 1; 

        // 2. Create the Internal Key (UserKey + Seq + Type)
        // We need this for the WAL entry so we can recover the exact operation.
        InternalKey internal_key(key, seq, kTypeValue);
        
        // 3. Write to the Write-Ahead Log (Durability)
        // Format: [Internal Key Size] [Internal Key] [Value Size] [Value]
        // We manually serialize using EncodeVarint32 since PutVarint32 is unavailable.
        std::string entry;
        char varint_buf[5];
        char* ptr;

        // Encode Internal Key Size
        ptr = EncodeVarint32(varint_buf, static_cast<uint32_t>(internal_key.Encode().size()));
        entry.append(varint_buf, ptr - varint_buf);
        // Append Internal Key
        entry.append(internal_key.Encode().data(), internal_key.Encode().size());
        
        // Encode Value Size
        ptr = EncodeVarint32(varint_buf, static_cast<uint32_t>(value.size()));
        entry.append(varint_buf, ptr - varint_buf);
        // Append Value
        entry.append(value.data(), value.size());
        
        Status s = log_->AddRecord(Slice(entry));
        if (!s.ok()) {
            return s;
        }

        // 4. Write to MemTable (Visibility)
        mem_->Add(seq, kTypeValue, key, value);

        return Status::OK();
    }
    
    Status Get(const Slice& key, std::string* value) override {
        // we need a consistent view of the sequence number in a full engine.
        // For now, we look for the latest version (kMaxSequenceNumber).
        LookupKey lkey(key, kMaxSequenceNumber);
        
        Status s;
        if (mem_->Get(lkey, value, &s)) {
            // Found in MemTable (either Value or Deleted)
            if (s.IsNotFound()) {
                return Status::NotFound("Key not found (deleted)");
            }
            return Status::OK();
        }

        // Phase 2: We only check MemTable. 
        // Phase 3 will add SSTable lookup here.
        return Status::NotFound("Key not found");
    }

    Status Delete(const Slice& key) override {
        std::lock_guard<std::mutex> lock(mutex_);
        uint64_t seq = 2; // Placeholder sequence

        // Deletion is just writing a new record with kTypeDeletion
        InternalKey internal_key(key, seq, kTypeDeletion);

        // Serialize for WAL (Value is empty for deletion)
        std::string entry;
        char varint_buf[5];
        char* ptr;

        // Encode Internal Key Size
        ptr = EncodeVarint32(varint_buf, static_cast<uint32_t>(internal_key.Encode().size()));
        entry.append(varint_buf, ptr - varint_buf);
        entry.append(internal_key.Encode().data(), internal_key.Encode().size());
        
        // Encode Value Size (0)
        ptr = EncodeVarint32(varint_buf, 0); 
        entry.append(varint_buf, ptr - varint_buf);
        
        Status s = log_->AddRecord(Slice(entry));
        if (!s.ok()) return s;

        // Add Tombstone to MemTable
        mem_->Add(seq, kTypeDeletion, key, Slice());
        
        return Status::OK();
    }

private:
    Options options_;
    std::string name_;

    std::mutex mutex_;
    MemTable* mem_;

    std::ofstream logfile_;
    log::Writer* log_;
};


Status DB::Open(const Options& options, const std::string& name, DB** dbptr){
    *dbptr = nullptr;

    DatabaseImpl* implementation = new DatabaseImpl(options, name);
    *dbptr = implementation;

    return Status::OK();
}

}