#ifndef MDCOMPILER_COMPILER_HPP
#define MDCOMPILER_COMPILER_HPP

#include <filesystem>
#include <string>
#include <vector>

namespace mdcompiler {

enum class Language {
    C,
    Cpp
};

struct CompileOptions {
    std::string standard;
    std::string optimize = "0";
    bool debug = false;
    std::vector<std::string> includes;
    std::vector<std::string> defines;
};

Language languageFromSource(const std::filesystem::path& source);
std::string resolveCompiler(Language language, const std::string& requested = "");
std::string buildCommand(
    const std::string& compiler,
    Language language,
    const std::filesystem::path& source,
    const std::filesystem::path& output,
    const CompileOptions& options
);

} // namespace mdcompiler

#endif
