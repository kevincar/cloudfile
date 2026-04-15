#include "cloudfile.hpp"

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#define NOMINMAX
#include <Windows.h>
#include <cfapi.h>

namespace {

std::string formatWindowsError(DWORD error) {
    LPSTR buffer = nullptr;
    const DWORD flags =
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD size = FormatMessageA(
        flags,
        nullptr,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&buffer),
        0,
        nullptr);

    if (size == 0 || buffer == nullptr) {
        return "Unknown Windows error " + std::to_string(error);
    }

    std::string message(buffer, size);
    LocalFree(buffer);

    while (!message.empty() && (message.back() == '\r' || message.back() == '\n')) {
        message.pop_back();
    }

    return message;
}

std::string formatHresult(HRESULT hr) {
    return formatWindowsError(HRESULT_CODE(hr));
}

class Handle {
  public:
    explicit Handle(HANDLE handle) : handle_(handle) {}

    ~Handle() {
        if (handle_ != INVALID_HANDLE_VALUE) {
            CloseHandle(handle_);
        }
    }

    Handle(const Handle &) = delete;
    Handle &operator=(const Handle &) = delete;

    HANDLE get() const { return handle_; }
    bool valid() const { return handle_ != INVALID_HANDLE_VALUE; }

  private:
    HANDLE handle_;
};

Handle openPlaceholderFile(const std::filesystem::path &path, DWORD access, DWORD shareMode) {
    return Handle(CreateFileW(
        path.c_str(),
        access,
        shareMode,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr));
}

bool ensureRegularFile(const std::filesystem::path &path, const char *command) {
    std::error_code error;
    if (!std::filesystem::exists(path, error)) {
        std::cerr << "Unable to " << command << " '" << path.string() << "': file does not exist\n";
        return false;
    }

    if (std::filesystem::is_directory(path, error)) {
        std::cerr << "Unable to " << command
                  << " '" << path.string()
                  << "': Windows placeholder hydration/dehydration APIs are file-only\n";
        return false;
    }

    return true;
}

CF_PLACEHOLDER_STATE getPlaceholderState(HANDLE fileHandle) {
    FILE_ATTRIBUTE_TAG_INFO attributeTagInfo{};
    if (!GetFileInformationByHandleEx(
            fileHandle, FileAttributeTagInfo, &attributeTagInfo, sizeof(attributeTagInfo))) {
        return CF_PLACEHOLDER_STATE_INVALID;
    }

    return CfGetPlaceholderStateFromFileInfo(&attributeTagInfo, FileAttributeTagInfo);
}

}  // namespace

int materialize(const std::filesystem::path &path) {
    if (!ensureRegularFile(path, "materialize")) {
        return 1;
    }

    Handle handle = openPlaceholderFile(
        path,
        FILE_READ_DATA | FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
    if (!handle.valid()) {
        std::cerr << "Error opening file for materialization: "
                  << formatWindowsError(GetLastError()) << '\n';
        return 1;
    }

    LARGE_INTEGER offset{};
    LARGE_INTEGER length;
    length.QuadPart = CF_EOF;

    const HRESULT hr = CfHydratePlaceholder(
        handle.get(), offset, length, CF_HYDRATE_FLAG_NONE, nullptr);
    if (FAILED(hr)) {
        std::cerr << "Error materializing file: " << formatHresult(hr) << '\n';
        return 1;
    }

    std::cout << "Requested materialization of file: " << path.string() << '\n';
    return 0;
}

int evict(const std::filesystem::path &path) {
    if (!ensureRegularFile(path, "evict")) {
        return 1;
    }

    Handle handle = openPlaceholderFile(path, 0, 0);
    if (!handle.valid()) {
        std::cerr << "Error opening file for eviction: "
                  << formatWindowsError(GetLastError()) << '\n';
        return 1;
    }

    LARGE_INTEGER offset{};
    LARGE_INTEGER length;
    length.QuadPart = CF_EOF;

    const HRESULT hr = CfDehydratePlaceholder(
        handle.get(), offset, length, CF_DEHYDRATE_FLAG_NONE, nullptr);
    if (FAILED(hr)) {
        std::cerr << "Error evicting file: " << formatHresult(hr) << '\n';
        return 1;
    }

    std::cout << "Evicted file: " << path.string() << '\n';
    return 0;
}

std::optional<CloudFileStatus> get_status(const std::filesystem::path &path) {
    if (!ensureRegularFile(path, "get status for")) {
        return std::nullopt;
    }

    Handle handle = openPlaceholderFile(
        path,
        FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
    if (!handle.valid()) {
        std::cerr << "Error opening file for status: "
                  << formatWindowsError(GetLastError()) << '\n';
        return std::nullopt;
    }

    const CF_PLACEHOLDER_STATE state = getPlaceholderState(handle.get());
    if (state == CF_PLACEHOLDER_STATE_INVALID) {
        std::cerr << "Error checking file status: " << formatWindowsError(GetLastError()) << '\n';
        return std::nullopt;
    }

    if ((state & CF_PLACEHOLDER_STATE_PLACEHOLDER) == 0) {
        std::cerr << "File is not a cloud placeholder: " << path.string() << '\n';
        return std::nullopt;
    }

    if ((state & CF_PLACEHOLDER_STATE_PARTIALLY_ON_DISK) != 0) {
        return CloudFileStatus::Evicted;
    }

    return CloudFileStatus::Materialized;
}
