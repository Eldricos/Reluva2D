// SPDX-License-Identifier: GPL-3.0-or-later
// Observational COMPUTER.DB entry-prefix comparison. No record format is assumed.
#include "engines/stafu/computer_database_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace {
bool visible(uint8_t c) { return c >= 32 && c <= 126; }
bool identifier(const std::string& s) {
    if (s.size() < 5 || s.size() > 48) return false;
    bool marker = false;
    for (unsigned char c : s) {
        if (!(std::isalnum(c) || c == '_' || c == '-')) return false;
        if (c == '_' || c == '-') marker = true;
    }
    return marker;
}
struct Candidate {
    size_t start;
    size_t precedingZeros;
    size_t nearestPrimary;
    size_t primaryIndex;
    std::string id, label, title;
};
bool nextString(const std::vector<uint8_t>& b, size_t& pos, std::string& value) {
    if (pos >= b.size()) return false;
    const size_t start = pos;
    while (pos < b.size() && visible(b[pos]) && pos - start <= 128) ++pos;
    if (pos == start || pos >= b.size() || b[pos] != 0 || pos - start > 128) return false;
    value.assign(reinterpret_cast<const char*>(b.data() + start), pos - start);
    ++pos;
    return true;
}
void printCandidate(const Candidate& c) {
    std::printf("  0x%08zx  zeros=%3zu  primary[%zu]=0x%08zx  delta=%+lld  id=%s | label=%s | title=%s\n",
        c.start, c.precedingZeros, c.primaryIndex, c.nearestPrimary,
        static_cast<long long>(c.start) - static_cast<long long>(c.nearestPrimary),
        c.id.c_str(), c.label.c_str(), c.title.c_str());
}
}
int stafuInspectComputerHeaders(const char* installed, const char* cd) {
    reluva::stafu::ResourceManager resources;
    std::string error;
    if (!resources.open(installed, cd, error)) {
        std::fprintf(stderr, "computer-header-compare: %s\n", error.c_str()); return 1;
    }
    std::vector<uint8_t> bytes;
    if (!resources.read("computer.db", bytes, error)) {
        std::fprintf(stderr, "computer-header-compare: %s\n", error.c_str()); return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if (!index.parse(bytes, error)) {
        std::fprintf(stderr, "computer-header-compare: %s\n", error.c_str()); return 1;
    }
    std::vector<Candidate> candidates;
    const auto& offsets = index.primaryOffsets();
    // Scan for three adjacent NUL-terminated strings where the first resembles
    // an internal identifier. These are candidates, not confirmed entries.
    for (size_t p = index.tableEnd(); p < bytes.size();) {
        if (!visible(bytes[p]) || (p > 0 && visible(bytes[p - 1]))) { ++p; continue; }
        const size_t start = p;
        size_t cursor = p;
        std::string id, label, title;
        if (!nextString(bytes, cursor, id)) { ++p; continue; }
        p = cursor;
        if (!identifier(id)) continue;
        if (!nextString(bytes, cursor, label) || label.size() > 64 ||
            !nextString(bytes, cursor, title) || title.size() > 96) continue;
        size_t zeros = 0;
        while (zeros < 256 && zeros < start && bytes[start - zeros - 1] == 0) ++zeros;
        const auto nearest = std::upper_bound(offsets.begin(), offsets.end(), start);
        if (nearest == offsets.begin()) continue;
        const size_t ordinal = size_t(nearest - offsets.begin() - 1);
        candidates.push_back({start, zeros, offsets[ordinal], ordinal, id, label, title});
    }
    std::printf("computer.db: %zu bytes; primary offsets=%zu\n", bytes.size(), offsets.size());
    std::printf("Candidate identifier/label/title triples: %zu\n", candidates.size());
    std::map<size_t, size_t> zeroHistogram, deltaHistogram;
    for (const auto& c : candidates) {
        ++zeroHistogram[c.precedingZeros];
        const size_t delta = c.start - c.nearestPrimary;
        if (delta <= 256) ++deltaHistogram[delta];
    }
    std::puts("\nFirst 24 candidates:");
    for (size_t i = 0; i < std::min<size_t>(24, candidates.size()); ++i)
        printCandidate(candidates[i]);
    std::puts("\nCandidates with 32-byte gap from nearest preceding primary offset (first 12):");
    size_t shown = 0;
    for (const auto& c : candidates)
        if (c.start - c.nearestPrimary == 32 && shown++ < 12) printCandidate(c);
    if (!shown) std::puts("  (none)");
    std::puts("\nPreceding zero-byte run lengths (top 12 by frequency):");
    std::vector<std::pair<size_t, size_t>> zeros(zeroHistogram.begin(), zeroHistogram.end());
    std::sort(zeros.begin(), zeros.end(), [](const auto& a, const auto& b) {
        return a.second != b.second ? a.second > b.second : a.first < b.first;
    });
    for (size_t i = 0; i < std::min<size_t>(12, zeros.size()); ++i)
        std::printf("  %zu zeros: %zu candidates\n", zeros[i].first, zeros[i].second);
    std::puts("\nIdentifier-to-nearest-primary distances <=256 (top 12):");
    std::vector<std::pair<size_t, size_t>> deltas(deltaHistogram.begin(), deltaHistogram.end());
    std::sort(deltas.begin(), deltas.end(), [](const auto& a, const auto& b) {
        return a.second != b.second ? a.second > b.second : a.first < b.first;
    });
    for (size_t i = 0; i < std::min<size_t>(12, deltas.size()); ++i)
        std::printf("  +%zu bytes: %zu candidates\n", deltas[i].first, deltas[i].second);
    std::puts("\nHeuristic comparison only. String triples and primary offsets are not verified record headers.");
    return 0;
}
