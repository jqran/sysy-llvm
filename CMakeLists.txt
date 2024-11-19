cmake_minimum_required(VERSION 3.10)

# Project name
project(testllvm LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# Include directories
include_directories(include)

# Link directories
link_directories(/usr/lib)

# Source files
file(GLOB_RECURSE SOURCES "src/main.cpp" "src/frontend/*.cpp")

# Create the executable
add_executable(testllvm ${SOURCES})

# Link against the LLVM library
target_link_libraries(testllvm PRIVATE LLVM)

#target_link_libraries(testllvm PRIVATE /usr/lib/libLLVM.so)

# Set optimization level
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
