#include <iostream>
#include <fstream>
#include <string>
#include "log_writer.h"
#include "lumina/common.h"

int main() {
    const std::string log_filename = "test.log";
    std::ofstream file(log_filename, std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        std::cerr << "Failed to open log file for writing." << std::endl;
        return 1;
    }

    lumina::log::Writer writer(&file);

    std::cout << "Starting WAL Test..." << std::endl;


    // Test 1: Small records
    std::cout << "Writing small records..." << std::endl;
    {
        lumina::Status s1 = writer.AddRecord(lumina::Slice("key1:value1"));
        lumina::Status s2 = writer.AddRecord(lumina::Slice("key2:value2"));

        if (!s1.ok()) {
            std::cerr << "Error writing small record 1: " << s1.ToString() << std::endl;
        }
        if (!s2.ok()) {
            std::cerr << "Error writing small record 2: " << s2.ToString() << std::endl;
        }
    }

    // Test 2: Medium record (~100 bytes)
    std::cout << "Writing medium record (~100 bytes)..." << std::endl;
    {
        std::string medium_data(100, 'a');
        lumina::Status s = writer.AddRecord(lumina::Slice(medium_data));

        if (!s.ok()) {
            std::cerr << "Error writing medium record: " << s.ToString() << std::endl;
        }
    }


    // Test 3: Large record (forces fragmentation across 32KB blocks)
    std::cout << "Writing a large 40KB record (forces fragmentation)..." << std::endl;
    {
        std::string large_data(40000, 'b');
        lumina::Status s = writer.AddRecord(lumina::Slice(large_data));

        if (!s.ok()) {
            std::cerr << "Error writing large record: " << s.ToString() << std::endl;
        }
    }

    file.close();

    std::cout << "\nWAL test complete. File written to: " << log_filename << std::endl;
    std::cout << "\nInspect the WAL contents with:" << std::endl;
    std::cout << "  ls -lh " << log_filename << std::endl;
    std::cout << "  hexdump -C " << log_filename << " | head -n 40" << std::endl;
    std::cout << "\nCheck the first 4 bytes = CRC, next 2 bytes = length, 7th byte = type" << std::endl;
    std::cout << "Scroll to offset 0x8000 (32768 decimal) to see large record fragmentation." << std::endl;

    return 0;
}
