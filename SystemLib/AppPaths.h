#pragma once
#include <filesystem>

namespace AppPaths {
std::filesystem::path executableDirectory();
std::filesystem::path asset(const char* name);
std::filesystem::path programs();
std::filesystem::path assembler();
std::filesystem::path builds();
}
