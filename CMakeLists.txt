cmake_minimum_required(VERSION 3.10)

# Project name
project(testllvm LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
SET(CMAKE_BUILD_TYPE "Debug" )

# Include directories
include_directories(include /usr/include/antlr4-runtime antlr)

# Link directories
link_directories(/usr/lib)

# Source files
file(GLOB_RECURSE SOURCES "src/test/testast.cpp" "src/midend/type.cpp" "src/midend/AntlrChecker.cpp" "src/midend/hir.cpp" "src/midend/builder.cpp" "antlr/*.cpp")

# Create the executable
add_executable(testllvm ${SOURCES})

# Link against the LLVM library
target_link_libraries(testllvm PRIVATE "/usr/lib/llvm-19/lib/libLLVM.so")

target_link_libraries(testllvm PRIVATE /usr/lib/x86_64-linux-gnu/libantlr4-runtime.a )

# Set optimization level
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

