// SPDX-License-Identifier: GPL-3.0-or-later
// Reference installation manifest, written to stdout only.
// Source trees and CD archives are never modified.
#include "engines/stafu/source_inventory.h"
#include "engines/stafu/zip_archive.h"
#include <zlib.h>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace {
struct Key {
    uint64_t size;
    uint32_t crc;
    bool operator<(const Key& rhs) const {
        return size != rhs.size ? size < rhs.size : crc < rhs.crc;
    }
};
struct Source {
    std::string archive;
    std::string entry;
    Key key;
};
std::string fold(std::string value) {
    for (char& c : value) {
        if (c == '\\') c = '/';
        if (c >= 'A' && c <= 'Z') c = char(c + ('a' - 'A'));
    }
    return value;
}
bool checksum(const std::filesystem::path& file, uint32_t& result) {
    std::ifstream stream(file, std::ios::binary);
    if (!stream) return false;
    char buffer[65536];
    uLong crc = crc32(0, Z_NULL, 0);
    while (stream) {
        stream.read(buffer, sizeof buffer);
        const auto n = stream.gcount();
        if (n > 0)
            crc = crc32(crc, reinterpret_cast<const Bytef*>(buffer), static_cast<uInt>(n));
    }
    if (!stream.eof()) return false;
    result = static_cast<uint32_t>(crc);
    return true;
}
void csv(const std::string& value) {
    std::putchar('"');
    for (char c : value) {
        if (c == '"') std::putchar('"');
        std::putchar(c);
    }
    std::putchar('"');
}
void row(const std::string& file, const std::string& status,
         const std::string& archive, const std::string& entry,
         uint64_t size, uint32_t crc) {
    csv(file); std::putchar(',');
    csv(status); std::putchar(',');
    csv(archive); std::putchar(',');
    csv(entry);
    std::printf(",%llu,%08X\n", static_cast<unsigned long long>(size), crc);
}
}

int stafuInstallManifest(const char* installedArg, const char* cdArg) {
    namespace fs = std::filesystem;
    const fs::path installed(installedArg), cd(cdArg);
    const auto inventory = reluva::stafu::inventorySource(
        installed, reluva::stafu::SourceKind::InstalledGame);
    if (!inventory.complete()) {
        for (const auto& error : inventory.errors)
            std::fprintf(stderr, "install-manifest: %s\n", error.c_str());
        return 1;
    }
    std::map<std::string, std::vector<Source>> byName;
    std::map<Key, std::vector<Source>> byData;
    for (const char* archiveName : {"STTNG.ZIP", "STTNGINS.ZIP"}) {
        const fs::path path = cd / archiveName;
        reluva::stafu::ZipArchive archive;
        std::string error;
        if (!archive.open(path, error)) {
            std::fprintf(stderr, "install-manifest: %s: %s\n", path.string().c_str(),
                         error.c_str());
            return 1;
        }
        for (const auto& entry : archive.entries()) {
            if (entry.name.empty() || entry.name.back() == '/') continue;
            Source source{archiveName, entry.name, {entry.uncompressedSize, entry.crc32}};
            byName[fold(entry.name)].push_back(source);
            byData[source.key].push_back(source);
        }
    }

    // CSV format v1: only MATCH rows are verified candidates for future extraction.
    // Other statuses must never be silently treated as installable resources.
    std::puts("installed_path,status,archive,archive_entry,size,crc32_hex");
    size_t matches = 0, unresolved = 0;
    for (const auto& file : inventory.files) {
        uint32_t crc = 0;
        if (!checksum(installed / fs::path(file.relativePath), crc)) {
            std::fprintf(stderr, "install-manifest: cannot read %s\n",
                         file.relativePath.c_str());
            return 1;
        }
        const Key key{file.size, crc};
        const auto name = byName.find(fold(file.relativePath));
        const auto data = byData.find(key);
        std::vector<Source> exact;
        if (name != byName.end())
            for (const auto& candidate : name->second)
                if (candidate.key.size == key.size && candidate.key.crc == key.crc)
                    exact.push_back(candidate);
        if (exact.size() == 1) {
            const auto& src = exact.front();
            row(file.relativePath, "MATCH", src.archive, src.entry, key.size, key.crc);
            ++matches;
        } else if (exact.size() > 1) {
            row(file.relativePath, "AMBIGUOUS", "", "", key.size, key.crc);
            ++unresolved;
        } else if (name != byName.end()) {
            row(file.relativePath, "NAME_DIFF", "", "", key.size, key.crc);
            ++unresolved;
        } else if (data != byData.end()) {
            row(file.relativePath, "DATA_MATCH", "", "", key.size, key.crc);
            ++unresolved;
        } else {
            row(file.relativePath, "UNMATCHED", "", "", key.size, key.crc);
            ++unresolved;
        }
    }
    std::fprintf(stderr, "install-manifest: files=%zu verified=%zu unresolved=%zu\n",
                 inventory.files.size(), matches, unresolved);
    return 0;
}
