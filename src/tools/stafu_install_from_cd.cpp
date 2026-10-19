// SPDX-License-Identifier: GPL-3.0-or-later
// Developer-only CD-to-managed-install workflow. No original game data embedded.
#include "stafu_reference_install_recipe.h"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

int stafuInstallExtract(const char* manifest, const char* cd, const char* destination);
int stafuInstallStage(const char* installed, const char* cd, const char* destination);

namespace {
namespace fs = std::filesystem;
bool within(const fs::path& child, const fs::path& parent) {
    auto a = child.begin(), b = parent.begin();
    for (; b != parent.end(); ++b, ++a)
        if (a == child.end() || *a != *b) return false;
    return true;
}
}

int stafuInstallFromCd(const char* cdArg, const char* destinationArg) {
    const fs::path cd(cdArg), destination(destinationArg);
    const fs::path extracted(destination.string() + ".stafu-extracted");
    const fs::path staging(destination.string() + ".stafu-staging");
    const fs::path recipe(destination.string() + ".stafu-recipe.csv");
    std::error_code ec;
    if (!fs::is_directory(cd, ec) || ec) {
        std::fprintf(stderr, "install-from-cd: CD source directory not found\n");
        return 1;
    }
    const fs::path parent = destination.parent_path().empty()
        ? fs::path(".") : destination.parent_path();
    if (!fs::is_directory(parent, ec) || ec) {
        std::fprintf(stderr, "install-from-cd: destination parent must exist\n");
        return 1;
    }
    const auto cdCanonical = fs::weakly_canonical(cd, ec);
    if (ec) return 1;
    const auto destCanonical = fs::weakly_canonical(destination, ec);
    if (ec || within(destCanonical, cdCanonical) || within(cdCanonical, destCanonical)) {
        std::fprintf(stderr, "install-from-cd: CD and destination must be separate\n");
        return 1;
    }
    // Every working path must be new. Never delete a pre-existing path.
    for (const auto& path : {destination, extracted, staging, recipe}) {
        ec.clear();
        if (fs::exists(path, ec) || ec) {
            std::fprintf(stderr, "install-from-cd: path already exists: %s\n",
                         path.string().c_str());
            return 1;
        }
    }
    {
        std::ofstream file(recipe, std::ios::binary);
        if (!file) {
            std::fprintf(stderr, "install-from-cd: cannot write temporary recipe\n");
            return 1;
        }
        file << reluva::stafu::kReferenceInstallManifest;
        file.close();
        if (!file) {
            std::fprintf(stderr, "install-from-cd: temporary recipe write failed\n");
            return 1;
        }
    }
    std::puts("install-from-cd: extracting verified reference files...");
    const int extraction = stafuInstallExtract(
        recipe.string().c_str(), cd.string().c_str(), extracted.string().c_str());
    if (extraction != 0) {
        std::fprintf(stderr, "install-from-cd: extraction failed; inspect any partial working files.\n");
        return 1;
    }
    // Remove only our generated recipe after successful extraction.
    fs::remove(recipe, ec);
    if (ec) {
        std::fprintf(stderr, "install-from-cd: cannot remove temporary recipe\n");
        return 1;
    }
    std::puts("install-from-cd: copying CD resources and validating staged installation...");
    if (stafuInstallStage(extracted.string().c_str(), cd.string().c_str(),
                          destination.string().c_str()) != 0) {
        std::fprintf(stderr, "install-from-cd: staging failed; working directories retained.\n");
        return 1;
    }
    // The final installation is complete. Only remove our temporary extracted copy.
    fs::remove_all(extracted, ec);
    if (ec) {
        std::fprintf(stderr, "install-from-cd: installation succeeded but temporary cleanup failed: %s\n",
                     ec.message().c_str());
        return 1;
    }
    std::printf("PASS: CD-only managed installation created at %s\n", destinationArg);
    std::puts("Recipe verified for one reference release; other editions are not yet supported.");
    return 0;
}
