cmake_minimum_required(VERSION 3.16)

project( SystemLib )

set(SYSTEM_LIB_SOURCES
        "EEPROM.h"
        "Bus.h"
        "Cycles.h"
        "System.h"
        "LCD.cpp"
        "RAM.h"
        "system_types.h"
        "_65C02.cpp"
        "W65C22S.cpp"
        "main.cpp")

add_library( SystemLib ${SYSTEM_LIB_SOURCES})

add_executable(main ${SYSTEM_LIB_SOURCES})

target_include_directories ( SystemLib PUBLIC ${PROJECT_SOURCE_DIR})