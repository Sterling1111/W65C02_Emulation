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
    if(NOT SFML_DIR)
        set(SFML_DIR ${CMAKE_SOURCE_DIR}/SFML/lib/cmake/SFML)
    endif()
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

add_executable(main main.cpp AppPaths.cpp WindowTheme.cpp LcdPanel.cpp BreadboardView.cpp EmulatorIde.cpp)
if(W65C02_PORTABLE)
    target_compile_definitions(main PRIVATE W65C02_PORTABLE)
    set_target_properties(main PROPERTIES OUTPUT_NAME "W65C02-Studio")
endif()

# Embed the window icon so it does not depend on the working directory.
set(APP_ICON_PNG "${CMAKE_SOURCE_DIR}/assets/icons/cpu.png")
set(APP_ICON_ICO "${CMAKE_SOURCE_DIR}/assets/icons/cpu.ico")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
             "${APP_ICON_PNG}" "${APP_ICON_ICO}")
file(READ "${APP_ICON_PNG}" APP_ICON_HEX HEX)
string(REGEX REPLACE "(..)" "0x\\1," APP_ICON_BYTES "${APP_ICON_HEX}")
configure_file(AppIcon.h.in generated/AppIcon.h @ONLY)
target_include_directories(main PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
if(WIN32)
    enable_language(RC)
    configure_file(AppIcon.rc.in generated/AppIcon.rc @ONLY)
    target_sources(main PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated/AppIcon.rc")
    set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/generated/AppIcon.rc"
        PROPERTIES OBJECT_DEPENDS "${APP_ICON_ICO}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    configure_file(w65c02-studio.desktop.in generated/w65c02-studio.desktop.in @ONLY)
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/w65c02-studio.desktop"
         INPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/w65c02-studio.desktop.in")
endif()
configure_file(${CMAKE_SOURCE_DIR}/firmware/roms/wozmon-basic.bin
               ${CMAKE_CURRENT_BINARY_DIR}/assets/wozmon-basic.bin COPYONLY)
# Bundle the OFL release of Sansation; no host font installation is needed.
configure_file(${CMAKE_SOURCE_DIR}/assets/fonts/Sansation-Regular.ttf
               ${CMAKE_CURRENT_BINARY_DIR}/assets/sansation.ttf COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/assets/fonts/DejaVuSansMono.ttf
               ${CMAKE_CURRENT_BINARY_DIR}/assets/DejaVuSansMono.ttf COPYONLY)
if(WIN32)
    set(IDE_ASSEMBLER ${CMAKE_SOURCE_DIR}/VASM/vasm6502_oldstyle.exe)
else()
    set(IDE_ASSEMBLER ${CMAKE_SOURCE_DIR}/VASM/vasm6502_oldstyle)
endif()
target_compile_definitions(main PRIVATE
    IDE_PROGRAMS_PATH="${CMAKE_SOURCE_DIR}/VASM"
    IDE_ASSEMBLER_PATH="${IDE_ASSEMBLER}"
    IDE_BUILDS_PATH="${CMAKE_BINARY_DIR}/ide")
target_link_libraries(main PRIVATE SystemLib EmulatorIdeCore sfml-graphics sfml-window sfml-system)

if(WIN32)
    target_link_libraries(main PRIVATE dwmapi -mwindows -static -static-libstdc++ -static-libgcc)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(X11 REQUIRED)
    target_link_libraries(main PRIVATE X11::X11)
endif()
