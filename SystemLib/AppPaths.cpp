#include "AppPaths.h"
#include <stdexcept>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace fs = std::filesystem;
namespace AppPaths {
fs::path executableDirectory() {
#ifdef _WIN32
    std::vector<wchar_t> path(32768);
    const auto length = GetModuleFileNameW(nullptr, path.data(), DWORD(path.size()));
    if (!length || length >= path.size())
        throw std::runtime_error("Cannot locate the application directory");
    return fs::path(std::wstring(path.data(), length)).parent_path();
#else
    std::vector<char> path(32768);
    const auto length = readlink("/proc/self/exe", path.data(), path.size());
    if (length <= 0 || size_t(length) >= path.size())
        throw std::runtime_error("Cannot locate the application directory");
    return fs::path(std::string(path.data(), size_t(length))).parent_path();
#endif
}
fs::path asset(const char* name) {
    return executableDirectory() / "assets" / name;
}
fs::path programs() {
#ifdef W65C02_PORTABLE
    return executableDirectory() / "programs";
#else
    return IDE_PROGRAMS_PATH;
#endif
}
fs::path assembler() {
#ifdef W65C02_PORTABLE
#ifdef _WIN32
    return executableDirectory() / "tools" / "vasm6502_oldstyle.exe";
#else
    return executableDirectory() / "tools" / "vasm6502_oldstyle";
#endif
#else
    return IDE_ASSEMBLER_PATH;
#endif
}
fs::path builds() {
#ifdef W65C02_PORTABLE
    return executableDirectory() / "builds";
#else
    return IDE_BUILDS_PATH;
#endif
}
}
