// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only exploratory pointer-reference statistics for STAFU resources.
// Values resembling file offsets are candidates, not confirmed pointers.
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace {
uint32_t read32(const std::vector<uint8_t>& b, size_t i) {
    return uint32_t(b[i]) | (uint32_t(b[i + 1]) << 8) |
           (uint32_t(b[i + 2]) << 16) | (uint32_t(b[i + 3]) << 24);
}
struct Example { size_t source; uint32_t target; };
void analyze(const std::vector<uint8_t>& bytes, const std::map<uint32_t, uint32_t>& targets,
             size_t alignment) {
    uint64_t withinFile = 0, exact = 0;
    std::vector<Example> samples;
    std::map<uint32_t, uint32_t> hitCounts;
    for (size_t pos = 0; pos + 4 <= bytes.size(); pos += alignment) {
        const uint32_t value = read32(bytes, pos);
        if (value < bytes.size()) ++withinFile;
        if (targets.find(value) != targets.end()) {
            ++exact;
            ++hitCounts[value];
            if (samples.size() < 20) samples.push_back({pos, value});
        }
    }
    const uint64_t examined = bytes.size() < 4 ? 0 : (bytes.size() - 4) / alignment + 1;
    std::printf("Alignment %zu: examined=%llu within-file=%llu exact-table-target=%llu distinct-targets=%zu\n",
                alignment, static_cast<unsigned long long>(examined),
                static_cast<unsigned long long>(withinFile),
                static_cast<unsigned long long>(exact), hitCounts.size());
    std::puts("  First 20 exact matches (source byte position -> candidate target):");
    for (const auto& e : samples)
        std::printf("    0x%08zx -> 0x%08x (%u)\n", e.source, e.target, e.target);
    if (samples.empty()) std::puts("    (none)");
    std::vector<std::pair<uint32_t, uint32_t>> ranked(hitCounts.begin(), hitCounts.end());
    std::sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) {
        return a.second != b.second ? a.second > b.second : a.first < b.first;
    });
    std::puts("  Most frequently matched targets:");
    for (size_t i = 0; i < std::min<size_t>(10, ranked.size()); ++i)
        std::printf("    0x%08x: %u occurrences\n", ranked[i].first, ranked[i].second);
}
}
int stafuInspectReferences(const char* installed, const char* cd, const char* name) {
    reluva::stafu::ResourceManager resources;
    std::string error;
    if (!resources.open(installed, cd, error)) {
        std::fprintf(stderr, "reference-inspect: %s\n", error.c_str()); return 1;
    }
    std::vector<uint8_t> bytes;
    if (!resources.read(name, bytes, error)) {
        std::fprintf(stderr, "reference-inspect: %s\n", error.c_str()); return 1;
    }
    if (bytes.size() < 8) {
        std::fprintf(stderr, "reference-inspect: file too small\n"); return 1;
    }
    const uint32_t count = read32(bytes, 0);
    const uint64_t tableEnd = 4ull + uint64_t(count) * 4ull;
    if (!count || tableEnd > bytes.size()) {
        std::puts("No bounded candidate offset table at file start."); return 1;
    }
    std::map<uint32_t, uint32_t> targets;
    uint32_t previous = 0;
    bool monotonic = true;
    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t offset = read32(bytes, 4ull + uint64_t(i) * 4ull);
        if (offset >= bytes.size() || (i && offset < previous)) monotonic = false;
        targets.emplace(offset, i);
        previous = offset;
    }
    if (!monotonic) {
        std::puts("Candidate table contains out-of-bounds or decreasing offsets."); return 1;
    }
    std::printf("Resource: %s, size=%zu, candidate count=%u, unique targets=%zu\n",
                name, bytes.size(), count, targets.size());
    std::printf("Candidate table occupies [0x00000004, 0x%08llx).\n",
                static_cast<unsigned long long>(tableEnd));
    std::puts("All-file scans include the candidate table itself; those matches are tautological.");
    analyze(bytes, targets, 4);
    analyze(bytes, targets, 1);
    std::puts("Non-table scans (exclude first count and candidate offset table):");
    std::vector<uint8_t> payload(bytes.begin() + static_cast<size_t>(tableEnd), bytes.end());
    // Source positions in this section are relative to tableEnd.
    analyze(payload, targets, 4);
    analyze(payload, targets, 1);
    std::printf("Payload sample positions above are relative to file offset 0x%08llx.\n",
                static_cast<unsigned long long>(tableEnd));
    std::puts("No matches alone establish pointer semantics; accidental matches remain possible.");
    return 0;
}
