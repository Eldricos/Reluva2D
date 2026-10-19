// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only heuristic article discovery. No on-disk record semantics assumed.
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
bool ascii(uint8_t c) { return c >= 32 && c <= 126; }
bool readString(const std::vector<uint8_t>& b, size_t& p, std::string& out,
                size_t limit) {
    const size_t begin = p;
    while (p < b.size() && ascii(b[p]) && p - begin <= limit) ++p;
    if (p == begin || p >= b.size() || b[p] != 0 || p - begin > limit) return false;
    out.assign(reinterpret_cast<const char*>(b.data() + begin), p - begin);
    ++p;
    return true;
}
bool identifier(const std::string& s) {
    if (s.size() < 3 || s.size() > 48) return false;
    bool hasLetter = false;
    for (unsigned char c : s) {
        if (!(std::isalnum(c) || c == '_' || c == '-')) return false;
        if (std::isalpha(c)) hasLetter = true;
    }
    return hasLetter;
}
struct Candidate {
    size_t start, zeroCount, zeroStart, primaryIndex, primaryOffset;
    std::string id, label, title, body;
    bool bodyPresent;
};
void show(const std::vector<uint8_t>& bytes, const Candidate& c) {
    std::printf("  at 0x%08zx  id=%s | label=%s | title=%s\n",
                c.start, c.id.c_str(), c.label.c_str(), c.title.c_str());
    std::printf("    zero run [0x%08zx,0x%08zx) length=%zu; preceding bytes:",
                c.zeroStart, c.start, c.zeroCount);
    const size_t before = c.zeroStart > 16 ? c.zeroStart - 16 : 0;
    for (size_t p = before; p < c.zeroStart; ++p)
        std::printf(" %02x", unsigned(bytes[p]));
    if (before == c.zeroStart) std::printf(" (none)");
    std::printf("\n    nearest primary[%zu]=0x%08zx; delta=%zu\n",
                c.primaryIndex, c.primaryOffset, c.start - c.primaryOffset);
    std::printf("    following text: %s\n", c.bodyPresent ? c.body.c_str() : "(no immediate printable NUL-terminated string)");
}
}
int stafuScanComputerArticles(const char* installed, const char* cd) {
    reluva::stafu::ResourceManager resources;
    std::string error;
    if (!resources.open(installed, cd, error)) {
        std::fprintf(stderr, "computer-article-scan: %s\n", error.c_str()); return 1;
    }
    std::vector<uint8_t> bytes;
    if (!resources.read("computer.db", bytes, error)) {
        std::fprintf(stderr, "computer-article-scan: %s\n", error.c_str()); return 1;
    }
    reluva::stafu::ComputerDatabaseIndex index;
    if (!index.parse(bytes, error)) {
        std::fprintf(stderr, "computer-article-scan: %s\n", error.c_str()); return 1;
    }
    const auto& offsets = index.primaryOffsets();
    std::vector<Candidate> candidates;
    size_t triples = 0;
    // Consider each ASCII string start separately, including simple identifiers.
    for (size_t p = index.tableEnd(); p < bytes.size(); ++p) {
        if (!ascii(bytes[p]) || (p > 0 && ascii(bytes[p - 1]))) continue;
        size_t cursor = p;
        std::string id, label, title, body;
        if (!readString(bytes, cursor, id, 48) || !identifier(id)) continue;
        if (!readString(bytes, cursor, label, 64) ||
            !readString(bytes, cursor, title, 96)) continue;
        ++triples;
        const bool bodyPresent = readString(bytes, cursor, body, 512);
        if (bodyPresent && body.size() > 100) body.resize(100);
        size_t zeros = 0;
        while (zeros < p && bytes[p - zeros - 1] == 0) ++zeros;
        const auto nearest = std::upper_bound(offsets.begin(), offsets.end(), p);
        if (nearest == offsets.begin()) continue;
        const size_t ordinal = size_t(nearest - offsets.begin() - 1);
        candidates.push_back({p, zeros, p - zeros, ordinal, offsets[ordinal],
                              id, label, title, body, bodyPresent});
    }
    std::map<size_t, size_t> zeros;
    size_t withBody = 0, withPadding = 0, longPadding = 0;
    for (const auto& c : candidates) {
        ++zeros[c.zeroCount];
        if (c.bodyPresent) ++withBody;
        if (c.zeroCount) ++withPadding;
        if (c.zeroCount >= 32) ++longPadding;
    }
    std::printf("computer.db: %zu bytes; primary offsets=%zu\n", bytes.size(), offsets.size());
    std::printf("Syntactic identifier/label/title triples: %zu\n", triples);
    std::printf("Candidates with preceding primary offset: %zu\n", candidates.size());
    std::printf("With immediately following NUL-terminated text: %zu\n", withBody);
    std::printf("With preceding zero bytes: %zu; with >=32 zero bytes: %zu\n",
                withPadding, longPadding);
    std::puts("\nFirst 30 candidates:");
    for (size_t i = 0; i < std::min<size_t>(30, candidates.size()); ++i)
        show(bytes, candidates[i]);
    std::puts("\nPadding lengths (top 12):");
    std::vector<std::pair<size_t, size_t>> histogram(zeros.begin(), zeros.end());
    std::sort(histogram.begin(), histogram.end(), [](const auto& a, const auto& b) {
        return a.second != b.second ? a.second > b.second : a.first < b.first;
    });
    for (size_t i = 0; i < std::min<size_t>(12, histogram.size()); ++i)
        std::printf("  %zu zero bytes: %zu candidates\n", histogram[i].first, histogram[i].second);
    std::puts("\nSelected known candidates (if matched):");
    for (const auto& c : candidates)
        if (c.id == "Enterprise" || c.id == "Cham_Field_1" ||
            c.id == "Tact_Shields" || c.id == "Iso_rod") show(bytes, c);
    std::puts("\nHeuristic matches are NOT validated article boundaries or decoded records.");
    return 0;
}
