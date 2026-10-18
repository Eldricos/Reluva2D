// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#include "engines/stafu/source_detection.h"

#include "common/fs.h"

#include <array>
#include <system_error>

namespace reluva::stafu {
namespace {

template <size_t N>
SourceDetection checkFiles(const std::filesystem::path& root, DetectedSourceKind kind,
                           const std::array<const char*, N>& names) {
    SourceDetection result;
    result.kind = kind;
    std::error_code ec;
    if (!std::filesystem::is_directory(root, ec)) {
        result.errors.push_back("Cannot inspect source directory: " + root.generic_string() +
                                (ec ? " (" + ec.message() + ")" : ""));
        return result;
    }

    for (const char* name : names) {
        const auto file = fs::findFile(root, name);
        if (!file) {
            result.missing.emplace_back(name);
            continue;
        }
        ec.clear();
        const auto size = std::filesystem::file_size(*file, ec);
        if (ec || size == 0) {
            result.errors.push_back(std::string("Cannot verify nonempty file: ") + name);
            result.missing.emplace_back(name);
        } else {
            result.found.emplace_back(name);
        }
    }

    if (result.found.size() == names.size() && result.errors.empty())
        result.status = DetectionStatus::Recognized;
    else if (!result.found.empty())
        result.status = DetectionStatus::Incomplete;
    return result;
}

} // namespace

SourceDetection detectSource(const std::filesystem::path& root, DetectedSourceKind kind) {
    // These names are supported by the user's inventories, not assumed to
    // identify every retail release. A recognized source is only a candidate.
    if (kind == DetectedSourceKind::InstalledGame) {
        return checkFiles(root, kind,
                          std::array{"STTNG.EXE", "STTNG.OVL", "ASTRO.DB", "COMPUTER.DB"});
    }
    return checkFiles(root, kind,
                      std::array{"STTNG.ZIP", "STTNG.DIR", "STTNGINS.ZIP", "TITLE.FVF"});
}

const char* statusName(DetectionStatus status) {
    switch (status) {
    case DetectionStatus::Recognized: return "recognized";
    case DetectionStatus::Incomplete: return "incomplete";
    case DetectionStatus::Unknown: return "unknown";
    }
    return "unknown";
}

} // namespace reluva::stafu
