cmake_minimum_required(VERSION 3.16)

project( SystemLib )

set(SYSTEM_LIB_SOURCES
        "System.cpp"
        "vrEmuLcd.cpp"
        "PortBus.cpp"
        "W65C02.cpp"
        "W65C22.cpp"
        "W65C51.cpp"
        "SerialTerminal.cpp"
        "EEPROM.cpp"
        "RAM.cpp"
        "Bus.cpp"
        "Cycles.cpp"
        "Lights.cpp"
        "LCD.cpp")

# The SFML distribution checked into this repository is for Windows.
# On Linux, use the installed SFML 2 development package.
if(WIN32)
    set(SFML_STATIC_LIBRARIES true)
    set(SFML_DIR ${CMAKE_SOURCE_DIR}/SFML/lib/cmake/SFML)
endif()
find_package(Threads REQUIRED)
find_package(SFML 2 COMPONENTS system window graphics REQUIRED)

add_library(SystemLib ${SYSTEM_LIB_SOURCES})
target_include_directories(SystemLib PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(SystemLib PUBLIC Threads::Threads)

add_library(EmulatorIdeCore AsmDocument.cpp IdeWorkspace.cpp)
target_include_directories(EmulatorIdeCore PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_features(EmulatorIdeCore PUBLIC cxx_std_17)
target_link_libraries(EmulatorIdeCore PUBLIC Threads::Threads)

add_executable(main main.cpp LcdPanel.cpp BreadboardView.cpp EmulatorIde.cpp)
configure_file(${CMAKE_SOURCE_DIR}/firmware/roms/wozmon-basic.bin
               ${CMAKE_CURRENT_BINARY_DIR}/assets/wozmon-basic.bin COPYONLY)
target_compile_definitions(main PRIVATE
    IDE_FIRMWARE_PATH="${CMAKE_CURRENT_BINARY_DIR}/assets/wozmon-basic.bin")
# Reuse the font shipped with SFML; no host font installation is needed.
configure_file(${CMAKE_SOURCE_DIR}/SFML/examples/shader/resources/sansation.ttf
               ${CMAKE_CURRENT_BINARY_DIR}/assets/sansation.ttf COPYONLY)
target_compile_definitions(main PRIVATE
    BREADBOARD_FONT_PATH="${CMAKE_CURRENT_BINARY_DIR}/assets/sansation.ttf")
configure_file(${CMAKE_SOURCE_DIR}/assets/fonts/DejaVuSansMono.ttf
               ${CMAKE_CURRENT_BINARY_DIR}/assets/DejaVuSansMono.ttf COPYONLY)
if(WIN32)
    set(IDE_ASSEMBLER ${CMAKE_SOURCE_DIR}/VASM/vasm6502_oldstyle.exe)
else()
    set(IDE_ASSEMBLER ${CMAKE_SOURCE_DIR}/VASM/vasm6502_oldstyle)
endif()
target_compile_definitions(main PRIVATE
    IDE_CODE_FONT_PATH="${CMAKE_CURRENT_BINARY_DIR}/assets/DejaVuSansMono.ttf"
    IDE_PROGRAMS_PATH="${CMAKE_SOURCE_DIR}/VASM"
    IDE_ASSEMBLER_PATH="${IDE_ASSEMBLER}"
    IDE_BUILDS_PATH="${CMAKE_BINARY_DIR}/ide")
target_link_libraries(main PRIVATE SystemLib EmulatorIdeCore sfml-graphics sfml-window sfml-system)

if(WIN32)
    target_link_libraries(main PRIVATE -mwindows -static -static-libstdc++ -static-libgcc)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(X11 REQUIRED)
    target_link_libraries(main PRIVATE X11::X11)
endif()
