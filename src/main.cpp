#include <cstdlib>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "compiler.hpp"

#ifdef _WIN32
#define MDCOMPILER_WIN32 1
#else
#define MDCOMPILER_WIN32 0
#endif

namespace fs = std::filesystem;

namespace {

void printUsage() {
    std::cout <<
        "mdcompiler - C/C++ compiler runner\n\n"
        "Kullanim:\n"
        "  mdcompiler build <kaynak.c|kaynak.cpp> [secenekler]\n"
        "  mdcompiler run <kaynak.c|kaynak.cpp> [secenekler] [-- <calistirma-parametreleri>]\n"
        "  mdcompiler compile <kaynak.c|kaynak.cpp> [secenekler]\n\n"
        "Secenekler:\n"
        "  -o, --output <dosya>  Cikti dosyasinin yolu\n"
        "  -c, --compiler <ad>   Derleyici adini zorla (g++, gcc, clang++, clang, cl)\n"
        "  -s, --std <surum>     Standart (ornegin c11, c17, c++17, c++20)\n"
        "  -O, --optimize <0-3>  Optimizasyon seviyesi\n"
        "  -g, --debug           Debug bilgisi ekle\n"
        "  -I, --include <klasor> Baslik klasoru ekle\n"
        "  -D, --define <NAME=X> Makro tanimla\n"
        "      --protocol        Makine okunabilir MDCOMPILER/1 protokolu kullan\n"
        "  -v, --version         Surum bilgisini yazdir\n"
        "  -h, --help            Yardimi yazdir\n";
}

std::string quote(const std::string& value) {
    std::string out = "\"";
    for (char ch : value) {
        if (ch == '"') {
            out.push_back('\\');
        }
        out.push_back(ch);
    }
    out.push_back('"');
    return out;
}

std::string quote(const fs::path& value) {
    return quote(value.string());
}

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.size() >= prefix.size() &&
        value.compare(0, prefix.size(), prefix) == 0;
}

std::string executableName(const fs::path& source, const fs::path& output) {
    fs::path result = output;
    if (result.empty()) {
        fs::create_directories("build");
        result = fs::path("build") / source.stem();
        if (MDCOMPILER_WIN32) {
            result.replace_extension(".exe");
        }
    }
    if (MDCOMPILER_WIN32 && result.extension() != ".exe") {
        result += ".exe";
    }
    fs::create_directories(result.parent_path().empty() ? fs::path(".") : result.parent_path());
    return fs::absolute(result).string();
}

struct Options {
    fs::path output;
    std::string compiler;
    std::string standard;
    std::string optimize = "0";
    bool debug = false;
    bool protocol = false;
    std::vector<std::string> includes;
    std::vector<std::string> defines;
};

int fail(const std::string& message) {
    std::cerr << "Hata: " << message << '\n';
    return 1;
}

