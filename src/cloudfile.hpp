#pragma once

#include <filesystem>
#include <optional>

enum class CloudFileStatus {
    Evicted,
    Materialized,
};

int materialize(const std::filesystem::path &path);
int evict(const std::filesystem::path &path);
std::optional<CloudFileStatus> get_status(const std::filesystem::path &path);
int status(const std::filesystem::path &path);
int copyfile(const std::filesystem::path &source, const std::filesystem::path &destination);
