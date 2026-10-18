// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/zip_archive.h"
#include <zlib.h>
#include <algorithm>
#include <array>
#include <fstream>
#include <limits>

namespace reluva::stafu {
namespace {
uint16_t u16(const uint8_t* p) { return uint16_t(p[0]) | uint16_t(uint16_t(p[1]) << 8); }
uint32_t u32(const uint8_t* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}
bool at(std::ifstream& f, uint64_t offset, uint8_t* out, size_t n) {
    if (offset > uint64_t(std::numeric_limits<std::streamoff>::max())) return false;
    f.clear();
    f.seekg(static_cast<std::streamoff>(offset));
    if (!f) return false;
    f.read(reinterpret_cast<char*>(out), static_cast<std::streamsize>(n));
    return size_t(f.gcount()) == n;
}
}
bool ZipArchive::open(const std::filesystem::path& path, std::string& error) {
    path_.clear(); fileSize_ = 0; directoryOffset_ = 0; entries_.clear(); index_.clear();
    std::error_code ec;
    const uint64_t length = std::filesystem::file_size(path, ec);
    if (ec) { error = ec.message(); return false; }
    std::ifstream f(path, std::ios::binary);
    if (!f) { error = "cannot open ZIP"; return false; }
    const size_t tailLen = size_t(std::min<uint64_t>(length, 65557));
    std::vector<uint8_t> tail(tailLen);
    if (!at(f, length - tailLen, tail.data(), tail.size())) { error = "cannot read ZIP tail"; return false; }
    size_t eocd = tailLen;
    if (tailLen >= 22) for (size_t i = tailLen - 22;; --i) {
        if (u32(tail.data() + i) == 0x06054b50 &&
            i + 22 + u16(tail.data() + i + 20) == tailLen) { eocd = i; break; }
        if (i == 0) break;
    }
    if (eocd == tailLen) { error = "ZIP end record missing"; return false; }
    const uint8_t* e = tail.data() + eocd;
    if (u16(e + 4) || u16(e + 6) || u16(e + 8) != u16(e + 10)) {
        error = "multi-disk ZIP unsupported"; return false;
    }
    const uint16_t count = u16(e + 10);
    const uint32_t dirSize = u32(e + 12), dirOffset = u32(e + 16);
    const uint64_t dirEnd = length - tailLen + eocd;
    if (count == 0xffff || dirSize == 0xffffffffu || dirOffset == 0xffffffffu) {
        error = "ZIP64 unsupported"; return false;
    }
    if (dirSize > 64u * 1024u * 1024u || uint64_t(dirOffset) + dirSize != dirEnd) {
        error = "invalid central directory bounds"; return false;
    }
    std::vector<uint8_t> directory(dirSize);
    if (!at(f, dirOffset, directory.data(), directory.size())) {
        error = "cannot read central directory"; return false;
    }
    std::vector<ZipEntry> parsed;
    std::unordered_map<std::string, size_t> lookup;
    parsed.reserve(count);
    size_t pos = 0;
    for (size_t i = 0; i < count; ++i) {
        if (pos + 46 > directory.size() || u32(directory.data() + pos) != 0x02014b50) {
            error = "invalid central directory record"; return false;
        }
        const uint8_t* p = directory.data() + pos;
        const size_t nameLen = u16(p + 28), extraLen = u16(p + 30), commentLen = u16(p + 32);
        const size_t recordLen = 46 + nameLen + extraLen + commentLen;
        if (recordLen > directory.size() - pos) { error = "truncated central directory record"; return false; }
        if (u16(p + 34)) { error = "multi-disk entry unsupported"; return false; }
        ZipEntry entry;
        entry.name.assign(reinterpret_cast<const char*>(p + 46), nameLen);
        entry.flags = u16(p + 8); entry.method = u16(p + 10);
        entry.crc32 = u32(p + 16); entry.compressedSize = u32(p + 20);
        entry.uncompressedSize = u32(p + 24); entry.localOffset = u32(p + 42);
        if (!lookup.emplace(entry.name, parsed.size()).second) {
            error = "duplicate ZIP entry name"; return false;
        }
        parsed.push_back(std::move(entry));
        pos += recordLen;
    }
    if (pos != directory.size()) { error = "trailing central directory bytes"; return false; }
    path_ = path; fileSize_ = length; directoryOffset_ = dirOffset;
    entries_ = std::move(parsed); index_ = std::move(lookup);
    error.clear();
    return true;
}
const ZipEntry* ZipArchive::find(const std::string& name) const {
    auto it = index_.find(name);
    return it == index_.end() ? nullptr : &entries_[it->second];
}
bool ZipArchive::read(const std::string& name, std::vector<uint8_t>& bytes,
                      std::string& error, uint64_t maxBytes) const {
    bytes.clear();
    const ZipEntry* e = find(name);
    if (!e) { error = "entry not found"; return false; }
    if (e->flags & 1) { error = "encrypted ZIP entry unsupported"; return false; }
    if (e->method != 0 && e->method != 8) { error = "unsupported compression method"; return false; }
    if (e->uncompressedSize > maxBytes) { error = "entry exceeds requested memory limit"; return false; }
    if (e->method == 0 && e->compressedSize != e->uncompressedSize) {
        error = "stored entry sizes disagree"; return false;
    }
    std::ifstream f(path_, std::ios::binary);
    if (!f) { error = "cannot reopen ZIP"; return false; }
    std::array<uint8_t, 30> local{};
    if (!at(f, e->localOffset, local.data(), local.size()) || u32(local.data()) != 0x04034b50) {
        error = "invalid local header"; return false;
    }
    if (u16(local.data() + 6) != e->flags || u16(local.data() + 8) != e->method) {
        error = "local and central header mismatch"; return false;
    }
    const size_t nameLen = u16(local.data() + 26);
    const uint64_t start = uint64_t(e->localOffset) + 30 + nameLen + u16(local.data() + 28);
    if (start > directoryOffset_ || e->compressedSize > directoryOffset_ - start ||
        start > fileSize_) { error = "compressed data outside ZIP data region"; return false; }
    std::vector<uint8_t> localName(nameLen);
    if (!at(f, uint64_t(e->localOffset) + 30, localName.data(), localName.size()) ||
        std::string(reinterpret_cast<const char*>(localName.data()), localName.size()) != e->name) {
        error = "local filename mismatch"; return false;
    }
    std::vector<uint8_t> result;
    result.reserve(e->uncompressedSize);
    std::array<uint8_t, 65536> input{}, output{};
    uint64_t remaining = e->compressedSize, offset = start;
    uLong checksum = crc32(0, Z_NULL, 0);
    z_stream stream{};
    const bool deflated = e->method == 8;
    if (deflated && inflateInit2(&stream, -MAX_WBITS) != Z_OK) {
        error = "DEFLATE initialization failed"; return false;
    }
    bool ok = true, ended = false;
    while (remaining && ok) {
        const size_t n = size_t(std::min<uint64_t>(remaining, input.size()));
        if (!at(f, offset, input.data(), n)) { error = "compressed data read failed"; ok = false; break; }
        offset += n; remaining -= n;
        if (!deflated) {
            result.insert(result.end(), input.data(), input.data() + n);
            checksum = crc32(checksum, input.data(), static_cast<uInt>(n));
            continue;
        }
        stream.next_in = input.data(); stream.avail_in = static_cast<uInt>(n);
        while (stream.avail_in && ok) {
            stream.next_out = output.data(); stream.avail_out = static_cast<uInt>(output.size());
            const int rc = inflate(&stream, Z_NO_FLUSH);
            const size_t got = output.size() - stream.avail_out;
            if (got > e->uncompressedSize - result.size()) {
                error = "uncompressed size exceeded"; ok = false; break;
            }
            result.insert(result.end(), output.data(), output.data() + got);
            checksum = crc32(checksum, output.data(), static_cast<uInt>(got));
            if (rc == Z_STREAM_END) {
                ended = true;
                if (stream.avail_in || remaining) { error = "trailing compressed data"; ok = false; }
                break;
            }
            if (rc != Z_OK || (!got && stream.avail_out != 0)) {
                error = "DEFLATE decode failed"; ok = false; break;
            }
        }
        if (ended) break;
    }
    if (deflated) {
        if (ok && !ended) { error = "truncated DEFLATE stream"; ok = false; }
        inflateEnd(&stream);
    }
    if (ok && result.size() != e->uncompressedSize) { error = "uncompressed size mismatch"; ok = false; }
    if (ok && uint32_t(checksum) != e->crc32) { error = "CRC-32 mismatch"; ok = false; }
    if (!ok) return false;
    bytes = std::move(result);
    error.clear();
    return true;
}
} // namespace reluva::stafu
