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
    size_t candidates = 0, overlaps = 0, examined = 0, inFile = 0;
    size_t stringStarts = 0, primary = 0, increasingTriples = 0;
};
struct Hit {
    size_t position;
    uint32_t value;
    bool isString, isPrimary;
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
    std::map<size_t, size_t> zeroHistogram;
    std::printf("computer.db: %zu bytes; primary offsets=%zu; ASCII string starts=%zu\n",
                b.size(), primary.size(), stringStarts.size());
    std::printf("Candidate triples: %zu\n", candidates.size());
    std::puts("Testing all byte alignments in the 48 bytes preceding each zero run.");
    std::puts("Offsets and target previews are observational; pointer semantics unverified.\n");
    for (size_t i = 0; i < candidates.size(); ++i) {
        const auto& c = candidates[i];
        ++total.candidates;
        ++zeroHistogram[c.zeroLength];
        const bool overlap = c.zeroLength < 2;
        if (overlap) ++total.overlaps;
        const bool selected = i < 12 || c.id == "Enterprise" ||
            c.id == "Cham_Field_1" || c.id == "Tact_Shields" ||
            c.id == "Iso_rod" || c.id == "Root";
        if (selected)
            std::printf("0x%08zx id=%s label=%s title=%s zeros=%zu %s\n",
                c.start, c.id.c_str(), c.label.c_str(), c.title.c_str(),
                c.zeroLength, overlap ? "[OVERLAP RISK]" : "");
        const size_t from = c.zeroStart > 48 ? c.zeroStart - 48 : 0;
        std::vector<Hit> hits;
        // Examine every byte position, not just words aligned to zeroStart.
        for (size_t pos = from; pos + 4 <= c.zeroStart; ++pos) {
            const uint32_t value = word(b, pos);
            ++total.examined;
            const bool valid = value != 0 && value < b.size();
            if (valid) ++total.inFile;
            const bool isString = valid && stringStarts.contains(value);
            const bool isPrimary = valid && primary.contains(value);
            if (isString) ++total.stringStarts;
            if (isPrimary) ++total.primary;
            if (valid) hits.push_back({pos, value, isString, isPrimary});
        }
        // A candidate triplet requires three consecutive u32 values, increasing,
        // in bounds, and with at least one target recognized as a string start.
        size_t triples = 0;
        for (size_t pos = from; pos + 12 <= c.zeroStart; ++pos) {
            const uint32_t v0 = word(b, pos), v1 = word(b, pos + 4),
                           v2 = word(b, pos + 8);
            if (!(v0 && v0 < v1 && v1 < v2 && v2 < b.size())) continue;
            if (!stringStarts.contains(v0) && !stringStarts.contains(v1) &&
                !stringStarts.contains(v2)) continue;
            ++triples;
            ++total.increasingTriples;
            if (selected && triples <= 4) {
                std::printf("  triple at 0x%08zx: 0x%08x 0x%08x 0x%08x\n",
                            pos, v0, v1, v2);
                for (const uint32_t value : {v0, v1, v2}) {
                    size_t target = value;
                    std::string preview;
                    const bool text = stringAt(b, target, preview, 160);
                    if (preview.size() > 80) preview.resize(80);
                    std::printf("    -> 0x%08x %s%s%s\n", value,
                        stringStarts.contains(value) ? "[STRING START] " : "",
                        primary.contains(value) ? "[PRIMARY] " : "",
                        text ? preview.c_str() : "(no bounded printable string)");
                }
            }
        }
        if (selected) {
            std::printf("  in-file u32 matches (%zu):\n", hits.size());
            size_t shown = 0;
            for (const auto& h : hits) {
                if (shown++ >= 16) { std::puts("    ..."); break; }
                std::printf("    at 0x%08zx -> 0x%08x %s%s\n",
                    h.position, h.value, h.isString ? "[STRING START] " : "",
                    h.isPrimary ? "[PRIMARY]" : "");
            }
            if (!triples) std::puts("  no qualifying increasing string-reference triple");
        }
    }
    std::printf("\nSummary: candidates=%zu overlap-risk=%zu examined-u32=%zu nonzero-in-file=%zu\n",
                total.candidates, total.overlaps, total.examined, total.inFile);
    std::printf("Exact ASCII string-start matches=%zu; primary-offset matches=%zu\n",
                total.stringStarts, total.primary);
    std::printf("Increasing in-file triples with >=1 string-start target=%zu\n",
                total.increasingTriples);
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
