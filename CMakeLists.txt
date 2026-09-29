cmake_minimum_required(VERSION 3.16)

project( W65C02_emulation )

## Project-wide setup
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

option(W65C02_BUILD_TESTS "Build emulator tests" ON)
option(W65C02_BUILD_BENCHMARKS "Build emulator benchmarks" ON)
option(W65C02_PORTABLE "Resolve programs and tools beside the executable" OFF)
add_subdirectory(SystemLib)
if(W65C02_BUILD_TESTS)
    add_subdirectory(SystemTest)
endif()
if(W65C02_BUILD_BENCHMARKS)
    add_subdirectory(SystemBenchmark)
endif()
