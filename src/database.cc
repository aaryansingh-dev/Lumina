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
        // clean up logic
        // destructor
    }

    Status Put(const Slice& key, const Slice& value) override{
        // currently skeleton code
        (void)key;
        (void)value;
        return Status::OK();
    }
    
    Status Get(const Slice& key, const std::string* value) override {
        (void)key;
        (void)value;
        return Status::OK();
    }

    Status Delete(const Slice& key) override{
        (void)key;
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