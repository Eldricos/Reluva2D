// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only comparison of installed STAFU files with original CD ZIP contents.
#include "engines/stafu/source_inventory.h"
#include "engines/stafu/zip_archive.h"
#include <zlib.h>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace {
std::string fold(std::string s) {
    for (char& c : s) {
        unsigned char u = static_cast<unsigned char>(c);
        if (u >= 'A' && u <= 'Z') c = char(u + ('a' - 'A'));
        if (c == '\\') c = '/';
    }
    return s;
}
struct Fingerprint {
    uint64_t size = 0;
    uint32_t crc = 0;
    bool operator<(const Fingerprint& b) const {
        return size < b.size || (size == b.size && crc < b.crc);
    }
};
bool checksum(const std::filesystem::path& path, uint32_t& value) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    std::vector<char> buffer(65536);
    uLong crc = crc32(0L, Z_NULL, 0);
    while (in) {
        in.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const auto n = in.gcount();
        if (n > 0) crc = crc32(crc, reinterpret_cast<const Bytef*>(buffer.data()),
                              static_cast<uInt>(n));
    }
    if (!in.eof()) return false;
    value = static_cast<uint32_t>(crc);
    return true;
}
struct Match { std::string archive; std::string name; };
}

int stafuInstallAnalyze(const char* installedArg, const char* cdArg) {
    namespace fs = std::filesystem;
    const fs::path installed(installedArg), cd(cdArg);
    const auto inventory = reluva::stafu::inventorySource(
        installed, reluva::stafu::SourceKind::InstalledGame);
    if (!inventory.complete()) {
        for (const auto& e : inventory.errors)
            std::fprintf(stderr, "install-analyze: inventory error: %s\n", e.c_str());
        return 1;
    }
    std::map<std::string, Match> byName;
    std::map<Fingerprint, std::vector<Match>> byFingerprint;
    for (const char* archiveName : {"STTNG.ZIP", "STTNGINS.ZIP"}) {
        fs::path path = cd / archiveName;
        std::error_code ec;
        if (!fs::is_regular_file(path, ec)) {
            std::fprintf(stderr, "install-analyze: missing %s\n", path.string().c_str());
            return 1;
        }
        reluva::stafu::ZipArchive zip;
        std::string error;
        if (!zip.open(path, error)) {
            std::fprintf(stderr, "install-analyze: %s: %s\n", archiveName, error.c_str());
            return 1;
        }
        for (const auto& entry : zip.entries()) {
            const Match match{archiveName, entry.name};
            byName.emplace(fold(entry.name), match);
            byFingerprint[{entry.uncompressedSize, entry.crc32}].push_back(match);
        }
    }
    size_t sameNameSameData = 0, renamedSameData = 0, changedName = 0, unmatched = 0;
    std::printf("STAFU installation comparison (read-only)\n");
    std::printf("Installed: %s\nCD: %s\n", installedArg, cdArg);
    std::printf("Matching method: uncompressed size + CRC32; not proof of installer action.\n\n");
    for (const auto& entry : inventory.files) {
        uint32_t crc = 0;
        if (!checksum(installed / fs::path(entry.relativePath), crc)) {
            std::fprintf(stderr, "install-analyze: cannot checksum %s\n",
                         entry.relativePath.c_str());
            return 1;
        }
        const Fingerprint key{entry.size, crc};
        const auto name = byName.find(fold(entry.relativePath));
        const auto fp = byFingerprint.find(key);
        if (name != byName.end()) {
            bool exact = false;
            if (fp != byFingerprint.end())
                for (const auto& match : fp->second)
                    if (fold(match.name) == fold(entry.relativePath)) exact = true;
            if (exact) {
                ++sameNameSameData;
                std::printf("MATCH       %s  [%s]\n", entry.relativePath.c_str(),
                            name->second.archive.c_str());
            } else {
                ++changedName;
                std::printf("NAME-DIFF   %s  [archive entry same name but different content]\n",
                            entry.relativePath.c_str());
            }
        } else if (fp != byFingerprint.end()) {
            ++renamedSameData;
            const auto& match = fp->second.front();
            std::printf("DATA-MATCH  %s  [%s:%s]\n", entry.relativePath.c_str(),
                        match.archive.c_str(), match.name.c_str());
        } else {
            ++unmatched;
            std::printf("UNMATCHED   %s  [not matched to ZIP contents]\n",
                        entry.relativePath.c_str());
        }
    }
    std::printf("\nSummary: installed=%zu same-name-and-data=%zu renamed-data=%zu "
                "same-name-different-data=%zu unmatched=%zu\n",
                inventory.files.size(), sameNameSameData, renamedSameData,
                changedName, unmatched);
    std::puts("UNMATCHED does not establish a generated file; data may come from "
              "other CD files, transforms, or unsupported sources.");
    return 0;
}
