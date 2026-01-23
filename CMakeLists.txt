cmake_minimum_required(VERSION 3.15)
project(Lumina VERSION 0.1.0 LANGUAGES CXX)

# Use C++20 for modern features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable all warnings and treat them as errors 
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror)
endif()

# Define the library
add_library(lumina
    src/database.cc
    src/crc32.cc
    src/log_writer.cc
    src/coding.cc

    # Optional: headers for IDE visibility
    include/lumina/common.h
    include/lumina/db.h
    include/lumina/options.h
    src/log_format.h
    src/log_writer.h
    src/crc32.h
    src/coding.h
    src/skiplist.h
    src/dbformat.h
)


# Modern CMake: Scoped include directories
# PUBLIC means "I need this to compile, and anyone using me needs it too."
target_include_directories(lumina PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# Google Test
find_package(GTest REQUIRED)

add_executable(lumina_test
    tests/coding_test.cc
    tests/skiplist_test.cc
)

target_include_directories(lumina_test PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(lumina_test PRIVATE lumina GTest::GTest GTest::Main pthread)
add_test(NAME LuminaCodingTest COMMAND $<TARGET_FILE:lumina_test>)

enable_testing()