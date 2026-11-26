#include "compiler.hpp"

#include <cctype>
#include <cstdlib>
#include <sstream>

#ifdef _WIN32
#define MDCOMPILER_WIN32 1
#else
#define MDCOMPILER_WIN32 0
#endif

namespace mdcompiler {
namespace {

std::string quote(const std::string& value) {
    std::string result = "\"";
    for (char character : value) {
        if (character == '"') {
            result.push_back('\\');
        }
        result.push_back(character);
    }
    result.push_back('"');
    return result;
}

bool executableExists(const std::string& name) {
    std::ostringstream command;
    if (MDCOMPILER_WIN32) {
        command << "where.exe " << quote(name) << " >nul 2>&1";
    } else {
        command << "command -v " << quote(name) << " >/dev/null 2>&1";
    }
    return std::system(command.str().c_str()) == 0;
}

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.size() >= prefix.size() &&
        value.compare(0, prefix.size(), prefix) == 0;
}

bool isMicrosoftCompiler(const std::string& compiler) {
    std::string name = compiler;
    for (char& character : name) {
        character = static_cast<char>(
            std::tolower(static_cast<unsigned char>(character))
        );
    }
    return name == "cl" || name == "cl.exe";
}

} // namespace

Language languageFromSource(const std::filesystem::path& source) {
    std::string extension = source.extension().string();
    for (char& character : extension) {
        character = static_cast<char>(
            std::tolower(static_cast<unsigned char>(character))
        );
    }
    return extension == ".c" ? Language::C : Language::Cpp;
}

std::string resolveCompiler(Language language, const std::string& requested) {
    if (!requested.empty()) {
        return requested;
    }

    const char* configured = std::getenv(
        language == Language::C ? "MDCOMPILER_CC" : "MDCOMPILER_CXX"
    );
    if (configured != nullptr && configured[0] != '\0') {
        return configured;
    }

    const char* candidates[] = {
        language == Language::C ? "clang" : "clang++",
        language == Language::C ? "gcc" : "g++",
        "cl",
        language == Language::C ? "cc" : "c++"
    };
    for (const char* candidate : candidates) {
        if (executableExists(candidate)) {
            return candidate;
        }
    }

    return "";
}

std::string buildCommand(
    const std::string& compiler,
    Language language,
    const std::filesystem::path& source,
    const std::filesystem::path& output,
    const CompileOptions& options
) {
    std::ostringstream command;
    if (compiler.find(' ') == std::string::npos) {
        command << compiler;
    } else {
        command << quote(compiler);
    }

    if (isMicrosoftCompiler(compiler)) {
        command << " /nologo";
        command << (language == Language::C ? " /TC" : " /TP");
        if (language == Language::Cpp) {
            command << " /EHsc";
        }
        if (!options.standard.empty()) {
            command << " /std:" << options.standard;
        }
        if (!options.optimize.empty()) {
            command << (options.optimize == "0" ? " /Od" : " /O" + options.optimize);
        }
        if (options.debug) {
            command << " /Zi";
        }
        for (const std::string& include : options.includes) {
            command << " /I" << quote(include);
        }
        for (const std::string& define : options.defines) {
            command << " /D" << quote(define);
        }
        command << " " << quote(source.string());
        command << " /Fe" << quote(output.string());
        if (options.debug) {
            command << " /link /DEBUG";
        }
    } else {
        if (!options.standard.empty()) {
            command << " -std=" << options.standard;
        }
        command << " -O" << options.optimize;
        if (options.debug) {
            command << " -g";
        }
        for (const std::string& include : options.includes) {
            command << " -I " << quote(include);
        }
        for (const std::string& define : options.defines) {
            command << " -D" << quote(define);
        }
        command << " " << quote(source.string());
        command << " -o " << quote(output.string());
    }
    return command.str();
}

} // namespace mdcompiler