bool parseOptions(
    const std::vector<std::string>& arguments,
    size_t& index,
    Options& options,
    std::vector<std::string>& runArguments
) {
    for (; index < arguments.size(); ++index) {
        const std::string& argument = arguments[index];
        if (argument == "--") {
            for (++index; index < arguments.size(); ++index) {
                runArguments.push_back(arguments[index]);
            }
            break;
        }
        if ((argument == "-o" || argument == "--output") && index + 1 < arguments.size()) {
            options.output = arguments[++index];
        } else if (startsWith(argument, "--output=")) {
            options.output = argument.substr(9);
        } else if ((argument == "-c" || argument == "--compiler") && index + 1 < arguments.size()) {
            options.compiler = arguments[++index];
        } else if (startsWith(argument, "--compiler=")) {
            options.compiler = argument.substr(11);
        } else if ((argument == "-s" || argument == "--std") && index + 1 < arguments.size()) {
            options.standard = arguments[++index];
        } else if ((argument == "-O" || argument == "--optimize") && index + 1 < arguments.size()) {
            options.optimize = arguments[++index];
        } else if (argument == "-g" || argument == "--debug") {
            options.debug = true;
        } else if (argument == "--protocol") {
            options.protocol = true;
        } else if ((argument == "-I" || argument == "--include") && index + 1 < arguments.size()) {
            options.includes.push_back(arguments[++index]);
        } else if ((argument == "-D" || argument == "--define") && index + 1 < arguments.size()) {
            options.defines.push_back(arguments[++index]);
        } else if (argument == "-h" || argument == "--help") {
            printUsage();
            return false;
        } else if (argument == "-v" || argument == "--version") {
            std::cout << "mdcompiler 1.0.0\n";
            return false;
        } else if (startsWith(argument, "-O") && argument.size() == 2) {
            if (index + 1 < arguments.size()) {
                options.optimize = arguments[++index];
            }
        } else {
            std::cerr << "Bilinmeyen veya eksik secenek: " << argument << '\n';
            return false;
        }
    }
    return true;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::vector<std::string> arguments(argv + 1, argv + argc);
    std::string command = arguments[0];

    if (command == "-h" || command == "--help") {
        printUsage();
        return 0;
    }
    if (command == "-v" || command == "--version") {
        std::cout << "mdcompiler 1.0.0\n";
        return 0;
    }
    if (command != "build" && command != "compile" && command != "run") {
        std::cerr << "Bilinmeyen komut: " << command << '\n';
        printUsage();
        return 1;
    }
    if (arguments.size() < 2) {
        std::cerr << "Bir kaynak dosya belirtilmelidir.\n";
        printUsage();
        return 1;
    }

    fs::path source(arguments[1]);
    if (!fs::exists(source)) {
        return fail("Kaynak dosya bulunamadi: " + source.string());
    }
    if (!fs::is_regular_file(source)) {
        return fail("Kaynak yolu bir dosya degil: " + source.string());
    }

    std::string extension = source.extension().string();
    for (char& ch : extension) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    if (extension != ".c" && extension != ".cc" && extension != ".cpp" &&
        extension != ".cxx" && extension != ".c++") {
        return fail("Desteklenen uzantilar: .c, .cc, .cpp, .cxx, .c++");
    }

    Options options;
    std::vector<std::string> runArguments;
    size_t optionIndex = 2;
    if (!parseOptions(arguments, optionIndex, options, runArguments)) {
        return 1;
    }

    mdcompiler::Language language = mdcompiler::languageFromSource(source);
    options.compiler = mdcompiler::resolveCompiler(language, options.compiler);
    if (options.compiler.empty()) {
        return fail(language == mdcompiler::Language::C
            ? "C derleyicisi bulunamadi (clang, gcc veya cl)."
            : "C++ derleyicisi bulunamadi (clang++, g++ veya cl).");
    }

    if (options.standard.empty()) {
        options.standard = language == mdcompiler::Language::C ? "c17" : "c++17";
    }

    fs::path output = executableName(source, options.output);
    mdcompiler::CompileOptions compileOptions;
    compileOptions.standard = options.standard;
    compileOptions.optimize = options.optimize;
    compileOptions.debug = options.debug;
    compileOptions.includes = options.includes;
    compileOptions.defines = options.defines;
    std::string compilerCommand = mdcompiler::buildCommand(
        options.compiler, language, source, output, compileOptions
    );

    if (options.protocol) {
        std::cout << "MDCOMPILER/1 status=ok phase=compile language="
                  << (language == mdcompiler::Language::C ? "c" : "cpp")
                  << " compiler=\"" << options.compiler << "\"\n";
    }
    std::cout << "Derleniyor: " << source << '\n';
    std::cout << "Derleyici: " << options.compiler << '\n';
    int compileResult = std::system(compilerCommand.c_str());
    if (compileResult != 0) {
        if (options.protocol) {
            std::cout << "MDCOMPILER/1 status=error phase=compile exit_code="
                      << compileResult << "\n";
        }
        return fail("Derleme basarisiz oldu (cikis kodu " +
            std::to_string(compileResult) + ").");
    }

    if (options.protocol) {
        std::cout << "MDCOMPILER/1 status=ok phase=compiled output=executable exit_code=0\n";
    }
    std::cout << "Cikti: " << output << '\n';
    if (command == "run") {
        std::ostringstream runCommand;
        runCommand << quote(output.string());
        for (const std::string& argument : runArguments) {
            runCommand << " " << quote(argument);
        }
        int runResult = std::system(runCommand.str().c_str());
        if (runResult != 0) {
            if (options.protocol) {
                std::cout << "MDCOMPILER/1 status=error phase=run exit_code="
                          << runResult << "\n";
            }
            return fail("Program basarisiz oldu (cikis kodu " +
                std::to_string(runResult) + ").");
        }
        if (options.protocol) {
            std::cout << "MDCOMPILER/1 status=ok phase=run exit_code=0\n";
        }
    }
    return 0;
}
