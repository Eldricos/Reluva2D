// SPDX-License-Identifier: GPL-3.0-or-later
// Developer-only managed installation staging; never modifies source data.
#include "engines/stafu/source_detection.h"
#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>
#include <zlib.h>

int stafuInstallValidate(const char* installed, const char* cd);

namespace {
namespace fs = std::filesystem;
bool within(const fs::path& child, const fs::path& parent) {
    auto a = child.begin(), b = parent.begin();
    for (; b != parent.end(); ++b, ++a)
        if (a == child.end() || *a != *b) return false;
    return true;
}
bool digest(const fs::path& path, uint64_t& size, uint32_t& sum) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    std::array<unsigned char, 65536> buffer{};
    size = 0;
    uLong crc = crc32(0, Z_NULL, 0);
    while (file) {
        file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        const auto n = file.gcount();
        if (n > 0) {
            size += static_cast<uint64_t>(n);
            crc = crc32(crc, buffer.data(), static_cast<uInt>(n));
        }
    }
    sum = static_cast<uint32_t>(crc);
    return file.eof() && !file.bad();
}
bool copyTree(const fs::path& source, const fs::path& target, size_t& count, std::string& error) {
    std::error_code ec;
    for (fs::recursive_directory_iterator it(source, fs::directory_options::none, ec), end;
         it != end && !ec; it.increment(ec)) {
        const auto& item = *it;
        const auto status = item.symlink_status(ec);
        if (ec) break;
        if (fs::is_symlink(status) || (!fs::is_regular_file(status) && !fs::is_directory(status))) {
            error = "source contains a link or unsupported entry: " + item.path().string();
            return false;
        }
        const auto relative = item.path().lexically_relative(source);
        const auto output = target / relative;
        if (fs::is_directory(status)) {
            fs::create_directories(output, ec);
            if (ec) break;
            continue;
        }
        fs::create_directories(output.parent_path(), ec);
        if (ec) break;
        if (!fs::copy_file(item.path(), output, fs::copy_options::none, ec) || ec) break;
        uint64_t sourceSize = 0, outputSize = 0;
        uint32_t sourceCrc = 0, outputCrc = 0;
        if (!digest(item.path(), sourceSize, sourceCrc) ||
            !digest(output, outputSize, outputCrc) ||
            sourceSize != outputSize || sourceCrc != outputCrc) {
            error = "copied file failed CRC32 verification: " + item.path().string();
            return false;
        }
        ++count;
    }
    if (ec) {
        error = "copy failed: " + ec.message();
        return false;
    }
    return true;
}
}

int stafuInstallStage(const char* installedArg, const char* cdArg, const char* destinationArg) {
    const fs::path installed(installedArg), cd(cdArg), destination(destinationArg);
    std::error_code ec;
    if (!fs::is_directory(installed, ec) || !fs::is_directory(cd, ec)) {
        std::fprintf(stderr, "install-stage: both source directories must exist\n");
        return 1;
    }
    if (fs::exists(destination, ec) || ec) {
        std::fprintf(stderr, "install-stage: destination already exists\n");
        return 1;
    }
    const auto parent = destination.parent_path().empty() ? fs::path(".") : destination.parent_path();
    if (!fs::is_directory(parent, ec) || ec) {
        std::fprintf(stderr, "install-stage: destination parent must exist\n");
        return 1;
    }
    const auto sourceAbs = fs::weakly_canonical(installed, ec);
    if (ec) return 1;
    const auto cdAbs = fs::weakly_canonical(cd, ec);
    if (ec) return 1;
    const auto destAbs = fs::weakly_canonical(destination, ec);
    if (ec) return 1;
    if (within(destAbs, sourceAbs) || within(destAbs, cdAbs) ||
        within(sourceAbs, destAbs) || within(cdAbs, destAbs) ||
        within(sourceAbs, cdAbs) || within(cdAbs, sourceAbs)) {
        std::fprintf(stderr, "install-stage: source/destination directories must be separate\n");
        return 1;
    }
    if (fs::is_symlink(fs::symlink_status(installed, ec)) ||
        fs::is_symlink(fs::symlink_status(cd, ec))) {
        std::fprintf(stderr, "install-stage: source directory links are unsupported\n");
        return 1;
    }
    const auto game = reluva::stafu::detectSource(
        installed, reluva::stafu::DetectedSourceKind::InstalledGame);
    const auto disc = reluva::stafu::detectSource(
        cd, reluva::stafu::DetectedSourceKind::CdContents);
    if (game.status != reluva::stafu::DetectionStatus::Recognized ||
        disc.status != reluva::stafu::DetectionStatus::Recognized) {
        std::fprintf(stderr, "install-stage: source detection failed; run install-validate first\n");
        return 1;
    }
    // Fixed sibling staging path; never delete an existing directory, even after failure.
    const fs::path staging = destination.string() + ".stafu-staging";
    if (fs::exists(staging, ec) || ec || !fs::create_directory(staging, ec)) {
        std::fprintf(stderr, "install-stage: staging directory already exists or cannot be created\n");
        return 1;
    }
    size_t installedCount = 0, cdCount = 0;
    std::string error;
    if (!copyTree(installed, staging, installedCount, error) ||
        !fs::create_directory(staging / "cd", ec) || ec ||
        !copyTree(cd, staging / "cd", cdCount, error)) {
        std::fprintf(stderr, "install-stage: FAIL: %s\nStaging left at: %s\n",
                     error.empty() ? ec.message().c_str() : error.c_str(), staging.string().c_str());
        return 1;
    }
    std::puts("install-stage: validating staged installation...");
    if (stafuInstallValidate(staging.string().c_str(), nullptr) != 0) {
        std::fprintf(stderr, "install-stage: validation failed; staging retained: %s\n",
                     staging.string().c_str());
        return 1;
    }
    if (fs::exists(destination, ec) || ec) {
        std::fprintf(stderr, "install-stage: destination appeared during staging; refusing overwrite\n");
        return 1;
    }
    fs::rename(staging, destination, ec);
    if (ec) {
        std::fprintf(stderr, "install-stage: promotion failed: %s; staging retained\n", ec.message().c_str());
        return 1;
    }
    std::printf("PASS: managed installation staged and validated: %s\n", destination.string().c_str());
    std::printf("Copied and CRC32-checked: %zu installed files, %zu CD files.\n",
                installedCount, cdCount);
    std::puts("Launcher configuration unchanged. Gameplay is not yet implemented.");
    return 0;
}
