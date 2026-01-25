#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <string>

#include "coding.h"
#include "crc32.h"
#include "log_writer.h"

using namespace lumina;
using namespace lumina::log;

class LogWriterTest : public ::testing::Test {
protected:
    void SetUp() override {
        temp_path_ = std::filesystem::temp_directory_path() / "lumina_wal_test.log";
        ofs_.open(temp_path_, std::ios::binary | std::ios::trunc);
        ASSERT_TRUE(ofs_.is_open());
        writer_ = std::make_unique<Writer>(&ofs_);
    }

    void TearDown() override {
        ofs_.close();
        std::filesystem::remove(temp_path_);
    }

    std::string ReadFile() {
        ofs_.flush();
        std::ifstream ifs(temp_path_, std::ios::binary);
        return std::string(
            (std::istreambuf_iterator<char>(ifs)),
            std::istreambuf_iterator<char>()
        );
    }

    std::filesystem::path temp_path_;
    std::ofstream ofs_;
    std::unique_ptr<Writer> writer_;
};

// ------------------------------------------------------------
// Single small record → FULL
// ------------------------------------------------------------
TEST_F(LogWriterTest, WritesSingleSmallRecord) {
    ASSERT_TRUE(writer_->AddRecord(Slice("hello")).ok());

    std::string data = ReadFile();
    ASSERT_EQ(data.size(), kHeaderSize + 5);

    uint32_t crc = DecodeFixed32(data.data());
    uint16_t len = DecodeFixed16(data.data() + 4);
    uint8_t type = data[6];

    EXPECT_EQ(len, 5);
    EXPECT_EQ(type, kFullType);
    EXPECT_EQ(
        std::string(data.data() + kHeaderSize, len),
        "hello"
    );

    uint32_t expected_crc = crc32::Value(&data[6], 1);
    expected_crc = crc32::Extend(expected_crc, "hello", 5);
    expected_crc = crc32::Mask(expected_crc);

    EXPECT_EQ(crc, expected_crc);
}

// ------------------------------------------------------------
// Multiple records
// ------------------------------------------------------------
TEST_F(LogWriterTest, WritesMultipleRecords) {
    ASSERT_TRUE(writer_->AddRecord(Slice("one")).ok());
    ASSERT_TRUE(writer_->AddRecord(Slice("two")).ok());
    ASSERT_TRUE(writer_->AddRecord(Slice("three")).ok());

    std::string data = ReadFile();
    size_t offset = 0;
    int count = 0;

    while (offset < data.size()) {
        uint16_t len = DecodeFixed16(data.data() + offset + 4);
        uint8_t type = data[offset + 6];

        EXPECT_EQ(type, kFullType);
        offset += kHeaderSize + len;
        count++;
    }

    EXPECT_EQ(count, 3);
}

// ------------------------------------------------------------
// Record spanning multiple blocks
// ------------------------------------------------------------
TEST_F(LogWriterTest, RecordSpansMultipleBlocks) {
    std::string big(kBlockSize * 2, 'x');
    ASSERT_TRUE(writer_->AddRecord(Slice(big)).ok());

    std::string data = ReadFile();
    size_t offset = 0;

    bool first = false, middle = false, last = false;

    while (offset < data.size()) {
        uint8_t type = data[offset + 6];
        if (type == kFirstType) first = true;
        if (type == kMiddleType) middle = true;
        if (type == kLastType) last = true;

        uint16_t len = DecodeFixed16(data.data() + offset + 4);
        offset += kHeaderSize + len;
    }

    EXPECT_TRUE(first);
    EXPECT_TRUE(middle);
    EXPECT_TRUE(last);
}

// ------------------------------------------------------------
// CRC correctness for fragmented records
// ------------------------------------------------------------
TEST_F(LogWriterTest, CRCIsCorrectForFragments) {
    std::string big(kBlockSize, 'z');
    ASSERT_TRUE(writer_->AddRecord(Slice(big)).ok());

    std::string data = ReadFile();
    size_t offset = 0;

    while (offset < data.size()) {
        uint32_t crc = DecodeFixed32(data.data() + offset);
        uint16_t len = DecodeFixed16(data.data() + offset + 4);
        uint8_t type = data[offset + 6];

        const char* payload = data.data() + offset + kHeaderSize;

        uint32_t expected = crc32::Value(
            reinterpret_cast<const char*>(&type), 1
        );
        expected = crc32::Extend(expected, payload, len);
        expected = crc32::Mask(expected);

        EXPECT_EQ(crc, expected);

        offset += kHeaderSize + len;
    }
}
