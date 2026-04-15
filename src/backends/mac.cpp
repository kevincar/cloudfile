#include "cloudfile.hpp"

#include <string>

extern "C" {
int materialize(const char *path);
int evict(const char *path);
int status(const char *path);
}

int materialize(const std::filesystem::path &path) {
    const std::string nativePath = path.string();
    return materialize(nativePath.c_str());
}

int evict(const std::filesystem::path &path) {
    const std::string nativePath = path.string();
    return evict(nativePath.c_str());
}

int status(const std::filesystem::path &path) {
    const std::string nativePath = path.string();
    return status(nativePath.c_str());
}
