// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/zip_archive.h"
#include <zlib.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {
int failures = 0;
#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); ++failures; } } while (0)
void w16(std::vector<uint8_t>& v, uint16_t x) { v.push_back(uint8_t(x)); v.push_back(uint8_t(x >> 8)); }
void w32(std::vector<uint8_t>& v, uint32_t x) { w16(v, uint16_t(x)); w16(v, uint16_t(x >> 16)); }
struct Fixture { std::string name, text; uint16_t method; };
std::vector<uint8_t> deflateRaw(const std::string& text) {
    z_stream z{};
    if (deflateInit2(&z, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) return {};
    std::vector<uint8_t> out(compressBound(uLong(text.size())));
    z.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(text.data()));
    z.avail_in = uInt(text.size());
    z.next_out = out.data(); z.avail_out = uInt(out.size());
    const int rc = deflate(&z, Z_FINISH);
    if (rc != Z_STREAM_END) out.clear();
    else out.resize(z.total_out);
    deflateEnd(&z);
    return out;
}
std::vector<uint8_t> makeZip(const std::vector<Fixture>& fixtures) {
    std::vector<uint8_t> file, directory;
    for (const auto& x : fixtures) {
        const uint32_t offset = uint32_t(file.size());
        const std::vector<uint8_t> packed = x.method == 8 ? deflateRaw(x.text) :
            std::vector<uint8_t>(x.text.begin(), x.text.end());
        const uint32_t crc = uint32_t(crc32(0, reinterpret_cast<const Bytef*>(x.text.data()), uInt(x.text.size())));
        w32(file, 0x04034b50); w16(file, 20); w16(file, 0); w16(file, x.method);
        w16(file, 0); w16(file, 0); w32(file, crc); w32(file, uint32_t(packed.size()));
        w32(file, uint32_t(x.text.size())); w16(file, uint16_t(x.name.size())); w16(file, 0);
        file.insert(file.end(), x.name.begin(), x.name.end());
        file.insert(file.end(), packed.begin(), packed.end());
        w32(directory, 0x02014b50); w16(directory, 20); w16(directory, 20);
        w16(directory, 0); w16(directory, x.method); w16(directory, 0); w16(directory, 0);
        w32(directory, crc); w32(directory, uint32_t(packed.size()));
        w32(directory, uint32_t(x.text.size())); w16(directory, uint16_t(x.name.size()));
        w16(directory, 0); w16(directory, 0); w16(directory, 0); w16(directory, 0);
        w32(directory, 0); w32(directory, offset);
        directory.insert(directory.end(), x.name.begin(), x.name.end());
    }
    const uint32_t offset = uint32_t(file.size()), size = uint32_t(directory.size());
    file.insert(file.end(), directory.begin(), directory.end());
    w32(file, 0x06054b50); w16(file, 0); w16(file, 0);
    w16(file, uint16_t(fixtures.size())); w16(file, uint16_t(fixtures.size()));
    w32(file, size); w32(file, offset); w16(file, 0);
    return file;
}
void write(const std::filesystem::path& p, const std::vector<uint8_t>& bytes) {
    std::ofstream f(p, std::ios::binary);
    f.write(reinterpret_cast<const char*>(bytes.data()), std::streamsize(bytes.size()));
}
}
int main() {
    using reluva::stafu::ZipArchive;
    const auto path = std::filesystem::temp_directory_path() / "reluva_stafu_zip_test.zip";
    const auto original = makeZip({{"stored.txt", "hello", 0}, {"deflated.txt", std::string(100000, 'Z'), 8},
                                   {"empty.txt", "", 0}});
    write(path, original);
    ZipArchive zip;
    std::string error;
    std::vector<uint8_t> bytes;
    CHECK(zip.open(path, error));
    CHECK(zip.size() == 3);
    CHECK(zip.find("stored.txt") != nullptr);
    CHECK(zip.find("missing") == nullptr);
    CHECK(zip.read("stored.txt", bytes, error));
    CHECK(std::string(bytes.begin(), bytes.end()) == "hello");
    CHECK(zip.read("deflated.txt", bytes, error));
    CHECK(bytes.size() == 100000);
    CHECK(zip.read("empty.txt", bytes, error));
    CHECK(bytes.empty());
    CHECK(!zip.read("missing", bytes, error));
    CHECK(!zip.read("deflated.txt", bytes, error, 1024));
    auto damaged = original;
    damaged[40] ^= 1; // stored payload byte -> CRC mismatch
    write(path, damaged);
    CHECK(!zip.read("stored.txt", bytes, error));
    damaged = original;
    damaged.resize(12); // invalid archive
    write(path, damaged);
    CHECK(!zip.open(path, error));
    CHECK(zip.size() == 0);
    std::filesystem::remove(path);
    return failures ? 1 : 0;
}
