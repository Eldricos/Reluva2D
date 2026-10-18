// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors
// Read-only STAFU ZIP entry verifier: stored (0) and raw DEFLATE (8).

#include <zlib.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

namespace {
uint16_t rd16(const unsigned char* p) { return uint16_t(p[0]) | uint16_t(uint16_t(p[1]) << 8); }
uint32_t rd32(const unsigned char* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}
bool readAt(std::ifstream& f, uint64_t offset, unsigned char* p, size_t n) {
    if (offset > uint64_t(std::numeric_limits<std::streamoff>::max())) return false;
    f.clear();
    f.seekg(static_cast<std::streamoff>(offset));
    if (!f) return false;
    f.read(reinterpret_cast<char*>(p), static_cast<std::streamsize>(n));
    return size_t(f.gcount()) == n;
}
struct Entry {
    std::string name;
    uint16_t method = 0, flags = 0;
    uint32_t crc = 0, compressed = 0, uncompressed = 0, local = 0;
};
bool locate(std::ifstream& f, uint64_t length, const std::string& wanted,
            Entry& result, std::string& error) {
    const size_t tailLen = size_t(std::min<uint64_t>(length, 65557));
    std::vector<unsigned char> tail(tailLen);
    if (!readAt(f, length - tailLen, tail.data(), tailLen)) {
        error = "cannot read ZIP tail"; return false;
    }
    size_t eocd = tailLen;
    if (tailLen >= 22) for (size_t i = tailLen - 22;; --i) {
        if (rd32(tail.data() + i) == 0x06054b50 &&
            i + 22 + rd16(tail.data() + i + 20) == tailLen) { eocd = i; break; }
        if (i == 0) break;
    }
    if (eocd == tailLen) { error = "ZIP end record missing"; return false; }
    const unsigned char* e = tail.data() + eocd;
    if (rd16(e + 4) || rd16(e + 6) || rd16(e + 8) != rd16(e + 10)) {
        error = "multi-disk ZIP unsupported"; return false;
    }
    const uint16_t count = rd16(e + 10);
    const uint32_t size = rd32(e + 12), offset = rd32(e + 16);
    if (count == 0xffff || size == 0xffffffffu || offset == 0xffffffffu) {
        error = "ZIP64 unsupported"; return false;
    }
    if (size > 64u * 1024u * 1024u || uint64_t(offset) + size != length - tailLen + eocd) {
        error = "invalid central directory bounds"; return false;
    }
    std::vector<unsigned char> dir(size);
    if (!readAt(f, offset, dir.data(), dir.size())) {
        error = "cannot read central directory"; return false;
    }
    size_t pos = 0;
    bool found = false;
    for (size_t i = 0; i < count; ++i) {
        if (pos + 46 > dir.size() || rd32(dir.data() + pos) != 0x02014b50) {
            error = "invalid central directory record"; return false;
        }
        const unsigned char* p = dir.data() + pos;
        const size_t nameLen = rd16(p + 28), extraLen = rd16(p + 30), commentLen = rd16(p + 32);
        const size_t record = 46 + nameLen + extraLen + commentLen;
        if (record > dir.size() - pos) {
            error = "truncated central directory record"; return false;
        }
        const std::string name(reinterpret_cast<const char*>(p + 46), nameLen);
        if (name == wanted) {
            if (found) { error = "ambiguous duplicate entry name"; return false; }
            found = true;
            result = {name, rd16(p + 10), rd16(p + 8), rd32(p + 16),
                      rd32(p + 20), rd32(p + 24), rd32(p + 42)};
            if (rd16(p + 34) != 0) { error = "entry spans multiple disks"; return false; }
        }
        pos += record;
    }
    if (pos != dir.size()) { error = "unexpected trailing central directory data"; return false; }
    if (!found) { error = "entry not found (names are case-sensitive)"; return false; }
    return true;
}
bool verify(std::ifstream& f, uint64_t length, const Entry& e, std::string& error) {
    if (e.flags & 0x0001u) { error = "encrypted entries unsupported"; return false; }
    if (e.method != 0 && e.method != 8) { error = "unsupported compression method"; return false; }
    std::array<unsigned char, 30> header{};
    if (!readAt(f, e.local, header.data(), header.size()) || rd32(header.data()) != 0x04034b50) {
        error = "invalid local header"; return false;
    }
    if (rd16(header.data() + 8) != e.method || rd16(header.data() + 6) != e.flags) {
        error = "local and central header mismatch"; return false;
    }
    const uint64_t start = uint64_t(e.local) + 30 + rd16(header.data() + 26) +
                           rd16(header.data() + 28);
    if (start > length || e.compressed > length - start) {
        error = "compressed data exceeds file bounds"; return false;
    }
    // ZIP method 8 uses raw DEFLATE, not zlib-wrapped data.
    z_stream stream{};
    if (e.method == 8 && inflateInit2(&stream, -MAX_WBITS) != Z_OK) {
        error = "cannot initialize DEFLATE"; return false;
    }
    std::array<unsigned char, 65536> input{}, output{};
    uint64_t remaining = e.compressed, offset = start, produced = 0;
    uLong crc = crc32(0L, Z_NULL, 0);
    bool ended = false, ok = true;
    while (remaining > 0 && ok) {
        const size_t n = size_t(std::min<uint64_t>(remaining, input.size()));
        if (!readAt(f, offset, input.data(), n)) { error = "compressed data read failed"; ok = false; break; }
        offset += n;
        remaining -= n;
        if (e.method == 0) {
            crc = crc32(crc, input.data(), static_cast<uInt>(n));
            produced += n;
            continue;
        }
        stream.next_in = input.data();
        stream.avail_in = static_cast<uInt>(n);
        while (stream.avail_in && ok) {
            stream.next_out = output.data();
            stream.avail_out = static_cast<uInt>(output.size());
            const int rc = inflate(&stream, Z_NO_FLUSH);
            const size_t got = output.size() - stream.avail_out;
            crc = crc32(crc, output.data(), static_cast<uInt>(got));
            produced += got;
            if (produced > e.uncompressed) { error = "uncompressed size exceeded"; ok = false; break; }
            if (rc == Z_STREAM_END) {
                ended = true;
                if (stream.avail_in || remaining) { error = "trailing compressed data"; ok = false; }
                break;
            }
            if (rc != Z_OK || (got == 0 && stream.avail_out != 0)) {
                error = "DEFLATE decoding failed"; ok = false; break;
            }
        }
        if (ended) break;
    }
    if (e.method == 8) {
        if (ok && !ended) {
            // Empty DEFLATE streams are still encoded and should have reached STREAM_END.
            error = "DEFLATE stream ended prematurely"; ok = false;
        }
        inflateEnd(&stream);
    }
    if (ok && produced != e.uncompressed) { error = "uncompressed size mismatch"; ok = false; }
    if (ok && uint32_t(crc) != e.crc) { error = "CRC-32 mismatch"; ok = false; }
    if (ok) std::printf("PASS: %s  method=%u  compressed=%u  uncompressed=%u  crc32=%08x\n",
                         e.name.c_str(), unsigned(e.method), unsigned(e.compressed),
                         unsigned(e.uncompressed), unsigned(e.crc));
    return ok;
}
} // namespace

int stafuArchiveTest(const char* path, const char* name) {
    std::error_code ec;
    const uintmax_t length = std::filesystem::file_size(path, ec);
    if (ec) { std::fprintf(stderr, "archive-test: %s\n", ec.message().c_str()); return 1; }
    std::ifstream f(path, std::ios::binary);
    if (!f) { std::fprintf(stderr, "archive-test: cannot open %s\n", path); return 1; }
    Entry e;
    std::string error;
    if (!locate(f, length, name, e, error) || !verify(f, length, e, error)) {
        std::fprintf(stderr, "archive-test: FAIL: %s: %s\n", name, error.c_str());
        return 1;
    }
    return 0;
}
