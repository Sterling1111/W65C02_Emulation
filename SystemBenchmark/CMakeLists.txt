cmake_minimum_required(VERSION 3.14)
project( SystemBenchmark )
enable_testing()

include(FetchContent)

FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG master) # need master for benchmark::benchmark

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googlebenchmark)

# source for the test executable
set  (Benchmark_SOURCES
        "main.cpp"
        "LoadRegisterBenchmarks.cpp")

add_executable( SystemBenchmark ${Benchmark_SOURCES} 	)
add_dependencies( SystemBenchmark SystemLib )
target_link_libraries(SystemBenchmark benchmark::benchmark SystemLib)