cmake_minimum_required(VERSION 3.14)
project( SystemTest )
enable_testing()

include(FetchContent)

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.10.x)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# source for the test executable
set  (TEST_SOURCES
        "LoggingTests.cpp"
        "LoadRegisterTests.cpp"
        "StoreRegisterTests.cpp"
        "JumpsAndCallsTests.cpp"
        "StackOperationsTests.cpp"
        "LogicalOperationsTests.cpp"
        "RegisterTransferTests.cpp"
        "ArithmeticOperationsTests.cpp"
        "main.cpp")

add_executable( SystemTest ${TEST_SOURCES} 	)
add_dependencies( SystemTest SystemLib )
target_link_libraries(SystemTest gtest SystemLib)