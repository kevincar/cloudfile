#include "cloudfile.hpp"

#include <filesystem>
#include <iostream>
#include <system_error>

namespace {

const char *toString(CloudFileStatus statusValue) {
    if (statusValue == CloudFileStatus::Evicted) {
        return "evicted";
    }

    return "materialized";
}

}  // namespace

int status(const std::filesystem::path &path) {
    const std::optional<CloudFileStatus> fileStatus = get_status(path);
    if (!fileStatus.has_value()) {
        return 1;
    }

    std::cout << toString(*fileStatus) << '\n';
    return 0;
}

int copyfile(const std::filesystem::path &source, const std::filesystem::path &destination) {
    const std::optional<CloudFileStatus> originalStatus = get_status(source);
    if (!originalStatus.has_value()) {
        return 1;
    }

    std::error_code error;
    std::filesystem::path resolvedDestination = destination;
    if (std::filesystem::is_directory(destination, error)) {
        resolvedDestination /= source.filename();
    } else if (error) {
        std::cerr << "Error checking destination: " << error.message() << '\n';
        return 1;
    }

    const bool shouldRestoreEvictedState = *originalStatus == CloudFileStatus::Evicted;
    if (shouldRestoreEvictedState && materialize(source) != 0) {
        return 1;
    }

    int exitCode = 0;
    error.clear();
    if (!std::filesystem::copy_file(source, resolvedDestination, error)) {
        if (error) {
            std::cerr << "Error copying file: " << error.message() << '\n';
        } else {
            std::cerr << "Error copying file: destination already exists\n";
        }
        exitCode = 1;
    }

    if (shouldRestoreEvictedState && evict(source) != 0) {
        exitCode = 1;
    }

    return exitCode;
}
