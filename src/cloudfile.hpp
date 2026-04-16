#pragma once

#include <filesystem>
#include <optional>

enum class CloudFileStatus {
    Evicted,
    Materialized,
};

void set_verbose(bool verbose);
bool is_verbose();
void set_force(bool force);
bool is_force();

int materialize(const std::filesystem::path &path);
int evict(const std::filesystem::path &path);
std::optional<CloudFileStatus> get_status(const std::filesystem::path &path);
int status(const std::filesystem::path &path);
int copyfile(const std::filesystem::path &source, const std::filesystem::path &destination);
int copydir(
    const std::filesystem::path &source,
    const std::filesystem::path &destination,
    bool source_is_directory_contents,
    bool destination_is_directory);

extern "C" int cloudfile_is_verbose(void);
