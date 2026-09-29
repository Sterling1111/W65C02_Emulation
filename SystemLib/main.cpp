#include "System.h"
#include "EmulatorIde.h"
#include "AppPaths.h"
#include <iostream>
#include <stdexcept>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

int main(int argc, char** argv) try {
    // System takes its clock frequency in MHz. The IDE can also change it live.
    constexpr double MHZ = 1.0;
    constexpr double KHZ = MHZ / 1000.0;
    constexpr double cpuMHz = 1 * KHZ;
    std::filesystem::path programs = AppPaths::programs();
    std::filesystem::path builds = AppPaths::builds();
    for (int i = 1; i < argc; ++i) {
        const std::string option = argv[i];
        if (option == "--programs" && i + 1 < argc) programs = argv[++i];
        else if (option == "--build-dir" && i + 1 < argc) builds = argv[++i];
        else if (option == "--help") {
            std::cout << "W65C02 Studio [--programs directory] [--build-dir directory]\n";
            return 0;
        } else throw std::runtime_error("Unknown or incomplete option: " + option);
    }
    System system{0x00, 0x3fff, 0x6000, 0x7fff,
                  0x8000, 0xffff, cpuMHz};
    EmulatorIde ide(system, programs, AppPaths::assembler(), builds);
    return ide.run();
} catch (const std::exception& error) {
    std::cerr << "W65C02 Studio: " << error.what() << '\n';
#ifdef _WIN32
    MessageBoxA(nullptr, error.what(), "W65C02 Studio - cannot start", MB_OK | MB_ICONERROR);
#endif
    return 1;
}
