// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace reluva::stafu {

// Evidence-based identification, not a guarantee of a complete playable install.
enum class DetectionStatus { Recognized, Incomplete, Unknown };
enum class DetectedSourceKind { InstalledGame, CdContents };

struct SourceDetection {
    DetectedSourceKind kind = DetectedSourceKind::InstalledGame;
    DetectionStatus status = DetectionStatus::Unknown;
    std::vector<std::string> found;
    std::vector<std::string> missing;
    std::vector<std::string> errors;
};

// Checks a small set of file names independently observed in the original
// installed-game and CD inventories. Case-insensitive and read-only.
// No file hashes or CD archive parsing required. Source may be incomplete.
SourceDetection detectSource(const std::filesystem::path& root, DetectedSourceKind kind);
const char* statusName(DetectionStatus status);

} // namespace reluva::stafu
