#ifndef LUMINA_DB_H
#define LUMINA_DB_H

#include <string>
#include <memory>
#include "options.h"
#include "common.h"


namespace lumina{


class DB {

public:

    static Status Open(const Options& options, const std::string& name, DB** dbptr);

    /**
     * This is a destructor. Virtual is used to make the destructor call implementation's destructor first to 
     * free up the memory and avoid memory leaks. 
     */
    virtual ~DB() = default;

    /**
     * @brief Set the database entry for "key" to "value".
     */
    virtual Status Put(const Slice& key, const Slice& value) = 0;

    /**
     * @brief If the database has an entry for "key", store the corresponding
     * value in *value and return Status::OK().
     * * If there is no entry for "key", return a Status::NotFound() error.
     * 
     * Pointer is created for std::string, to avoid dangling pointers after the memtable is flushed to memory.
     */
    virtual Status Get(const Slice& key, const std::string* value) = 0;


    /**
     * @brief Remove the database entry for "key".
     * It is not an error if "key" did not exist in the database.
     */
    virtual Status Delete(const Slice& key) = 0;


protected:

    DB() = default;


private:

    DB(const DB&) = delete;
    DB& operator=(const DB&) = delete;

};   

}

#endif