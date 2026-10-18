// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#include "engines/stafu/source_inventory.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

int failures = 0;

#define CHECK(cond) do { \
    if (!(cond)) { \
        std::fprintf(stderr, "%s:%d: CHECK failed: %s\\n", __FILE__, __LINE__, #cond); \
        ++failures; \
    } \
} while (0)

void writeFile(const std::filesystem::path& path, const std::string& contents) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out.write(contents.data(), static_cast<std::streamsize>(contents.size()));
}

void testInventory(const std::filesystem::path& root) {
    std::filesystem::remove_all(root);
    writeFile(root / "DATA" / "second.bin", "abc");
    writeFile(root / "first.dat", "");
    writeFile(root / "DATA" / "nested" / "third.bin", "hello");

    const auto inventory = reluva::stafu::inventorySource(
        root, reluva::stafu::SourceKind::CdContents);
    CHECK(inventory.complete());
    CHECK(inventory.kind == reluva::stafu::SourceKind::CdContents);
    CHECK(inventory.files.size() == 3);
    CHECK(inventory.files.size() < 2 || inventory.files[0].relativePath == "DATA/nested/third.bin");
    CHECK(inventory.files.size() < 2 || inventory.files[1].relativePath == "DATA/second.bin");
    CHECK(inventory.files.size() < 3 || inventory.files[2].relativePath == "first.dat");

    if (inventory.files.size() == 3) {
        CHECK(inventory.files[0].size == 5);
        CHECK(inventory.files[0].md5 == "5d41402abc4b2a76b9719d911017c592");
        CHECK(inventory.files[1].size == 3);
        CHECK(inventory.files[1].md5 == "900150983cd24fb0d6963f7d28e17f72");
        CHECK(inventory.files[2].size == 0);
        CHECK(inventory.files[2].md5 == "d41d8cd98f00b204e9800998ecf8427e");
    }

    std::filesystem::remove_all(root);
}

void testInvalidRoot(const std::filesystem::path& root) {
    std::filesystem::remove_all(root);
    const auto missing = reluva::stafu::inventorySource(
        root, reluva::stafu::SourceKind::InstalledGame);
    CHECK(!missing.complete());
    CHECK(missing.files.empty());
    CHECK(!missing.errors.empty());

    writeFile(root / "not-a-directory", "x");
    const auto fileRoot = reluva::stafu::inventorySource(
        root / "not-a-directory", reluva::stafu::SourceKind::InstalledGame);
    CHECK(!fileRoot.complete());
    CHECK(fileRoot.files.empty());

    std::filesystem::remove_all(root);
}

} // namespace

int main() {
    const auto tmp = std::filesystem::temp_directory_path() / "reluva_stafu_inventory_tests";
    testInventory(tmp / "tree");
    testInvalidRoot(tmp / "invalid");
    if (failures)
        std::fprintf(stderr, "%d check(s) failed\\n", failures);
    else
        std::printf("All STAFU inventory tests passed\\n");
    return failures ? 1 : 0;
}
