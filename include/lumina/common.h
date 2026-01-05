#ifndef LUMINA_COMMON_H
#define LUMINA_COMMON_H

#include <string>
#include <string_view>
#include <cstring>
#include <cassert>

namespace lumina {

/**
 * @brief Slice is a performance utility. It points to existing memory
 * (const char*) rather than copying it.
 *
 * Design Decision: This is the foundation of our "Zero-Copy" architecture.
 * By passing a pointer and a length instead of a std::string, we avoid
 * heap allocations and data duplication during high-frequency writes.
 */
class Slice {
public:
    Slice() : data_(""), size_(0) {}
    Slice(const char* d, size_t n) : data_(d), size_(n) {}
    Slice(const std::string& s) : data_(s.data()), size_(s.size()) {}
    Slice(const char* s) : data_(s), size_(strlen(s)) {}

    const char* data() const { return data_; }
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    char operator[](size_t n) const {
        assert(n < size());
        return data_[n];
    }

    void clear() {
        data_ = "";
        size_ = 0;
    }

    // Three-way comparison (Lexicographical)
    // Used by the Skip List and SSTables to keep keys sorted.
    int compare(const Slice& b) const {
        const size_t min_len = (size_ < b.size_) ? size_ : b.size_;
        int r = memcmp(data_, b.data_, min_len);
        if (r == 0) {
            if (size_ < b.size_) r = -1;
            else if (size_ > b.size_) r = +1;
        }
        return r;
    }

    std::string ToString() const { return std::string(data_, size_); }

private:
    const char* data_;
    size_t size_;
};

inline bool operator==(const Slice& x, const Slice& y) {
    return ((x.size() == y.size()) &&
            (memcmp(x.data(), y.data(), x.size()) == 0));
}

inline bool operator!=(const Slice& x, const Slice& y) { return !(x == y); }

/**
 * @brief Status represents the result of an operation.
 *
 * Design Decision: Following Google's C++ style, we return Status objects
 * instead of throwing exceptions. This makes control flow explicit,
 * improves readability of failure paths, and avoids the performance
 * overhead of stack unwinding.
 */
class Status {
public:
    enum Code {
        kOk = 0,
        kNotFound = 1,
        kCorruption = 2,
        kIOError = 3,
        kNotSupported = 4,
        kInvalidArgument = 5
    };

    Status() : code_(kOk), msg_("") {}
    Status(Code code, std::string_view msg) : code_(code), msg_(msg) {}

    static Status OK() { return Status(); }
    static Status NotFound(std::string_view m) { return Status(kNotFound, m); }
    static Status Corruption(std::string_view m) { return Status(kCorruption, m); }
    static Status IOError(std::string_view m) { return Status(kIOError, m); }
    static Status InvalidArgument(std::string_view m) { return Status(kInvalidArgument, m); }

    bool ok() const { return code_ == kOk; }
    bool IsNotFound() const { return code_ == kNotFound; }
    bool IsCorruption() const { return code_ == kCorruption; }
    bool IsIOError() const { return code_ == kIOError; }

    std::string ToString() const {
        if (code_ == kOk) return "OK";
        std::string res;
        switch (code_) {
            case kNotFound:        res = "NotFound: "; break;
            case kCorruption:      res = "Corruption: "; break;
            case kIOError:         res = "IOError: "; break;
            case kInvalidArgument: res = "InvalidArgument: "; break;
            default:               res = "Error: "; break;
        }
        res.append(msg_);
        return res;
    }

private:
    Code code_;
    std::string msg_;
};

} // namespace lumina

#endif // LUMINA_COMMON_H