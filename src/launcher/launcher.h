// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#pragma once

#include "common/config.h"
#include "common/detection.h"
#include "engines/engine.h"
#include "engines/stafu/computer_browser.h"
#include "engines/stafu/computer_string_index.h"

#include <atomic>
#include <mutex>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <filesystem>
#include <vector>

namespace reluva {

class SdlSystem;

// The game list UI, drawn with Dear ImGui on the SDL renderer.
class Launcher {
public:
    Launcher(SdlSystem& system, Config& config);
    ~Launcher();
    Launcher(const Launcher&) = delete;
    Launcher& operator=(const Launcher&) = delete;

    // Show the launcher until the user starts a game (returned) or quits.
    std::optional<GameTarget> run();

    // Shown in the status bar next time the launcher appears.
    void setStatus(std::string message) { status_ = std::move(message); }

private:
    void drawUi(std::optional<GameTarget>& launch, bool& quit);
    void drawComputerUi();
    void openComputer(const GameTarget& target);
    void handlePickedFolder();
    void inspectInstallSource(const std::string& folder);
    void startStafuInstall(const std::string& cdFolder);
    std::string existingStafuInstallation() const;
    void finishStafuInstall();
    void addGame(const DetectedGame& game, const std::string& path);
    GameTarget targetFromConfig(const std::string& name) const;

    static void pickFolderCallback(void* userdata, const char* const* files, int filter);

    SdlSystem& sys_;
    Config& config_;
    std::string selected_;
    stafu::ComputerNavigationIndex computerGraph_;
    stafu::ComputerStringIndex computerStrings_;
    std::unique_ptr<stafu::ComputerBrowser> computerBrowser_;
    bool computerOpen_ = false;
    std::string pendingCdTarget_;
    bool pendingInstallSource_ = false;
    bool showInstallConfirmation_ = false;
    std::string confirmedInstallSource_;
    std::string installBlockedReason_;
    std::thread installThread_;
    std::atomic<bool> installRunning_{false};
    std::atomic<bool> installFinished_{false};
    std::atomic<int> installResult_{1};
    std::filesystem::path installDestination_;
    std::string status_;

    // The folder dialog answers on another thread.
    std::mutex pickMutex_;
    std::optional<std::string> pickedFolder_;
    std::optional<std::string> pickError_;
    std::atomic<bool> pickInProgress_{false};

    // Folder with several matches, waiting for the user to choose one.
    std::string pendingPath_;
    std::vector<DetectedGame> pendingChoices_;
};

} // namespace reluva
