// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only comparison of candidate COMPUTER.DB article/string structures.
#include "engines/stafu/computer_database_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace {
uint32_t le32(const std::vector<uint8_t>& b, size_t p) {
    return uint32_t(b[p]) | (uint32_t(b[p + 1]) << 8) |
           (uint32_t(b[p + 2]) << 16) | (uint32_t(b[p + 3]) << 24);
}
bool printable(uint8_t c) { return c >= 32 && c <= 126; }
void inspect(const std::vector<uint8_t>& b, const reluva::stafu::ComputerDatabaseIndex& index,
             const char* label, const char* needle) {
    const auto* first = reinterpret_cast<const uint8_t*>(needle);
    const size_t len = std::strlen(needle);
    auto it = std::search(b.begin(), b.end(), first, first + len);
    std::printf("\n=== %s ===\n", label);
    if (it == b.end()) { std::printf("Search string not found: %s\n", needle); return; }
    const size_t hit = size_t(it - b.begin());
    const size_t begin = hit > 160 ? hit - 160 : 0;
    const size_t end = std::min(b.size(), hit + 320);
    std::printf("Search match at 0x%08zx; examination [0x%08zx,0x%08zx)\n", hit, begin, end);
    std::printf("Primary offsets in window (NOT proven article starts):\n");
    bool found = false;
    for (size_t i = 0; i < index.primaryOffsets().size(); ++i) {
        const size_t offset = index.primaryOffsets()[i];
        if (offset >= begin && offset < end) {
            std::printf("  primary[%zu] = 0x%08zx\n", i, offset);
            found = true;
        }
    }
    if (!found) std::puts("  (none)");
    std::puts("Printable null-terminated candidates (up to 18; >=4 chars):");
    size_t shown = 0;
    for (size_t p = begin; p < end && shown < 18;) {
        if (!printable(b[p]) || (p > 0 && printable(b[p - 1]))) { ++p; continue; }
        const size_t start = p;
        while (p < b.size() && printable(b[p]) && p - start < 256) ++p;
        if (p < b.size() && b[p] == 0 && p - start >= 4) {
            std::printf("  0x%08zx len=%zu: ", start, p - start);
            for (size_t k = start; k < std::min(p, start + 100); ++k) std::putchar(char(b[k]));
            if (p - start > 100) std::printf("...");
            std::putchar('\n');
            ++shown;
        }
        if (p == start) ++p;
    }
    if (!shown) std::puts("  (none)");
    // Show candidate u32 fields before the first matching string. No pointer semantics assumed.
    const size_t wordsBegin = hit > 48 ? hit - 48 : 0;
    std::printf("32-bit LE words preceding match (byte positions are absolute):\n");
    for (size_t p = wordsBegin; p + 4 <= hit; p += 4)
        std::printf("  0x%08zx: 0x%08x\n", p, le32(b, p));
    // Find exact file-position references to the search string. Check all byte alignments.
    std::printf("Exact references to search start 0x%08zx (first 12, excluding self):\n", hit);
    shown = 0;
    if (hit <= UINT32_MAX) {
        for (size_t p = index.tableEnd(); p + 4 <= b.size() && shown < 12; ++p) {
            if (p == hit || le32(b, p) != uint32_t(hit)) continue;
            std::printf("  0x%08zx -> 0x%08zx\n", p, hit);
            ++shown;
        }
    }
    if (!shown) std::puts("  (none)");
}
}
int stafuInspectComputerStrings(const char* installed, const char* cd) {
    reluva::stafu::ResourceManager resources;
    std::string error;
    if (!resources.open(installed, cd, error)) {
        std::fprintf(stderr, "computer-string-inspect: %s\n", error.c_str()); return 1;
    }
    std::vector<uint8_t> b;
    if (!resources.read("computer.db", b, error)) {
        std::fprintf(stderr, "computer-string-inspect: %s\n", error.c_str()); return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if (!index.parse(b, error)) {
        std::fprintf(stderr, "computer-string-inspect: %s\n", error.c_str()); return 1;
    }
    std::printf("computer.db: %zu bytes; primary candidates=%zu\n",
                b.size(), index.primaryOffsets().size());
    inspect(b, index, "Enterprise", "U.S.S. Enterprise NCC-1701-D");
    inspect(b, index, "Deanna Troi", "Education: Psychology major at the University of Betazed.");
    inspect(b, index, "Chameleon Field", "Cham_Field_1");
    std::puts("\nThese are string and reference observations, not decoded article records.");
    return 0;
}
