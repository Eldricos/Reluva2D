// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#include "engines/engine.h"
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/source_detection.h"

#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace reluva {
namespace {

class StafuEngine : public Engine {
public:
    StafuEngine(System& system, GameTarget target)
        : system_(system), target_(std::move(target)) {}

    Error run() override {
        // Recheck source data at startup: a previously detected folder may
        // have been moved or changed after being added to the launcher.
        const auto detection = stafu::detectSource(
            target_.path, stafu::DetectedSourceKind::InstalledGame);
        if (detection.status != stafu::DetectionStatus::Recognized)
            return Error::fail("STAFU installed game files are missing or incomplete at: " +
                               target_.path.string());

        // A managed install can keep CD contents under <install>/cd.
        // Do not guess sibling directory names or mutate original game data.
        std::filesystem::path cdRoot;
        std::error_code ec;
        const auto candidate = target_.path / "cd";
        if (std::filesystem::is_directory(candidate, ec))
            cdRoot = candidate;

        stafu::ResourceManager resources;
        std::string error;
        if (!resources.open(target_.path, cdRoot, error))
            return Error::fail("STAFU resource initialization failed: " + error);

        // Verify actual resource reads, not just filenames and sizes.
        // Both resources are known to exist in the verified installed source.
        for (const char* name : {"astro.db", "computer.db"}) {
            std::vector<uint8_t> bytes;
            if (!resources.read(name, bytes, error) || bytes.empty())
                return Error::fail(std::string("STAFU cannot load ") + name + ": " +
                                   (error.empty() ? "empty resource" : error));
        }

        return Error::fail(
            "STAFU installation recognized and initial resources loaded successfully; "
            "gameplay initialization is not implemented yet.");
    }

private:
    System& system_;
    GameTarget target_;
};

class StafuMetaEngine : public MetaEngine {
public:
    const char* id() const override { return "stafu"; }
    const char* name() const override {
        return "Star Trek: A Final Unity (STAFU)";
    }

    std::vector<DetectedGame> detect(const std::filesystem::path& dir) const override {
        const auto source = stafu::detectSource(
            dir, stafu::DetectedSourceKind::InstalledGame);
        if (source.status != stafu::DetectionStatus::Recognized)
            return {};
        // These markers are verified for one original DOS installation.
        // They do not yet establish a specific language or release variant.
        return {DetectedGame{"stafu", "stafu", "Star Trek: The Next Generation - A Final Unity",
                             "unknown", "dos"}};
    }

    std::unique_ptr<Engine> create(
        System& system, const GameTarget& target) const override {
        return std::make_unique<StafuEngine>(system, target);
    }
};
} // namespace

const MetaEngine& stafuMetaEngine() {
    static const StafuMetaEngine instance;
    return instance;
}
} // namespace reluva
