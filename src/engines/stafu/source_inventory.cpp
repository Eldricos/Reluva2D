// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#include "engines/stafu/source_inventory.h"

#include "common/fs.h"

#include <algorithm>
#include <system_error>

namespace reluva::stafu {
namespace {

std::string pathForMessage(const std::filesystem::path& path) {
    return path.generic_string();
}

void addError(SourceInventory& inventory, const std::string& message) {
    inventory.errors.push_back(message);
}

} // namespace

SourceInventory inventorySource(const std::filesystem::path& root, SourceKind kind) {
    SourceInventory result;
    result.root = root;
    result.kind = kind;

    std::error_code ec;
    const auto status = std::filesystem::status(root, ec);
    if (ec) {
        addError(result, "Cannot inspect source root '" + pathForMessage(root) +
                         "': " + ec.message());
        return result;
    }
    if (!std::filesystem::exists(status) || !std::filesystem::is_directory(status)) {
        addError(result, "Source root is not a directory: '" + pathForMessage(root) + "'");
        return result;
    }

    std::filesystem::recursive_directory_iterator it(
        root, std::filesystem::directory_options::none, ec);
    const std::filesystem::recursive_directory_iterator end;
    if (ec) {
        addError(result, "Cannot scan source root '" + pathForMessage(root) +
                         "': " + ec.message());
        return result;
    }

    while (it != end) {
        const std::filesystem::path path = it->path();
        std::error_code entryEc;
        const auto symlinkStatus = it->symlink_status(entryEc);
        if (entryEc) {
            addError(result, "Cannot inspect '" + pathForMessage(path) +
                             "': " + entryEc.message());
        } else if (std::filesystem::is_symlink(symlinkStatus)) {
            // Do not follow links outside the selected source tree.
        } else if (std::filesystem::is_regular_file(symlinkStatus)) {
            const auto size = std::filesystem::file_size(path, entryEc);
            if (entryEc) {
                addError(result, "Cannot read size of '" + pathForMessage(path) +
                                 "': " + entryEc.message());
            } else {
                InventoryEntry entry;
                const auto relative = std::filesystem::relative(path, root, entryEc);
                if (entryEc) {
                    addError(result, "Cannot determine relative path for '" +
                                     pathForMessage(path) + "': " + entryEc.message());
                } else {
                    entry.relativePath = relative.generic_string();
                    entry.size = size;
                    const auto digest = fs::md5File(path, 0);
                    if (!digest) {
                        addError(result, "Cannot hash file '" + pathForMessage(path) + "'");
                    } else {
                        entry.md5 = *digest;
                    }
                    result.files.push_back(std::move(entry));
                }
            }
        }

        it.increment(ec);
        if (ec) {
            addError(result, "Error while scanning source tree: " + ec.message());
            ec.clear();
        }
    }

    std::sort(result.files.begin(), result.files.end(),
              [](const InventoryEntry& a, const InventoryEntry& b) {
                  return a.relativePath < b.relativePath;
              });
    return result;
}

} // namespace reluva::stafu
