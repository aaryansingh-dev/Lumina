#include "coding.h"
#include "crc32.h"
#include "log_writer.h"

#include <algorithm>

namespace lumina{
namespace log{


Writer::Writer(std::ofstream* dest): dest_(dest), block_offset_(0){}

Writer::~Writer() = default;

Status Writer::AddRecord(const Slice& slice){
    const char* ptr = slice.data();
    size_t left = slice.size();
    
    // need to find if this slice can fit in the current block
    // if does not fit, we need to break it into multiple parts and store it
    Status s;
    bool begin = true;
    
    do{
    const int leftover = kBlockSize - block_offset_;
    assert(leftover >= 0);
      
    // if the leftover space is smaller than the header, we cannot fit anything.
    // pad the area and move to next block.
    if(leftover <= kHeaderSize){
        if (leftover > 0){
            // pad with 0s
            static const char kPadding[kHeaderSize] = {0};  // prevents dynamic allocation of the memory
            dest_->write(kPadding, leftover);
        }
        block_offset_ = 0;
    }

    const size_t avail = kBlockSize - block_offset_ - kHeaderSize;
    const size_t fragment_length = (left < avail) ? left : avail;

    RecordType type;
    const bool end = (left == fragment_length);
    if(begin && end){
        type = kFullType;
    }else if(begin){
        type = kFirstType;
    }else if(end){
        type = kLastType;
    }else{
        type = kMiddleType;
    }

    s = EmitPhysicalRecord(type, ptr, fragment_length);
    ptr += fragment_length;
    left -= fragment_length;
    begin = false;

    }while(s.ok() && left > 0);

    return s;
}


Status Writer::EmitPhysicalRecord(RecordType type, const char* ptr, size_t length){
    assert(length <= 0xffff); 
    assert(block_offset_ + kHeaderSize + length <= kBlockSize);

    char buf[kHeaderSize];
    // encode length (2 bytes)
    EncodeFixed16(&buf[4], static_cast<uint16_t>(length));
    // encode type (1 byte)
    buf[6] = static_cast<char>(type);
    
    // CRC of type + payload
    uint32_t crc = crc32::Value(&buf[6], 1);
    crc = crc32::Extend(crc, ptr, length);
    crc = crc32::Mask(crc);

    // Encode CRC (4 bytes)
    EncodeFixed32(buf, crc);

    dest_->write(buf, kHeaderSize);
    if (length > 0) {
        dest_->write(ptr, length);
    }
    dest_->flush();

    block_offset_ += static_cast<int>(kHeaderSize + length);
    return Status::OK();
}


} //log
} //lumina