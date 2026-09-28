#ifndef W65C02_IDE_WORKSPACE_H
#define W65C02_IDE_WORKSPACE_H
#include "AsmDocument.h"
#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include <cstdint>

struct AsmBuildResult {
    bool success = false;
    int errorLine = 0;
    std::string output;
    std::filesystem::path rom;
    std::map<uint16_t, size_t> sourceLines; // Main source only; one-based lines.
    std::map<size_t, std::vector<uint16_t>> breakpointAddresses; // Includes label aliases.
};
class IdeWorkspace {
public:
    explicit IdeWorkspace(std::filesystem::path directory);
    std::vector<std::string> programs() const;
    AsmDocument open(const std::string& name) const;
    void save(const std::string& name, AsmDocument& document) const;
    void validateNewName(const std::string& name) const;
    const std::filesystem::path& directory() const { return root; }
    static std::string readText(const std::filesystem::path& path);
    static std::string starterProgram();
private:
    std::filesystem::path root;
    std::filesystem::path pathFor(const std::string& name) const;
};
AsmBuildResult assembleProgram(const std::filesystem::path& assembler,
                              const std::filesystem::path& programDirectory,
                              const std::filesystem::path& buildDirectory,
                              const std::string& filename, const std::string& source);
#endif
