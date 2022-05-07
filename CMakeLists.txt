cmake_minimum_required(VERSION 3.16)

project( W65C02_emulation )

## Project-wide setup
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

add_subdirectory(SystemLib)
add_subdirectory(SystemTest)
add_subdirectory(SystemBenchmark)