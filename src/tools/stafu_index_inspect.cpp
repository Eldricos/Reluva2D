// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only exploration of the initial index structures in STAFU COMPUTER.DB.
// Candidate pointers are observations, not an established format specification.
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace {
uint32_t u32(const std::vector<uint8_t>& b, size_t at) {
    return uint32_t(b[at]) | (uint32_t(b[at + 1]) << 8) |
           (uint32_t(b[at + 2]) << 16) | (uint32_t(b[at + 3]) << 24);
}
struct Hit { size_t source; uint32_t target; };
void scan(const std::vector<uint8_t>& b, size_t begin, size_t end,
          size_t stride, const std::map<uint32_t, uint32_t>& indexed) {
    size_t valid = 0, exact = 0, other = 0;
    std::vector<Hit> hits;
    for (size_t at = begin; at + 4 <= end; at += stride) {
        const uint32_t value = u32(b, at);
        if (!value || value >= b.size()) continue;
        ++valid;
        if (indexed.contains(value)) {
            ++exact;
            if (hits.size() < 24) hits.push_back({at, value});
        } else ++other;
    }
    std::printf("  stride=%zu: nonzero in-file values=%zu, exact primary offsets=%zu, other=%zu\n",
                stride, valid, exact, other);
    for (const auto& h : hits)
        std::printf("    file 0x%08zx -> 0x%08x (primary index %u)\n",
                    h.source, h.target, indexed.at(h.target));
}
void dump(const std::vector<uint8_t>& b, size_t begin, size_t end) {
    for (size_t at = begin; at < end; at += 16) {
        const size_t n = std::min<size_t>(16, end - at);
        std::printf("  0x%08zx: ", at);
        for (size_t j = 0; j < 16; ++j)
            if (j < n) std::printf("%02x ", unsigned(b[at + j]));
            else std::printf("   ");
        std::printf(" |");
        for (size_t j = 0; j < n; ++j) {
            const auto c = b[at + j];
            std::putchar(c >= 32 && c <= 126 ? char(c) : '.');
        }
        std::puts("|");
    }
}
}
int stafuInspectIndex(const char* installed, const char* cd, const char* name) {
    reluva::stafu::ResourceManager resources;
    std::string error;
    if (!resources.open(installed, cd, error)) {
        std::fprintf(stderr, "index-inspect: %s\n", error.c_str()); return 1;
    }
    std::vector<uint8_t> b;
    if (!resources.read(name, b, error)) {
        std::fprintf(stderr, "index-inspect: %s\n", error.c_str()); return 1;
    }
    if (b.size() < 8) {
        std::fprintf(stderr, "index-inspect: file too small\n"); return 1;
    }
    const uint32_t count = u32(b, 0);
    const uint64_t tableEnd64 = 4ull + uint64_t(count) * 4ull;
    if (!count || tableEnd64 > b.size()) {
        std::fprintf(stderr, "index-inspect: candidate primary table does not fit\n");
        return 1;
    }
    const size_t tableEnd = static_cast<size_t>(tableEnd64);
    std::map<uint32_t, uint32_t> indexed;
    bool ordered = true, inBounds = true;
    uint32_t prev = 0;
    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t offset = u32(b, 4ull + uint64_t(i) * 4ull);
        if (offset >= b.size()) inBounds = false;
        if (i && offset < prev) ordered = false;
        indexed.emplace(offset, i);
        prev = offset;
    }
    std::printf("Resource: %s; bytes=%zu\n", name, b.size());
    std::printf("Primary candidate: count=%u; table=[0x00000004,0x%08zx); unique=%zu; ordered=%s; in-bounds=%s\n",
                count, tableEnd, indexed.size(), ordered ? "yes" : "no",
                inBounds ? "yes" : "no");
    if (!ordered || !inBounds) return 1;
    // Inspect exactly 128 bytes beyond the candidate table, with absolute addresses.
    const size_t windowEnd = std::min(b.size(), tableEnd + size_t(128));
    std::printf("Post-table window [0x%08zx,0x%08zx):\n", tableEnd, windowEnd);
    dump(b, tableEnd, windowEnd);
    std::puts("Post-table 32-bit LE words (including zero and non-pointer values):");
    for (size_t at = tableEnd; at + 4 <= windowEnd; at += 4) {
        const uint32_t value = u32(b, at);
        const auto it = indexed.find(value);
        std::printf("  0x%08zx: 0x%08x (%u) %s\n", at, value, value,
                    !value ? "zero" : it != indexed.end() ? "EXACT PRIMARY OFFSET" :
                    value < b.size() ? "other in-file value" : "out of file");
    }
    std::puts("Nonzero candidate reference statistics (all positions are absolute):");
    std::printf("Post-table window [0x%08zx,0x%08zx):\n", tableEnd, windowEnd);
    scan(b, tableEnd, windowEnd, 4, indexed);
    std::printf("Remaining file [0x%08zx,0x%08zx):\n", windowEnd, b.size());
    scan(b, windowEnd, b.size(), 4, indexed);
    scan(b, windowEnd, b.size(), 1, indexed);
    std::puts("Interpretation remains provisional: matches do not prove pointer semantics.");
    return 0;
}
