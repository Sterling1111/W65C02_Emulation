cmake_minimum_required(VERSION 3.14)
project( SystemTest )
enable_testing()

cmake_policy(SET CMP0135 NEW)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# source for the test executable
set  (TEST_SOURCES
        "IdeTests.cpp"
        "CpuConformanceTests.cpp"
        "SerialTests.cpp"
        "TimingTests.cpp"
        "W65C22Tests.cpp"
        "LCDTests.cpp"
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
target_link_libraries(SystemTest gtest SystemLib EmulatorIdeCore)
target_compile_definitions(SystemTest PRIVATE
    TEST_SERIAL_ROM="${CMAKE_SOURCE_DIR}/firmware/roms/wozmon-basic.bin")
if(WIN32)
    target_compile_definitions(SystemTest PRIVATE TEST_VASM_PATH="${CMAKE_SOURCE_DIR}/VASM/vasm6502_oldstyle.exe")
else()
    target_compile_definitions(SystemTest PRIVATE TEST_VASM_PATH="${CMAKE_SOURCE_DIR}/VASM/vasm6502_oldstyle")
endif()

# Optional firmware integration check; pass an assembled via_demo.asm ROM.
add_executable(ViaDemoSmoke EXCLUDE_FROM_ALL ViaDemoSmoke.cpp)
target_link_libraries(ViaDemoSmoke PRIVATE SystemLib)

# Optional integration check for VASM/star_dodge.asm; pass its assembled ROM.
add_executable(StarDodgeSmoke EXCLUDE_FROM_ALL StarDodgeSmoke.cpp)
target_link_libraries(StarDodgeSmoke PRIVATE SystemLib)
