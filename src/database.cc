#include <iostream>

#include "lumina/db.h"

namespace lumina{

/**
 * @brief Private implementation of the DB interface.
 * * We use this internal class to manage the lifecycle of the WAL, 
 * MemTable, and SSTables without exposing those complexities in the 
 * public include/lumina/db.h header.
 */
class DatabaseImpl: public DB{

public:

    DatabaseImpl(const Options& options, const std::string& name) : options_(options), name_(name){
        // initialization logic to be added
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

    Status Delete(const Slice& key){
        (void)key;
        return Status::OK();
    }

private:
    Options options_;
    std::string name_;
};


Status DB::Open(const Options& options, const std::string& name, DB** dbptr){
    *dbptr = nullptr;

    DatabaseImpl* implementation = new DatabaseImpl(options, name);
    *dbptr = implementation;

    return Status::OK();
}

}