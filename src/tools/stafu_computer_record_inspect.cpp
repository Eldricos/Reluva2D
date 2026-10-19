// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only observations around indexed COMPUTER.DB positions.
// Primary offsets are NOT assumed to delimit logical records.
#include "engines/stafu/computer_database_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace {
uint32_t read32(const std::vector<uint8_t>& b, size_t p) {
    return uint32_t(b[p]) | (uint32_t(b[p + 1]) << 8) |
           (uint32_t(b[p + 2]) << 16) | (uint32_t(b[p + 3]) << 24);
}
void dump(const std::vector<uint8_t>& b, size_t start, size_t end) {
    for (size_t p = start; p < end; p += 16) {
        std::printf("    0x%08zx: ", p);
        const size_t n = std::min<size_t>(16, end - p);
        for (size_t j = 0; j < 16; ++j) {
            if (j < n) std::printf("%02x ", unsigned(b[p + j]));
            else std::printf("   ");
        }
        std::printf(" |");
        for (size_t j = 0; j < n; ++j) {
            const auto c = b[p + j];
            std::putchar(c >= 32 && c <= 126 ? char(c) : '.');
        }
        std::puts("|");
    }
}
void inspect(const std::vector<uint8_t>& b,
             const reluva::stafu::ComputerDatabaseIndex& index,
             uint32_t ordinal) {
    uint32_t offset = 0;
    if (!index.offsetAt(ordinal, offset)) return;
    const size_t at = offset;
    const size_t begin = at > 32 ? at - 32 : 0;
    const size_t end = std::min(b.size(), at + size_t(96));
    std::printf("\nPrimary[%u] = 0x%08x (%u)\n", ordinal, offset, offset);
    if (size_t(ordinal) + 1 < index.primaryOffsets().size()) {
        const auto next = index.primaryOffsets()[size_t(ordinal) + 1];
        std::printf("  Next candidate offset: 0x%08x (distance %u bytes; NOT a proven record length)\n",
                    next, next - offset);
    }
    std::printf("  Nearby bytes [0x%08zx,0x%08zx), marker at 0x%08zx:\n", begin, end, at);
    dump(b, begin, end);

    // Detect printable runs, including ones crossing the offset itself.
    const size_t textBegin = at > 128 ? at - 128 : 0;
    const size_t textEnd = std::min(b.size(), at + size_t(256));
    std::printf("  Printable runs in [0x%08zx,0x%08zx) (first 8, length >= 6):\n",
                textBegin, textEnd);
    size_t shown = 0;
    for (size_t p = textBegin; p < textEnd && shown < 8;) {
        if (b[p] < 32 || b[p] > 126) { ++p; continue; }
        const size_t first = p;
        while (p < textEnd && b[p] >= 32 && b[p] <= 126) ++p;
        if (p - first < 6) continue;
        std::printf("    0x%08zx (%zu bytes): ", first, p - first);
        const size_t limit = std::min(p, first + size_t(100));
        for (size_t j = first; j < limit; ++j) std::putchar(char(b[j]));
        if (limit < p) std::printf("...");
        std::putchar('\n');
        ++shown;
    }
    if (!shown) std::puts("    (none)");
    std::map<uint32_t, uint32_t> lookup;
    for (uint32_t i = 0; i < index.primaryOffsets().size(); ++i)
        lookup.emplace(index.primaryOffsets()[i], i);
    std::puts("  Nonzero 32-bit LE references matching primary offsets (first 12):");
    shown = 0;
    // Scan byte-by-byte: observed reference groups are not necessarily file-aligned.
    for (size_t p = begin; p + 4 <= end && shown < 12; ++p) {
        const uint32_t value = read32(b, p);
        const auto it = lookup.find(value);
        if (!value || it == lookup.end()) continue;
        std::printf("    0x%08zx -> primary[%u] (0x%08x)\n",
                    p, it->second, value);
        ++shown;
    }
    if (!shown) std::puts("    (none)");
}
}
int stafuInspectComputerRecords(const char* installed, const char* cd) {
    reluva::stafu::ResourceManager resources;
    std::string error;
    if (!resources.open(installed, cd, error)) {
        std::fprintf(stderr, "computer-record-inspect: resource manager: %s\n", error.c_str());
        return 1;
    }
    std::vector<uint8_t> bytes;
    if (!resources.read("computer.db", bytes, error)) {
        std::fprintf(stderr, "computer-record-inspect: resource read: %s\n", error.c_str());
        return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if (!index.parse(bytes, error)) {
        std::fprintf(stderr, "computer-record-inspect: index parse: %s\n", error.c_str());
        return 1;
    }
    std::printf("computer.db: %zu bytes; %zu primary candidate offsets\n",
                bytes.size(), index.primaryOffsets().size());
    constexpr std::array<uint32_t, 5> selected{1, 2, 3, 55, 162};
    for (uint32_t ordinal : selected) {
        if (ordinal >= index.primaryOffsets().size()) {
            std::fprintf(stderr, "computer-record-inspect: missing primary index %u\n", ordinal);
            return 1;
        }
        inspect(bytes, index, ordinal);
    }
    std::puts("\nObservations only: neither offsets nor nearby text are confirmed record boundaries.");
    return 0;
}
