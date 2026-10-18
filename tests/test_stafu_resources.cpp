// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {
int failures = 0;
#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); ++failures; } } while (0)
void w16(std::vector<uint8_t>& v, uint16_t n) { v.push_back(uint8_t(n)); v.push_back(uint8_t(n >> 8)); }
void w32(std::vector<uint8_t>& v, uint32_t n) { w16(v, uint16_t(n)); w16(v, uint16_t(n >> 16)); }
void write(const std::filesystem::path& path, const std::vector<uint8_t>& data) {
    std::ofstream f(path, std::ios::binary);
    f.write(reinterpret_cast<const char*>(data.data()), std::streamsize(data.size()));
}
void makeZip(const std::filesystem::path& path, const std::string& name, const std::string& value) {
    std::vector<uint8_t> v;
    const uint32_t crc = [] (const std::string& s) {
        // CRC32 of ASCII strings; test fixture uses known values supplied below.
        uint32_t c = 0xffffffffu;
        for (unsigned char b : s) { c ^= b; for (int i = 0; i < 8; ++i) c = (c >> 1) ^ ((c & 1) ? 0xedb88320u : 0u); }
        return ~c;
    }(value);
    w32(v, 0x04034b50); w16(v, 20); w16(v, 0); w16(v, 0); w16(v, 0); w16(v, 0);
    w32(v, crc); w32(v, uint32_t(value.size())); w32(v, uint32_t(value.size()));
    w16(v, uint16_t(name.size())); w16(v, 0);
    v.insert(v.end(), name.begin(), name.end()); v.insert(v.end(), value.begin(), value.end());
    const uint32_t offset = uint32_t(v.size());
    w32(v, 0x02014b50); w16(v, 20); w16(v, 20); w16(v, 0); w16(v, 0);
    w16(v, 0); w16(v, 0); w32(v, crc); w32(v, uint32_t(value.size()));
    w32(v, uint32_t(value.size())); w16(v, uint16_t(name.size()));
    w16(v, 0); w16(v, 0); w16(v, 0); w16(v, 0); w32(v, 0); w32(v, 0);
    v.insert(v.end(), name.begin(), name.end());
    const uint32_t size = uint32_t(v.size()) - offset;
    w32(v, 0x06054b50); w16(v, 0); w16(v, 0); w16(v, 1); w16(v, 1);
    w32(v, size); w32(v, offset); w16(v, 0);
    write(path, v);
}
std::string str(const std::vector<uint8_t>& v) { return {v.begin(), v.end()}; }
}
int main() {
    namespace fs = std::filesystem;
    namespace st = reluva::stafu;
    const auto root = fs::temp_directory_path() / "reluva_stafu_resource_manager_tests";
    fs::remove_all(root);
    const auto installed = root / "installed", cd = root / "cd";
    fs::create_directories(installed); fs::create_directories(cd);
    makeZip(cd / "STTNG.ZIP", "shared.db", "main");
    makeZip(cd / "STTNGINS.ZIP", "installer.exe", "installer");
    st::ResourceManager resources;
    std::string error;
    std::vector<uint8_t> bytes;
    st::ResourceLocation loc{};
    CHECK(resources.open(installed, cd, error));
    CHECK(resources.locate("shared.db", loc) && loc.origin == st::ResourceOrigin::MainArchive);
    CHECK(resources.read("shared.db", bytes, error) && str(bytes) == "main");
    CHECK(resources.read("installer.exe", bytes, error) && str(bytes) == "installer");
    CHECK(!resources.read("missing", bytes, error));
    CHECK(!resources.read("../shared.db", bytes, error));
    CHECK(!resources.read("shared.db", bytes, error, 2));
    std::ofstream(installed / "SHARED.DB", std::ios::binary) << "loose";
    CHECK(resources.locate("shared.db", loc) && loc.origin == st::ResourceOrigin::InstalledFile);
    CHECK(resources.read("shared.db", bytes, error) && str(bytes) == "loose");
    CHECK(!resources.read("shared.db", bytes, error, 2));
    CHECK(resources.open(installed, {}, error));
    CHECK(resources.read("shared.db", bytes, error) && str(bytes) == "loose");
    CHECK(!resources.read("installer.exe", bytes, error));
    CHECK(!resources.open({}, {}, error));
    fs::remove_all(root);
    return failures ? 1 : 0;
}
