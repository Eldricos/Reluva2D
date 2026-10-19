// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only investigation of candidate COMPUTER.DB offset boundaries.
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
uint32_t u32(const std::vector<uint8_t>& b, size_t p) {
    return uint32_t(b[p]) | (uint32_t(b[p + 1]) << 8) |
           (uint32_t(b[p + 2]) << 16) | (uint32_t(b[p + 3]) << 24);
}
void bytesAt(const std::vector<uint8_t>& b, size_t at, size_t length) {
    const size_t end = std::min(b.size(), at + length);
    std::printf("    0x%08zx: ", at);
    for (size_t p = at; p < end; ++p) std::printf("%02x ", unsigned(b[p]));
    std::printf(" |");
    for (size_t p = at; p < end; ++p) {
        const unsigned char c = b[p];
        std::putchar(c >= 32 && c <= 126 ? c : '.');
    }
    std::puts("|");
}
}
int stafuInspectRanges(const char* installed, const char* cd, const char* name) {
    reluva::stafu::ResourceManager rm;
    std::string error;
    if (!rm.open(installed, cd, error)) {
        std::fprintf(stderr, "range-inspect: %s\n", error.c_str()); return 1;
    }
    std::vector<uint8_t> b;
    if (!rm.read(name, b, error)) {
        std::fprintf(stderr, "range-inspect: %s\n", error.c_str()); return 1;
    }
    if (b.size() < 8) {
        std::fprintf(stderr, "range-inspect: resource too small\n"); return 1;
    }
    const uint32_t count = u32(b, 0);
    if (!count || 4ull + uint64_t(count) * 4 > b.size()) {
        std::puts("No bounded candidate u32 offset table at file start.");
        return 1;
    }
    std::printf("Resource: %s, size: %zu, candidate offsets: %u\n", name, b.size(), count);
    const uint32_t shown = std::min<uint32_t>(count, 16);
    for (uint32_t i = 0; i < shown; ++i) {
        const uint32_t start = u32(b, 4ull + uint64_t(i) * 4);
        const uint32_t end = i + 1 < count ? u32(b, 4ull + uint64_t(i + 1) * 4)
                                           : static_cast<uint32_t>(b.size());
        if (start > end || end > b.size()) {
            std::printf("[%u] invalid interval [%u, %u)\n", i, start, end);
            continue;
        }
        std::printf("[%u] [0x%08x, 0x%08x) length=%u\n", i, start, end, end - start);
        std::puts("  Start:");
        bytesAt(b, start, std::min<size_t>(32, end - start));
        if (end > start + 32) {
            std::puts("  End:");
            bytesAt(b, end - std::min<size_t>(32, end - start),
                    std::min<size_t>(32, end - start));
        }
    }
    std::puts("Intervals are mathematical differences between candidate offsets, not confirmed records.");
    return 0;
}
