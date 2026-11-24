#include <cstdlib>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

bool executableExists(const std::string& name) {
    std::ostringstream command;
    if (MDCOMPILER_WIN32) {
        command << "where.exe " << quote(name) << " >nul 2>&1";
    } else {
        command << "command -v " << quote(name) << " >/dev/null 2>&1";
    }
    return std::system(command.str().c_str()) == 0;
}

enum class Language {
    C,
    Cpp
};

Language languageFrom(const fs::path& source) {
    std::string extension = source.extension().string();
    for (char& ch : extension) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    if (extension == ".c") {
        return Language::C;
    }
    return Language::Cpp;
}

std::string environmentCompiler(Language language) {
    if (language == Language::C) {
        if (const char* value = std::getenv("MDCOMPILER_CC")) {
            return value;
        }
    } else {
        if (const char* value = std::getenv("MDCOMPILER_CXX")) {
            return value;
        }
    }
    return "";
}

std::string defaultCompiler(Language language) {
    std::vector<std::string> candidates;
    if (language == Language::C) {
        candidates = {"clang", "gcc", "cl", "cc"};
    } else {
        candidates = {"clang++", "g++", "cl", "c++"};
    }

    for (const std::string& candidate : candidates) {
        if (executableExists(candidate)) {
            return candidate;
        }
    }
    return "";
}

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

bool isMicrosoftCompiler(const std::string& compiler) {
    std::string name = compiler;
    for (char& ch : name) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return name == "cl" || name == "cl.exe";
}

struct Options {
    fs::path output;
    std::string compiler;
    std::string standard;
    std::string optimize = "0";
    bool debug = false;
    std::vector<std::string> includes;
    std::vector<std::string> defines;
};

int fail(const std::string& message) {
    std::cerr << "Hata: " << message << '\n';
    return 1;
}

std::string buildCommand(
    const std::string& compiler,
    Language language,
    const fs::path& source,
    const fs::path& output,
    const Options& options
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
            if (startsWith(options.standard, "c++")) {
                command << " /std:" << options.standard;
            } else if (language == Language::Cpp) {
                command << " /std:c++" << (options.standard == "c++20" ? "20" : "17");
            } else if (options.standard == "c11") {
                command << " /std:c11";
            } else if (options.standard == "c17") {
                command << " /std:c17";
            }
        }
        if (!options.optimize.empty()) {
            if (options.optimize == "0") {
                command << " /Od";
            } else {
                command << " /O" << options.optimize;
            }
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
        } else if ((argument == "-c" || argument == "--compiler") && index + 1 < arguments.size()) {
            options.compiler = arguments[++index];
        } else if ((argument == "-s" || argument == "--std") && index + 1 < arguments.size()) {
            options.standard = arguments[++index];
        } else if ((argument == "-O" || argument == "--optimize") && index + 1 < arguments.size()) {
            options.optimize = arguments[++index];
        } else if (argument == "-g" || argument == "--debug") {
            options.debug = true;
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

    Language language = languageFrom(source);
    if (options.compiler.empty()) {
        options.compiler = environmentCompiler(language);
    }
    if (options.compiler.empty()) {
        options.compiler = defaultCompiler(language);
    }
    if (options.compiler.empty()) {
        return fail(language == Language::C
            ? "C derleyicisi bulunamadi (clang, gcc veya cl)."
            : "C++ derleyicisi bulunamadi (clang++, g++ veya cl).");
    }

    if (options.standard.empty()) {
        options.standard = language == Language::C ? "c17" : "c++17";
    }

    fs::path output = executableName(source, options.output);
    std::string compilerCommand =
        buildCommand(options.compiler, language, source, output, options);

    std::cout << "Derleniyor: " << source << '\n';
    std::cout << "Derleyici: " << options.compiler << '\n';
    int compileResult = std::system(compilerCommand.c_str());
    if (compileResult != 0) {
        return fail("Derleme basarisiz oldu (cikis kodu " +
            std::to_string(compileResult) + ").");
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
            return fail("Program basarisiz oldu (cikis kodu " +
                std::to_string(runResult) + ").");
        }
    }
    return 0;
}
