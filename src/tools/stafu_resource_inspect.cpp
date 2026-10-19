// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only STAFU resource inspection; format interpretations remain unverified.
#include "engines/stafu/resource_manager.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {
void dump(const std::vector<uint8_t>& bytes, size_t start, size_t count) {
    const size_t end = std::min(bytes.size(), start + count);
    for (size_t i = start; i < end; i += 16) {
        std::printf("%08zx: ", i);
        for (size_t j = 0; j < 16; ++j)
            if (i + j < end) std::printf("%02x ", unsigned(bytes[i + j]));
            else std::printf("   ");
        std::printf(" |");
        for (size_t j = i; j < std::min(end, i + 16); ++j) {
            const unsigned char c = bytes[j];
            std::putchar(c >= 32 && c <= 126 ? c : '.');
        }
        std::puts("|");
    }
}
void strings(const std::vector<uint8_t>& bytes) {
    constexpr size_t kMin = 6, kMaxOutput = 24;
    size_t found = 0;
    for (size_t i = 0; i < bytes.size() && found < kMaxOutput;) {
        if (bytes[i] < 32 || bytes[i] > 126) { ++i; continue; }
        const size_t start = i;
        while (i < bytes.size() && bytes[i] >= 32 && bytes[i] <= 126) ++i;
        if (i - start < kMin) continue;
        const size_t length = std::min<size_t>(i - start, 100);
        std::printf("  0x%08zx (%zu bytes): %.*s%s\n", start, i - start,
                    int(length), reinterpret_cast<const char*>(bytes.data() + start),
                    i - start > length ? "..." : "");
        ++found;
    }
    if (!found) std::puts("  (no ASCII runs of six or more bytes found)");
}
}
int stafuInspectResource(const char* installed, const char* cd, const char* name) {
    reluva::stafu::ResourceManager manager;
    std::string error;
    if (!manager.open(installed, cd, error)) {
        std::fprintf(stderr, "resource-inspect: %s\n", error.c_str());
        return 1;
    }
    reluva::stafu::ResourceLocation location{};
    if (!manager.locate(name, location)) {
        std::fprintf(stderr, "resource-inspect: resource not found or invalid name\n");
        return 1;
    }
    std::vector<uint8_t> bytes;
    if (!manager.read(name, bytes, error)) {
        std::fprintf(stderr, "resource-inspect: %s\n", error.c_str());
        return 1;
    }
    const char* origin = location.origin == reluva::stafu::ResourceOrigin::InstalledFile
        ? "installed file" : location.origin == reluva::stafu::ResourceOrigin::MainArchive
        ? "STTNG.ZIP" : "STTNGINS.ZIP";
    std::printf("Resource: %s\nOrigin: %s\nSize: %zu bytes\n", name, origin, bytes.size());
    std::puts("First 128 bytes:");
    dump(bytes, 0, std::min<size_t>(128, bytes.size()));
    if (bytes.size() > 64) {
        std::puts("Last 64 bytes:");
        dump(bytes, bytes.size() - 64, 64);
    }
    std::puts("First 24 printable ASCII runs (minimum six characters):");
    strings(bytes);
    std::puts("These are raw observations, not a decoded file format.");
    return 0;
}
