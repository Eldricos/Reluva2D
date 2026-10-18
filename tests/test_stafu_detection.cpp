// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#include "engines/stafu/source_detection.h"

#include <cstdio>
#include <filesystem>
#include <fstream>

namespace {
int failures = 0;
#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); ++failures; } } while (0)

void write(const std::filesystem::path& root, const char* name) {
    std::filesystem::create_directories(root);
    std::ofstream(root / name, std::ios::binary) << "test";
}

void test(const std::filesystem::path& root) {
    namespace st = reluva::stafu;
    std::filesystem::remove_all(root);
    CHECK(st::detectSource(root, st::DetectedSourceKind::InstalledGame).status == st::DetectionStatus::Unknown);
    std::filesystem::create_directories(root);
    CHECK(st::detectSource(root, st::DetectedSourceKind::InstalledGame).status == st::DetectionStatus::Unknown);
    write(root, "sttng.exe");
    CHECK(st::detectSource(root, st::DetectedSourceKind::InstalledGame).status == st::DetectionStatus::Incomplete);
    write(root, "STTNG.OVL");
    write(root, "ASTRO.DB");
    write(root, "COMPUTER.DB");
    auto installed = st::detectSource(root, st::DetectedSourceKind::InstalledGame);
    CHECK(installed.status == st::DetectionStatus::Recognized);
    CHECK(installed.found.size() == 4);
    CHECK(installed.missing.empty());
    CHECK(st::detectSource(root, st::DetectedSourceKind::CdContents).status == st::DetectionStatus::Unknown);

    std::filesystem::remove_all(root);
    write(root, "STTNG.ZIP");
    write(root, "STTNG.DIR");
    write(root, "STTNGINS.ZIP");
    write(root, "TITLE.FVF");
    CHECK(st::detectSource(root, st::DetectedSourceKind::CdContents).status == st::DetectionStatus::Recognized);
    std::filesystem::remove_all(root);
}
} // namespace

int main() {
    test(std::filesystem::temp_directory_path() / "reluva_stafu_detection_tests");
    return failures ? 1 : 0;
}
