#include "coding.h"
#include "gtest/gtest.h"
#include <string>

using namespace lumina;

TEST(CodingTest, Fixed32EncodeDecode) {
    uint32_t values[] = {0, 1, 255, 256, 65535, 0x12345678, UINT32_MAX};

    for (uint32_t v : values) {
        char buf[4];
        EncodeFixed32(buf, v);
        uint32_t decoded = DecodeFixed32(buf);
        EXPECT_EQ(decoded, v);
    }
}

TEST(CodingTest, Fixed16EncodeDecode) {
    uint16_t values[] = {0, 1, 255, 256, 65535};

    for (uint16_t v : values) {
        char buf[2];
        EncodeFixed16(buf, v);
        uint16_t decoded = DecodeFixed16(buf);
        EXPECT_EQ(decoded, v);
    }
}

TEST(CodingTest, Fixed64EncodeDecode) {
    uint64_t values[] = {0, 1, 0xFFFF, 0x123456789ABCDEF0, UINT64_MAX};

    for (uint64_t v : values) {
        char buf[8];
        EncodeFixed64(buf, v);
        uint64_t decoded = DecodeFixed64(buf);
        EXPECT_EQ(decoded, v);
    }
}

TEST(CodingTest, PutFixed32Append) {
    std::string dst;
    PutFixed32(&dst, 0xDEADBEEF);
    EXPECT_EQ(dst.size(), 4u);
    EXPECT_EQ(DecodeFixed32(dst.data()), 0xDEADBEEF);
}

TEST(CodingTest, PutFixed64Append) {
    std::string dst;
    PutFixed64(&dst, 0x123456789ABCDEF0);
    EXPECT_EQ(dst.size(), 8u);
    EXPECT_EQ(DecodeFixed64(dst.data()), 0x123456789ABCDEF0);
}

// Optional: test Varint32 if implemented
TEST(CodingTest, Varint32EncodeDecode) {
    uint32_t values[] = {0, 1, 127, 128, 255, 300, 16384, UINT32_MAX};

    for (uint32_t v : values) {
        char buf[5];
        char* end = EncodeVarint32(buf, v);
        uint32_t decoded;
        const char* p = buf;
        p = GetVarint32Ptr(p, end, &decoded);
        EXPECT_EQ(decoded, v);
    }
}
