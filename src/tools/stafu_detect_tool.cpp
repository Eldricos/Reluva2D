// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#include "engines/stafu/source_detection.h"

#include <cstdio>
#include <cstring>
#include <filesystem>

int main(int argc, char** argv) {
    if (argc != 3 || (std::strcmp(argv[1], "installed") != 0 &&
                      std::strcmp(argv[1], "cd") != 0)) {
        std::fprintf(stderr, "usage: reluva-stafu-detect <installed|cd> <directory>\n");
        return 2;
    }
    const auto kind = std::strcmp(argv[1], "cd") == 0
        ? reluva::stafu::DetectedSourceKind::CdContents
        : reluva::stafu::DetectedSourceKind::InstalledGame;
    const auto result = reluva::stafu::detectSource(std::filesystem::path(argv[2]), kind);
    std::printf("Status: %s (candidate identification only)\n",
                reluva::stafu::statusName(result.status));
    for (const auto& name : result.found)
        std::printf("FOUND   %s\n", name.c_str());
    for (const auto& name : result.missing)
        std::printf("MISSING %s\n", name.c_str());
    for (const auto& error : result.errors)
        std::fprintf(stderr, "ERROR   %s\n", error.c_str());
    return result.status == reluva::stafu::DetectionStatus::Recognized ? 0 : 1;
}
