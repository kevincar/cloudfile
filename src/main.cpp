#include "cloudfile.hpp"

#include <filesystem>
#include <iostream>
#include <string_view>

namespace {

void printUsage() {
    std::cout << "Usage: cloudfile [-v|--verbose] [-f|--force] <command> <file-path>\n"
              << "       cloudfile [-v|--verbose] [-f|--force] copyfile <source-path> <destination-path>\n"
              << "       cloudfile [-v|--verbose] [-f|--force] copydir <source-dir> <destination-path>\n"
              << "Commands:\n"
              << "  materialize - Download the file from the cloud\n"
              << "  evict - Remove local copy while keeping it in the cloud\n"
              << "  status - Print whether the file is evicted or materialized\n"
              << "  copyfile - Copy a file while preserving its cloud state\n"
              << "  copydir - Copy a directory tree file-by-file while preserving cloud state\n";
}

bool endsWithDirectorySeparator(std::string_view path) {
    return !path.empty() && (path.back() == '/' || path.back() == '\\');
}

}  // namespace

int main(int argc, const char *argv[]) {
    int argumentIndex = 1;
    while (argumentIndex < argc) {
        const std::string_view option{argv[argumentIndex]};
        if (option == "-v" || option == "--verbose") {
            set_verbose(true);
            ++argumentIndex;
            continue;
        }

        if (option == "-f" || option == "--force") {
            set_force(true);
            ++argumentIndex;
            continue;
        }

        break;
    }

    const int remainingArguments = argc - argumentIndex;
    if (remainingArguments < 2 || remainingArguments > 3) {
        printUsage();
        return 1;
    }

    const std::string_view command{argv[argumentIndex]};

    if (command == "copyfile") {
        if (remainingArguments != 3) {
            printUsage();
            return 1;
        }

        const std::filesystem::path sourcePath{argv[argumentIndex + 1]};
        const std::filesystem::path destinationPath{argv[argumentIndex + 2]};
        return copyfile(sourcePath, destinationPath);
    }

    if (command == "copydir") {
        if (remainingArguments != 3) {
            printUsage();
            return 1;
        }

        const std::filesystem::path sourcePath{argv[argumentIndex + 1]};
        const std::filesystem::path destinationPath{argv[argumentIndex + 2]};
        return copydir(
            sourcePath,
            destinationPath,
            endsWithDirectorySeparator(argv[argumentIndex + 2]));
    }

    if (remainingArguments != 2) {
        printUsage();
        return 1;
    }

    const std::filesystem::path filePath{argv[argumentIndex + 1]};

    if (command == "materialize") {
        return materialize(filePath);
    }

    if (command == "evict") {
        return evict(filePath);
    }

    if (command == "status") {
        return ::status(filePath);
    }

    printUsage();
    return 1;
}
