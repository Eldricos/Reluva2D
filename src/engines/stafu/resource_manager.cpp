// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <fstream>
#include <limits>

namespace reluva::stafu {
namespace {
bool safeName(const std::string& name) {
    return !name.empty() && name != "." && name != ".." &&
        name.find_first_of("/\\:") == std::string::npos &&
        name.find('\0') == std::string::npos;
}
std::filesystem::path loose(const std::filesystem::path& root, const std::string& name) {
    if (root.empty()) return {};
    std::error_code ec;
    // No directory traversal, and no symlinks outside the source root.
    for (const auto& entry : std::filesystem::directory_iterator(root, ec)) {
        if (ec) break;
        if (!entry.is_regular_file(ec) || entry.is_symlink(ec)) continue;
        const std::string filename = entry.path().filename().string();
        if (filename.size() != name.size()) continue;
        bool equal = true;
        for (size_t i = 0; i < name.size(); ++i) {
            const unsigned char a = static_cast<unsigned char>(filename[i]);
            const unsigned char b = static_cast<unsigned char>(name[i]);
            const auto lower = [](unsigned char c) { return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : int(c); };
            if (lower(a) != lower(b)) { equal = false; break; }
        }
        if (equal) return entry.path();
    }
    return {};
}
bool readLoose(const std::filesystem::path& path, std::vector<uint8_t>& bytes,
               std::string& error, uint64_t maxBytes) {
    std::error_code ec;
    const uint64_t size = std::filesystem::file_size(path, ec);
    if (ec) { error = ec.message(); return false; }
    if (size > maxBytes || size > uint64_t(std::numeric_limits<std::streamsize>::max())) {
        error = "loose resource exceeds memory limit"; return false;
    }
    std::ifstream f(path, std::ios::binary);
    if (!f) { error = "cannot open loose resource"; return false; }
    std::vector<uint8_t> data(size);
    if (size && !f.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size))) {
        error = "cannot read loose resource"; return false;
    }
    bytes = std::move(data);
    return true;
}
}
bool ResourceManager::open(const std::filesystem::path& installedRoot,
                           const std::filesystem::path& cdRoot, std::string& error) {
    installedRoot_.clear();
    main_ = ZipArchive{}; installer_ = ZipArchive{};
    mainReady_ = false; installerReady_ = false;
    std::error_code ec;
    if (!installedRoot.empty() && !std::filesystem::is_directory(installedRoot, ec)) {
        error = "installed source directory not found"; return false;
    }
    ec.clear();
    if (!cdRoot.empty() && !std::filesystem::is_directory(cdRoot, ec)) {
        error = "CD source directory not found"; return false;
    }
    if (installedRoot.empty() && cdRoot.empty()) {
        error = "at least one source directory is required"; return false;
    }
    // A missing archive is allowed when the corresponding source is partial.
    // A present but corrupt archive is not silently ignored.
    if (!cdRoot.empty()) {
        for (const auto& spec : {std::pair{"STTNG.ZIP", &main_}, {"STTNGINS.ZIP", &installer_}}) {
            const auto path = loose(cdRoot, spec.first);
            if (path.empty()) continue;
            if (!spec.second->open(path, error)) {
                error = std::string(spec.first) + ": " + error;
                return false;
            }
            if (spec.second == &main_) mainReady_ = true;
            else installerReady_ = true;
        }
    }
    installedRoot_ = installedRoot;
    error.clear();
    return true;
}
bool ResourceManager::locate(const std::string& name, ResourceLocation& location) const {
    if (!safeName(name)) return false;
    if (!loose(installedRoot_, name).empty()) {
        location = {ResourceOrigin::InstalledFile, name}; return true;
    }
    if (mainReady_ && main_.find(name)) {
        location = {ResourceOrigin::MainArchive, name}; return true;
    }
    if (installerReady_ && installer_.find(name)) {
        location = {ResourceOrigin::InstallerArchive, name}; return true;
    }
    return false;
}
bool ResourceManager::read(const std::string& name, std::vector<uint8_t>& bytes,
                           std::string& error, uint64_t maxBytes) const {
    bytes.clear();
    ResourceLocation location{};
    if (!locate(name, location)) { error = "resource not found or unsafe name"; return false; }
    if (location.origin == ResourceOrigin::InstalledFile)
        return readLoose(loose(installedRoot_, name), bytes, error, maxBytes);
    if (location.origin == ResourceOrigin::MainArchive)
        return main_.read(name, bytes, error, maxBytes);
    return installer_.read(name, bytes, error, maxBytes);
}
} // namespace reluva::stafu
