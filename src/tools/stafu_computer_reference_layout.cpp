// SPDX-License-Identifier: GPL-3.0-or-later
// Diagnostic only: compare candidate string prefixes with nearby u32 values.
#include "engines/stafu/computer_database_index.h"
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace {
bool printable(uint8_t c) { return c >= 32 && c <= 126; }
bool stringAt(const std::vector<uint8_t>& b, size_t& p, std::string& out, size_t cap) {
    const size_t start = p;
    while (p < b.size() && printable(b[p]) && p - start <= cap) ++p;
    if (p == start || p >= b.size() || b[p] != 0 || p - start > cap) return false;
    out.assign(reinterpret_cast<const char*>(b.data() + start), p - start);
    ++p;
    return true;
}
bool idLike(const std::string& s) {
    if (s.size() < 3 || s.size() > 48) return false;
    bool letter = false;
    for (unsigned char c : s) {
        if (!(std::isalnum(c) || c == '_' || c == '-')) return false;
        if (std::isalpha(c)) letter = true;
    }
    return letter;
}
uint32_t word(const std::vector<uint8_t>& b, size_t p) {
    return uint32_t(b[p]) | (uint32_t(b[p+1]) << 8) |
           (uint32_t(b[p+2]) << 16) | (uint32_t(b[p+3]) << 24);
}
struct Candidate {
    size_t start, zeroStart, zeroLength;
    std::string id, label, title;
};
struct Stats {
    size_t candidates = 0, wordCount = 0, inFile = 0, stringStarts = 0, primary = 0;
};
}
int stafuAnalyzeComputerReferences(const char* installed, const char* cd) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if (!manager.open(installed, cd, error)) {
        std::fprintf(stderr, "computer-reference-layout: %s\n", error.c_str()); return 1;
    }
    std::vector<uint8_t> b;
    if (!manager.read("computer.db", b, error)) {
        std::fprintf(stderr, "computer-reference-layout: %s\n", error.c_str()); return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if (!index.parse(b, error)) {
        std::fprintf(stderr, "computer-reference-layout: %s\n", error.c_str()); return 1;
    }
    // Independently catalogue all ASCII NUL-terminated string starts.
    std::set<size_t> stringStarts;
    for (size_t p = index.tableEnd(); p < b.size(); ++p) {
        if (!printable(b[p]) || (p && printable(b[p-1]))) continue;
        size_t q = p;
        std::string value;
        if (stringAt(b, q, value, 1024)) stringStarts.insert(p);
    }
    std::set<uint32_t> primary(index.primaryOffsets().begin(), index.primaryOffsets().end());
    std::vector<Candidate> candidates;
    for (size_t p = index.tableEnd(); p < b.size(); ++p) {
        if (!printable(b[p]) || (p && printable(b[p-1]))) continue;
        size_t q = p;
        std::string id, label, title;
        if (!stringAt(b, q, id, 48) || !idLike(id) ||
            !stringAt(b, q, label, 64) || !stringAt(b, q, title, 96)) continue;
        size_t z = p;
        while (z && b[z-1] == 0) --z;
        candidates.push_back({p, z, p-z, id, label, title});
    }
    Stats total;
    size_t overlaps = 0;
    std::map<size_t, size_t> zeroHistogram;
    std::printf("computer.db: %zu bytes; primary offsets=%zu; ASCII string starts=%zu\n",
                b.size(), primary.size(), stringStarts.size());
    std::printf("Candidate triples: %zu\n", candidates.size());
    std::puts("Reference analysis: aligned u32 words in the 32 bytes immediately before the zero run.");
    std::puts("Matches mean numerical equality only, not confirmed pointer semantics.\n");
    for (size_t i = 0; i < candidates.size(); ++i) {
        const auto& c = candidates[i];
        ++zeroHistogram[c.zeroLength];
        const bool overlap = c.zeroLength < 2;
        if (overlap) ++overlaps;
        const bool selected = i < 12 || c.id == "Enterprise" ||
            c.id == "Cham_Field_1" || c.id == "Tact_Shields" ||
            c.id == "Iso_rod" || c.id == "Root";
        if (selected)
            std::printf("0x%08zx id=%s label=%s title=%s zeros=%zu %s\n",
                c.start, c.id.c_str(), c.label.c_str(), c.title.c_str(),
                c.zeroLength, overlap ? "[OVERLAP RISK]" : "");
        ++total.candidates;
        const size_t from = c.zeroStart > 32 ? c.zeroStart - 32 : 0;
        // Anchor at zeroStart: values are tested at 4-byte intervals backwards.
        for (size_t distance = 4; distance <= 32 && c.zeroStart >= distance; distance += 4) {
            const size_t pos = c.zeroStart - distance;
            if (pos < from || pos + 4 > b.size()) continue;
            const uint32_t value = word(b, pos);
            ++total.wordCount;
            const bool valid = value != 0 && value < b.size();
            const bool isString = valid && stringStarts.contains(value);
            const bool isPrimary = valid && primary.contains(value);
            if (valid) ++total.inFile;
            if (isString) ++total.stringStarts;
            if (isPrimary) ++total.primary;
            if (selected)
                std::printf("  -%2zu at 0x%08zx = 0x%08x %s%s%s\n",
                    distance, pos, value,
                    isString ? "STRING_START " : "",
                    isPrimary ? "PRIMARY_OFFSET " : "",
                    !valid ? "(zero/out-of-file)" : "");
        }
    }
    std::printf("\nSummary: candidates=%zu overlap-risk=%zu examined-u32=%zu nonzero-in-file=%zu\n",
                total.candidates, overlaps, total.wordCount, total.inFile);
    std::printf("Exact ASCII string-start matches=%zu; primary-offset matches=%zu\n",
                total.stringStarts, total.primary);
    std::puts("Common zero-run lengths:");
    std::vector<std::pair<size_t,size_t>> histogram(zeroHistogram.begin(), zeroHistogram.end());
    std::sort(histogram.begin(), histogram.end(), [](const auto& a, const auto& b) {
        return a.second != b.second ? a.second > b.second : a.first < b.first;
    });
    for (size_t i = 0; i < std::min<size_t>(10, histogram.size()); ++i)
        std::printf("  %zu zeros: %zu candidates\n", histogram[i].first, histogram[i].second);
    std::puts("No record boundary or pointer encoding has been established.");
    return 0;
}
