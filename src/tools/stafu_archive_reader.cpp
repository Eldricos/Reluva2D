// SPDX-License-Identifier: GPL-3.0-or-later
// STAFU developer command uses the same reader as the engine.
#include "engines/stafu/zip_archive.h"
#include <cstdio>
#include <string>
#include <vector>

int stafuArchiveTest(const char* path, const char* name) {
    reluva::stafu::ZipArchive archive;
    std::string error;
    std::vector<uint8_t> data;
    if (!archive.open(path, error) || !archive.read(name, data, error)) {
        std::fprintf(stderr, "archive-test: FAIL: %s: %s\n", name, error.c_str());
        return 1;
    }
    const auto* entry = archive.find(name);
    std::printf("PASS: %s  method=%u  compressed=%u  uncompressed=%u  crc32=%08x\n",
                name, unsigned(entry->method), unsigned(entry->compressedSize),
                unsigned(entry->uncompressedSize), unsigned(entry->crc32));
    return 0;
}
