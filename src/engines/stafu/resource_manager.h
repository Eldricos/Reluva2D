// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "engines/stafu/zip_archive.h"
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace reluva::stafu {
enum class ResourceOrigin { InstalledFile, MainArchive, InstallerArchive };
struct ResourceLocation {
    ResourceOrigin origin;
    std::string name;
};
// Source roots are explicit and may differ from managed installation paths.
// Priority: installed loose files, main ZIP, then installer ZIP.
// Only flat resource names are accepted to prevent directory traversal.
class ResourceManager {
public:
    bool open(const std::filesystem::path& installedRoot,
              const std::filesystem::path& cdRoot, std::string& error);
    bool locate(const std::string& name, ResourceLocation& location) const;
    bool read(const std::string& name, std::vector<uint8_t>& bytes, std::string& error,
              uint64_t maxBytes = 64ull * 1024 * 1024) const;
private:
    std::filesystem::path installedRoot_;
    ZipArchive main_, installer_;
    bool mainReady_ = false, installerReady_ = false;
};
} // namespace reluva::stafu
