// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace reluva::stafu {
struct ZipEntry {
    std::string name;
    uint16_t method = 0, flags = 0;
    uint32_t crc32 = 0, compressedSize = 0, uncompressedSize = 0, localOffset = 0;
};
// Read-only ZIP reader. Open once, look up entries repeatedly.
// Each read validates local headers, data boundaries, uncompressed size and CRC-32.
// Entry names are case-sensitive. No ZIP64, multi-disk or encrypted ZIP support.
class ZipArchive {
public:
    bool open(const std::filesystem::path& path, std::string& error);
    const ZipEntry* find(const std::string& name) const;
    bool read(const std::string& name, std::vector<uint8_t>& bytes, std::string& error,
              uint64_t maxBytes = 64ull * 1024 * 1024) const;
    size_t size() const { return entries_.size(); }
    const std::vector<ZipEntry>& entries() const { return entries_; }
private:
    std::filesystem::path path_;
    uint64_t fileSize_ = 0;
    uint64_t directoryOffset_ = 0;
    std::vector<ZipEntry> entries_;
    std::unordered_map<std::string, size_t> index_;
};
} // namespace reluva::stafu
