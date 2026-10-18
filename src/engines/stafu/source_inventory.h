// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace reluva::stafu {

// Keep installed-game and CD inventories separate even when their relative
// paths happen to be identical.
enum class SourceKind {
    InstalledGame,
    CdContents,
};

struct InventoryEntry {
    // Path relative to the selected source root, using '/' separators.
    std::string relativePath;
    uintmax_t size = 0;
    // Full-file MD5, used for inventory comparison and copy verification.
    // This is an integrity check, not a security/authenticity mechanism.
    std::string md5;
};

struct SourceInventory {
    std::filesystem::path root;
    SourceKind kind = SourceKind::InstalledGame;
    std::vector<InventoryEntry> files;
    // Any scan, metadata, or read errors make the inventory incomplete.
    std::vector<std::string> errors;

    bool complete() const { return errors.empty(); }
};

// Recursively inventory regular files beneath root. Does not follow symbolic
// links. The source tree is read-only from this function's perspective.
SourceInventory inventorySource(const std::filesystem::path& root, SourceKind kind);

} // namespace reluva::stafu
