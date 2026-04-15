#include "cloudfile.hpp"

#include <optional>
#include <string>

extern "C" {
int materialize(const char *path);
int evict(const char *path);
int get_cloudfile_status(const char *path, int *status_code);
}

int materialize(const std::filesystem::path &path) {
    const std::string nativePath = path.string();
    return materialize(nativePath.c_str());
}

int evict(const std::filesystem::path &path) {
    const std::string nativePath = path.string();
    return evict(nativePath.c_str());
}

std::optional<CloudFileStatus> get_status(const std::filesystem::path &path) {
    const std::string nativePath = path.string();
    int statusCode = -1;
    if (get_cloudfile_status(nativePath.c_str(), &statusCode) != 0) {
        return std::nullopt;
    }

    if (statusCode == 0) {
        return CloudFileStatus::Evicted;
    }

    return CloudFileStatus::Materialized;
}
