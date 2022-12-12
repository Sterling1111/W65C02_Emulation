cmake_minimum_required(VERSION 3.14)
project( SystemTest )
enable_testing()

include(FetchContent)

FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
        )

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