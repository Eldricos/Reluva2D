// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

// Read-only STAFU source inventory. Usage:
//   reluva-stafu-inventory installed <source-directory>
//   reluva-stafu-inventory cd <source-directory>
// Redirect stdout to a TSV file if desired.

#include "engines/stafu/source_inventory.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>

namespace {

std::string escapeTsv(const std::string& input) {
    std::string output;
    for (char c : input) {
        switch (c) {
        case '\t': output += "\\t"; break;
        case '\n': output += "\\n"; break;
        case '\r': output += "\\r"; break;
        case '\\': output += "\\\\"; break;
        default: output += c; break;
        }
    }
    return output;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3 || (std::strcmp(argv[1], "installed") != 0 &&
                      std::strcmp(argv[1], "cd") != 0)) {
        std::fprintf(stderr,
                     "usage: reluva-stafu-inventory <installed|cd> <source-directory>\n");
        return 2;
    }

    const auto kind = std::strcmp(argv[1], "cd") == 0
        ? reluva::stafu::SourceKind::CdContents
        : reluva::stafu::SourceKind::InstalledGame;
    const auto inventory = reluva::stafu::inventorySource(
        std::filesystem::path(argv[2]), kind);

    std::puts("relative_path\tsize_bytes\tmd5");
    uintmax_t totalBytes = 0;
    for (const auto& file : inventory.files) {
        const std::string path = escapeTsv(file.relativePath);
        std::printf("%s\t%ju\t%s\n", path.c_str(),
                    static_cast<uintmax_t>(file.size), file.md5.c_str());
        totalBytes += file.size;
    }

    std::fprintf(stderr, "Source: %s\nFiles: %zu\nBytes: %ju\nStatus: %s\n",
                 argv[1], inventory.files.size(),
                 static_cast<uintmax_t>(totalBytes),
                 inventory.complete() ? "complete" : "incomplete");
    for (const auto& error : inventory.errors)
        std::fprintf(stderr, "ERROR: %s\n", error.c_str());

    return inventory.complete() ? 0 : 1;
}
