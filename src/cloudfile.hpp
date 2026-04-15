#pragma once

#include <filesystem>

int materialize(const std::filesystem::path &path);
int evict(const std::filesystem::path &path);
int status(const std::filesystem::path &path);
