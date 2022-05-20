cmake_minimum_required(VERSION 3.16)

project( SystemLib )

set(SYSTEM_LIB_SOURCES
        "System.cpp"
        "vrEmuLcd.cpp"
        "PortBus.cpp"
        "W65C02.cpp"
        "W65C22.cpp"
        "EEPROM.cpp"
        "RAM.cpp"
        "Bus.cpp"
        "main.cpp"
        "Cycles.cpp"
        "Lights.cpp"
        "LCD.cpp")

if(win32)
    set(SFML_STATIC_LIBRARIES TRUE)
endif()
set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR ${CMAKE_SOURCE_DIR}/SFML/lib/cmake/SFML)
find_package(SFML COMPONENTS system window graphics audio network REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/SFML/include)

add_library( SystemLib ${SYSTEM_LIB_SOURCES})

add_executable(main ${SYSTEM_LIB_SOURCES})
target_link_libraries(main sfml-system sfml-window sfml-graphics sfml-audio)

target_include_directories ( SystemLib PUBLIC ${PROJECT_SOURCE_DIR})
