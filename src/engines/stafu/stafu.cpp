
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#include "engines/engine.h"

#include <memory>
#include <utility>

namespace reluva {

    namespace {

        class StafuEngine : public Engine {
        public:
            StafuEngine(System& system, GameTarget target)
                : system_(system), target_(std::move(target)) {
            }

            Error run() override {
                // Initial registration milestone only.
                // Game initialization will be implemented in a later step.
                return Error::fail(
                    "STAFU is registered, but game initialization is not implemented yet.");
            }

        private:
            System& system_;
            GameTarget target_;
        };

        class StafuMetaEngine : public MetaEngine {
        public:
            const char* id() const override {
                return "stafu";
            }

            const char* name() const override {
                return "Star Trek: A Final Unity (STAFU)";
            }

            std::vector<DetectedGame> detect(
                const std::filesystem::path& dir) const override {
                // Detection will be added after we verify the original
                // game's installation files and identify reliable markers.
                (void)dir;
                return {};
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