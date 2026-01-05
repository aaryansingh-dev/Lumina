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
    include/lumina/common.h
    include/lumina/db.h
    include/lumina/options.h
)

# Modern CMake: Scoped include directories
# PUBLIC means "I need this to compile, and anyone using me needs it too."
target_include_directories(lumina PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

enable_testing()