// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

// Read-only STAFU ZIP central-directory inspection. No extraction or writes.
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace {
uint16_t u16(const unsigned char* p) { return uint16_t(p[0]) | uint16_t(uint16_t(p[1]) << 8); }
uint32_t u32(const unsigned char* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}
struct Directory {
    std::vector<unsigned char> raw;
    std::vector<std::string> rows;
    std::map<unsigned, size_t> methods;
    uint32_t offset = 0;
    uint32_t size = 0;
    uint16_t entries = 0;
    bool sidecar = false;
};
bool readAt(std::ifstream& f, uint64_t offset, unsigned char* p, size_t n) {
    f.clear();
    f.seekg(static_cast<std::streamoff>(offset));
    if (!f) return false;
    f.read(reinterpret_cast<char*>(p), static_cast<std::streamsize>(n));
    return static_cast<size_t>(f.gcount()) == n;
}
bool load(const char* path, Directory& d, std::string& error) {
    std::error_code ec;
    const auto length = std::filesystem::file_size(path, ec);
    if (ec) { error = ec.message(); return false; }
    std::ifstream f(path, std::ios::binary);
    if (!f) { error = "Cannot open input"; return false; }
    const size_t tailSize = static_cast<size_t>(std::min<uintmax_t>(length, 65557));
    std::vector<unsigned char> tail(tailSize);
    if (!readAt(f, length - tailSize, tail.data(), tail.size())) {
        error = "Cannot read archive tail"; return false;
    }
    size_t eocd = tailSize;
    if (tailSize >= 22) {
        for (size_t i = tailSize - 22;; --i) {
            if (u32(tail.data() + i) == 0x06054b50 &&
                i + 22 + u16(tail.data() + i + 20) == tailSize) { eocd = i; break; }
            if (i == 0) break;
        }
    }
    if (eocd == tailSize) { error = "ZIP end-of-central-directory not found"; return false; }
    const auto* e = tail.data() + eocd;
    if (u16(e + 4) || u16(e + 6) || u16(e + 8) != u16(e + 10)) {
        error = "Multi-disk ZIP unsupported"; return false;
    }
    d.entries = u16(e + 10);
    d.size = u32(e + 12);
    d.offset = u32(e + 16);
    if (d.entries == 0xffff || d.size == 0xffffffffu || d.offset == 0xffffffffu) {
        error = "ZIP64 unsupported"; return false;
    }
    const uint64_t physicalEnd = length - tailSize + eocd;
    // STTNG.DIR: 20-byte prefix, then copied central directory and EOCD.
    // A conventional ZIP instead uses the EOCD's central-directory offset.
    uint64_t physicalStart = d.offset;
    if (uint64_t(d.offset) + d.size != physicalEnd) {
        if (physicalEnd == uint64_t(d.size) + 20) {
            physicalStart = 20;
            d.sidecar = true;
        } else {
            error = "Central directory offset/size does not match archive layout";
            return false;
        }
    }
    if (d.size > 64 * 1024 * 1024 || physicalStart > physicalEnd ||
        uint64_t(d.size) != physicalEnd - physicalStart) {
        error = "Invalid or oversized central directory"; return false;
    }
    d.raw.resize(d.size);
    if (!readAt(f, physicalStart, d.raw.data(), d.raw.size())) {
        error = "Cannot read central directory"; return false;
    }
    size_t pos = 0;
    for (size_t i = 0; i < d.entries; ++i) {
        if (pos + 46 > d.raw.size() || u32(d.raw.data() + pos) != 0x02014b50) {
            error = "Invalid central directory record at entry " + std::to_string(i);
            return false;
        }
        const auto* p = d.raw.data() + pos;
        const unsigned nameLen = u16(p + 28), extraLen = u16(p + 30), commentLen = u16(p + 32);
        const size_t recordSize = 46u + nameLen + extraLen + commentLen;
        if (recordSize > d.raw.size() - pos) {
            error = "Truncated central directory record"; return false;
        }
        const unsigned method = u16(p + 10);
        ++d.methods[method];
        const std::string name(reinterpret_cast<const char*>(p + 46), nameLen);
        // Escape nonprintable characters to keep one row per archive entry.
        std::string safe;
        for (unsigned char c : name) {
            if (c >= 32 && c <= 126 && c != '\\') safe += char(c);
            else {
                char escaped[5];
                std::snprintf(escaped, sizeof escaped, "\\x%02X", unsigned(c));
                safe += escaped;
            }
        }
        char row[160];
        std::snprintf(row, sizeof row, "%5zu  method=%u flags=%04x compressed=%u uncompressed=%u local=%u  ",
                      i, method, unsigned(u16(p + 8)), unsigned(u32(p + 20)),
                      unsigned(u32(p + 24)), unsigned(u32(p + 42)));
        d.rows.emplace_back(std::string(row) + safe);
        pos += recordSize;
    }
    if (pos != d.raw.size()) { error = "Trailing bytes in central directory"; return false; }
    return true;
}
void summary(const Directory& d) {
    std::printf("Entries: %u  central-size: %u  declared-offset: %u  layout: %s\n",
                unsigned(d.entries), unsigned(d.size), unsigned(d.offset),
                d.sidecar ? "STTNG.DIR-style sidecar" : "standard ZIP");
    std::puts("Compression methods:");
    for (const auto& [method, count] : d.methods)
        std::printf("  method %u: %zu entries\n", method, count);
}
} // namespace

int stafuArchiveList(const char* path) {
    Directory d;
    std::string error;
    if (!load(path, d, error)) {
        std::fprintf(stderr, "archive-list: %s: %s\n", path, error.c_str());
        return 1;
    }
    std::printf("Archive: %s\n", path);
    summary(d);
    for (const auto& row : d.rows) std::puts(row.c_str());
    return 0;
}

int stafuArchiveCompare(const char* zipPath, const char* dirPath) {
    Directory zip, dir;
    std::string error;
    if (!load(zipPath, zip, error)) {
        std::fprintf(stderr, "archive-compare: %s: %s\n", zipPath, error.c_str());
        return 1;
    }
    if (!load(dirPath, dir, error)) {
        std::fprintf(stderr, "archive-compare: %s: %s\n", dirPath, error.c_str());
        return 1;
    }
    std::printf("ZIP: %s\n", zipPath);
    summary(zip);
    std::printf("DIR: %s\n", dirPath);
    summary(dir);
    const bool metadata = zip.entries == dir.entries && zip.size == dir.size &&
                          zip.offset == dir.offset;
    const bool bytes = zip.raw == dir.raw;
    std::printf("EOCD directory metadata: %s\n", metadata ? "MATCH" : "DIFFER");
    std::printf("Central directory bytes: %s\n", bytes ? "IDENTICAL" : "DIFFER");
    if (!bytes) {
        const size_t n = std::min(zip.raw.size(), dir.raw.size());
        size_t i = 0;
        while (i < n && zip.raw[i] == dir.raw[i]) ++i;
        if (i < n) std::printf("First differing directory byte: %zu\n", i);
        else std::puts("Directory lengths differ");
    }
    return metadata && bytes ? 0 : 1;
}
