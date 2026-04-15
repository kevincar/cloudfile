#include "cloudfile.hpp"

#include <filesystem>
#include <iostream>
#include <string_view>

namespace {

void printUsage() {
    std::cout << "Usage: cloudfile <command> <file-path>\n"
              << "       cloudfile copyfile <source-path> <destination-path>\n"
              << "Commands:\n"
              << "  materialize - Download the file from the cloud\n"
              << "  evict - Remove local copy while keeping it in the cloud\n"
              << "  status - Print whether the file is evicted or materialized\n"
              << "  copyfile - Copy a file while preserving its cloud state\n";
}

}  // namespace

int main(int argc, const char *argv[]) {
    if (argc < 3 || argc > 4) {
        printUsage();
        return 1;
    }

    const std::string_view command{argv[1]};

    if (command == "copyfile") {
        if (argc != 4) {
            printUsage();
            return 1;
        }

        const std::filesystem::path sourcePath{argv[2]};
        const std::filesystem::path destinationPath{argv[3]};
        return copyfile(sourcePath, destinationPath);
    }

    if (argc != 3) {
        printUsage();
        return 1;
    }

    const std::filesystem::path filePath{argv[2]};

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
