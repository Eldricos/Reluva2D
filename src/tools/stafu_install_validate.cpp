// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only standalone STAFU installation validator.
#include "engines/stafu/source_detection.h"
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/zip_archive.h"
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

namespace {
void printDetection(const char* label, const reluva::stafu::SourceDetection& result) {
    std::printf("%s: %s\n", label, reluva::stafu::statusName(result.status));
    for (const auto& name : result.missing) std::printf("  MISSING %s\n", name.c_str());
    for (const auto& error : result.errors) std::printf("  ERROR %s\n", error.c_str());
}
}

int stafuInstallValidate(const char* installedArg, const char* cdArg) {
    namespace fs = std::filesystem;
    const fs::path installed(installedArg);
    fs::path cd;
    if (cdArg) cd = fs::path(cdArg);
    else cd = installed / "cd";
    std::printf("STAFU installation validator (read-only)\nInstalled: %s\nCD: %s\n",
                installed.string().c_str(), cd.string().c_str());
    bool ok = true;
    const auto game = reluva::stafu::detectSource(
        installed, reluva::stafu::DetectedSourceKind::InstalledGame);
    printDetection("Installed files", game);
    if (game.status != reluva::stafu::DetectionStatus::Recognized) ok = false;
    const auto disc = reluva::stafu::detectSource(
        cd, reluva::stafu::DetectedSourceKind::CdContents);
    printDetection("CD files", disc);
    if (disc.status != reluva::stafu::DetectionStatus::Recognized) ok = false;

    for (const char* name : {"STTNG.ZIP", "STTNGINS.ZIP"}) {
        reluva::stafu::ZipArchive zip;
        std::string error;
        if (!zip.open(cd / name, error)) {
            std::printf("FAIL archive %s: %s\n", name, error.c_str());
            ok = false;
        } else {
            std::printf("PASS archive %s: %zu entries indexed\n", name, zip.size());
        }
    }

    // The engine's current startup uses the same resource manager and two DB reads.
    // Do not claim full resource integrity or gameplay readiness.
    if (ok) {
        reluva::stafu::ResourceManager resources;
        std::string error;
        if (!resources.open(installed, cd, error)) {
            std::printf("FAIL resource manager: %s\n", error.c_str());
            ok = false;
        } else {
            for (const char* name : {"astro.db", "computer.db"}) {
                std::vector<uint8_t> bytes;
                error.clear();
                if (!resources.read(name, bytes, error) || bytes.empty()) {
                    std::printf("FAIL resource %s: %s\n", name, error.c_str());
                    ok = false;
                } else {
                    std::printf("PASS resource %s: %zu bytes\n", name, bytes.size());
                }
            }
        }
    }
    if (!ok) {
        std::puts("FAIL: STAFU resource setup is incomplete or invalid.");
        return 1;
    }
    std::puts("PASS: STAFU startup resource setup validated.");
    std::puts("This does not verify every asset, original installer modes, or gameplay readiness.");
    return 0;
}
