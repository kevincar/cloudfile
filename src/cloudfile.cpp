#include "cloudfile.hpp"

#include <filesystem>
#include <iostream>
#include <system_error>

namespace {

bool g_verbose = false;
bool g_force = false;

const char *toString(CloudFileStatus statusValue) {
    if (statusValue == CloudFileStatus::Evicted) {
        return "evicted";
    }

    return "materialized";
}

std::string displayName(const std::filesystem::path &path) {
    const std::filesystem::path filename = path.filename();
    if (!filename.empty()) {
        return filename.string();
    }

    return path.string();
}

}  // namespace

void set_verbose(bool verbose) {
    g_verbose = verbose;
}

bool is_verbose() {
    return g_verbose;
}

void set_force(bool force) {
    g_force = force;
}

bool is_force() {
    return g_force;
}

extern "C" int cloudfile_is_verbose(void) {
    return g_verbose ? 1 : 0;
}

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
    const bool destinationExists = std::filesystem::exists(destination, error);
    if (error) {
        std::cerr << "Error checking destination '" << destination.string()
                  << "': " << error.message() << '\n';
        return 1;
    }

    if (destinationExists) {
        const bool destinationIsDirectory = std::filesystem::is_directory(destination, error);
        if (error) {
            std::cerr << "Error checking destination '" << destination.string()
                      << "': " << error.message() << '\n';
            return 1;
        }

        if (destinationIsDirectory) {
            resolvedDestination /= source.filename();
        }
    }

    const bool shouldRestoreEvictedState = *originalStatus == CloudFileStatus::Evicted;
    if (shouldRestoreEvictedState && materialize(source) != 0) {
        return 1;
    }

    int exitCode = 0;
    error.clear();
    const auto copyOptions = is_force()
        ? std::filesystem::copy_options::overwrite_existing
        : std::filesystem::copy_options::none;
    const bool copied = std::filesystem::copy_file(source, resolvedDestination, copyOptions, error);
    if (copied) {
        if (is_verbose()) {
            std::cout << "Copy " << displayName(source) << '\n';
        }
    } else if (error) {
        std::cerr << "Error copying file from '" << source.string()
                  << "' to '" << resolvedDestination.string()
                  << "': " << error.message() << '\n';
        exitCode = 1;
    } else if (is_force()) {
        std::cerr << "Error copying file to '" << resolvedDestination.string()
                  << "': destination already exists\n";
        exitCode = 1;
    } else if (is_verbose()) {
        std::cout << "Skip " << displayName(source) << '\n';
    }

    if (shouldRestoreEvictedState && evict(source) != 0) {
        exitCode = 1;
    }

    return exitCode;
}

int copydir(
    const std::filesystem::path &source,
    const std::filesystem::path &destination,
    bool source_is_directory_contents,
    bool destination_is_directory) {
    std::error_code error;
    if (!std::filesystem::is_directory(source, error)) {
        if (error) {
            std::cerr << "Error checking source directory '" << source.string()
                      << "': " << error.message() << '\n';
        } else {
            std::cerr << "Error copying directory '" << source.string()
                      << "': source is not a directory\n";
        }
        return 1;
    }

    std::filesystem::path destinationRoot = destination;
    if (!source_is_directory_contents && destination_is_directory) {
        destinationRoot /= source.filename();
    }

    if (!std::filesystem::create_directories(destinationRoot, error) && error) {
        std::cerr << "Error creating destination directory '" << destinationRoot.string()
                  << "': " << error.message() << '\n';
        return 1;
    }

    for (std::filesystem::recursive_directory_iterator it(source, error), end; it != end; it.increment(error)) {
        if (error) {
            std::cerr << "Error walking source directory '" << source.string()
                      << "': " << error.message() << '\n';
            return 1;
        }

        const std::filesystem::path relativePath = std::filesystem::relative(it->path(), source, error);
        if (error) {
            std::cerr << "Error computing relative path for '" << it->path().string()
                      << "' against '" << source.string()
                      << "': " << error.message() << '\n';
            return 1;
        }

        const std::filesystem::path destinationPath = destinationRoot / relativePath;
        if (it->is_directory(error)) {
            if (error) {
                std::cerr << "Error checking directory entry '" << it->path().string()
                          << "': " << error.message() << '\n';
                return 1;
            }

            if (!std::filesystem::create_directories(destinationPath, error) && error) {
                std::cerr << "Error creating destination directory '" << destinationPath.string()
                          << "': " << error.message() << '\n';
                return 1;
            }
            continue;
        }

        if (it->is_regular_file(error)) {
            if (error) {
                std::cerr << "Error checking file entry '" << it->path().string()
                          << "': " << error.message() << '\n';
                return 1;
            }

            if (copyfile(it->path(), destinationPath) != 0) {
                return 1;
            }
            continue;
        }

        if (error) {
            std::cerr << "Error checking directory entry '" << it->path().string()
                      << "': " << error.message() << '\n';
            return 1;
        }

        std::cerr << "Error copying directory: unsupported entry type '" << it->path().string() << "'\n";
        return 1;
    }

    return 0;
}
