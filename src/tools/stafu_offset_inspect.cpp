// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only candidate offset-table analysis. No file-format assumptions are enforced.
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
uint32_t le32(const std::vector<uint8_t>& b, size_t i) {
    return uint32_t(b[i]) | (uint32_t(b[i + 1]) << 8) |
           (uint32_t(b[i + 2]) << 16) | (uint32_t(b[i + 3]) << 24);
}
void preview(const std::vector<uint8_t>& b, size_t at) {
    const size_t n = std::min<size_t>(32, b.size() - at);
    for (size_t j = 0; j < n; ++j) std::printf("%02x ", unsigned(b[at + j]));
    std::printf(" |");
    for (size_t j = 0; j < n; ++j) {
        const auto c = b[at + j];
        std::putchar(c >= 32 && c <= 126 ? char(c) : '.');
    }
    std::puts("|");
}
}
int stafuInspectOffsets(const char* installed, const char* cd, const char* name) {
    reluva::stafu::ResourceManager resources;
    std::string error;
    if (!resources.open(installed, cd, error)) {
        std::fprintf(stderr, "offset-inspect: %s\n", error.c_str()); return 1;
    }
    std::vector<uint8_t> bytes;
    if (!resources.read(name, bytes, error)) {
        std::fprintf(stderr, "offset-inspect: %s\n", error.c_str()); return 1;
    }
    if (bytes.size() < 8) {
        std::fprintf(stderr, "offset-inspect: resource too short\n"); return 1;
    }
    const uint32_t count = le32(bytes, 0);
    const uint64_t tableEnd = 4ull + uint64_t(count) * 4ull;
    std::printf("Resource: %s\nSize: %zu\nCandidate count (u32 LE at 0): %u\n",
                name, bytes.size(), count);
    std::printf("Candidate table: [4, %llu)\n",
                static_cast<unsigned long long>(tableEnd));
    if (count == 0 || tableEnd > bytes.size()) {
        std::puts("REJECTED: count does not fit a table of 32-bit offsets.");
        return 0;
    }
    uint32_t valid = 0, monotonic = 0, afterTable = 0;
    uint32_t previous = 0;
    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t offset = le32(bytes, 4ull + uint64_t(i) * 4ull);
        if (offset < bytes.size()) ++valid;
        if (offset >= tableEnd && offset < bytes.size()) ++afterTable;
        if (i == 0 || offset >= previous) ++monotonic;
        previous = offset;
    }
    std::printf("Offsets within file: %u/%u\n", valid, count);
    std::printf("Offsets after candidate table: %u/%u\n", afterTable, count);
    std::printf("Nondecreasing transitions: %u/%u\n", monotonic, count);
    std::puts("First 12 candidate offsets and raw 32-byte previews:");
    for (uint32_t i = 0; i < std::min<uint32_t>(count, 12); ++i) {
        const uint32_t offset = le32(bytes, 4ull + uint64_t(i) * 4ull);
        std::printf("  [%u] 0x%08x (%u): ", i, offset, offset);
        if (offset < bytes.size()) preview(bytes, offset);
        else std::puts("(outside file)");
    }
    if (count > 12) {
        const uint32_t last = le32(bytes, 4ull + uint64_t(count - 1) * 4ull);
        std::printf("  [last %u] 0x%08x (%u): ", count - 1, last, last);
        if (last < bytes.size()) preview(bytes, last);
        else std::puts("(outside file)");
    }
    std::puts("Candidate interpretation only; offset semantics not established.");
    return 0;
}
