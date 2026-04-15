#include "cloudfile.hppl"

#include <filesystem>
#include <iostream>
#include <string_view>

namespace {

void printUsage() {
    std::cout << "Usage: cloudfile <command> <file-path>\n"
              << "Commands:\n"
              << "  materialize - Download the file from the cloud\n"
              << "  evict - Remove local copy while keeping it in the cloud\n";
}

}  // namespace

int main(int argc, const char *argv[]) {
    if (argc != 3) {
        printUsage();
        return 1;
    }

    const std::string_view command{argv[1]};
    const std::filesystem::path filePath{argv[2]};

    if (command == "materialize") {
        return materialize(filePath);
    }

    if (command == "evict") {
        return evict(filePath);
    }

    printUsage();
    return 1;
}
