// SPDX-License-Identifier: GPL-3.0-or-later
// Developer-only manifest extractor. Never touches launcher configuration.
#include "engines/stafu/zip_archive.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include <zlib.h>

namespace {
struct Record {
    std::string destination, archive, entry;
    uint64_t size = 0;
    uint32_t crc = 0;
};
bool csvFields(const std::string& line, std::vector<std::string>& out) {
    out.clear();
    size_t i = 0;
    while (true) {
        std::string value;
        if (i < line.size() && line[i] == '"') {
            ++i;
            bool closed = false;
            while (i < line.size()) {
                if (line[i] == '"') {
                    if (i + 1 < line.size() && line[i + 1] == '"') {
                        value += '"'; i += 2;
                    } else { ++i; closed = true; break; }
                } else value += line[i++];
            }
            if (!closed || (i < line.size() && line[i] != ',')) return false;
        } else {
            while (i < line.size() && line[i] != ',') {
                if (line[i] == '"') return false;
                value += line[i++];
            }
        }
        out.push_back(std::move(value));
        if (i == line.size()) return true;
        if (line[i++] != ',') return false;
    }
}
bool decimal(const std::string& s, uint64_t& n) {
    if (s.empty()) return false;
    n = 0;
    for (char c : s) {
        if (c < '0' || c > '9' || n > (UINT64_MAX - uint64_t(c - '0')) / 10)
            return false;
        n = n * 10 + uint64_t(c - '0');
    }
    return true;
}
bool hex32(const std::string& s, uint32_t& n) {
    if (s.size() != 8) return false;
    n = 0;
    for (char c : s) {
        unsigned digit;
        if (c >= '0' && c <= '9') digit = unsigned(c - '0');
        else if (c >= 'A' && c <= 'F') digit = unsigned(c - 'A' + 10);
        else if (c >= 'a' && c <= 'f') digit = unsigned(c - 'a' + 10);
        else return false;
        n = (n << 4) | digit;
    }
    return true;
}
bool safeRelative(const std::string& name) {
    namespace fs = std::filesystem;
    if (name.empty() || name.find('\\') != std::string::npos ||
        name.find(':') != std::string::npos || name.find('\0') != std::string::npos)
        return false;
    const fs::path path(name);
    if (path.is_absolute() || path.has_root_path()) return false;
    for (const auto& part : path)
        if (part == ".." || part == "." || part.empty()) return false;
    return true;
}
std::string fold(std::string s) {
    for (char& c : s) if (c >= 'A' && c <= 'Z') c = char(c - 'A' + 'a');
    return s;
}
}

int stafuInstallExtract(const char* manifestArg, const char* cdArg, const char* destinationArg) {
    namespace fs = std::filesystem;
    const fs::path destination(destinationArg), cd(cdArg);
    std::error_code ec;
    if (fs::exists(destination, ec) || ec) {
        std::fprintf(stderr, "install-extract: destination must not exist: %s\n", destinationArg);
        return 1;
    }
    std::ifstream input(manifestArg, std::ios::binary);
    if (!input) { std::fprintf(stderr, "install-extract: cannot open manifest\n"); return 1; }
    // Windows PowerShell 5.1 redirects native stdout as UTF-16 LE.
    // Normalize that representation to the ASCII CSV emitted by install-manifest.
    std::string raw((std::istreambuf_iterator<char>(input)),
                    std::istreambuf_iterator<char>());
    std::string normalized;
    if (raw.size() >= 2 &&
        static_cast<unsigned char>(raw[0]) == 0xFF &&
        static_cast<unsigned char>(raw[1]) == 0xFE) {
        if ((raw.size() - 2) % 2 != 0) {
            std::fprintf(stderr, "install-extract: truncated UTF-16 LE manifest\n");
            return 1;
        }
        for (size_t i = 2; i < raw.size(); i += 2) {
            if (raw[i + 1] != '\0' || static_cast<unsigned char>(raw[i]) > 0x7F) {
                std::fprintf(stderr, "install-extract: unsupported non-ASCII UTF-16 manifest\n");
                return 1;
            }
            normalized.push_back(raw[i]);
        }
    } else {
        normalized = std::move(raw);
        if (normalized.size() >= 3 &&
            static_cast<unsigned char>(normalized[0]) == 0xEF &&
            static_cast<unsigned char>(normalized[1]) == 0xBB &&
            static_cast<unsigned char>(normalized[2]) == 0xBF)
            normalized.erase(0, 3);
    }
    std::istringstream manifest(normalized);
    std::string line;
    if (!std::getline(manifest, line)) {
        std::fprintf(stderr, "install-extract: empty manifest\n"); return 1;
    }
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line != "installed_path,status,archive,archive_entry,size,crc32_hex") {
        std::fprintf(stderr, "install-extract: unexpected manifest header\n"); return 1;
    }
    std::vector<Record> records;
    std::map<std::string, bool> destinations;
    std::vector<std::string> fields;
    size_t unresolved = 0, lineNumber = 1;
    while (std::getline(manifest, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (!csvFields(line, fields) || fields.size() != 6 ||
            !safeRelative(fields[0])) {
            std::fprintf(stderr, "install-extract: invalid CSV row %zu\n", lineNumber);
            return 1;
        }
        if (fields[1] != "MATCH") {
            if (fields[1] != "UNMATCHED" && fields[1] != "NAME_DIFF" &&
                fields[1] != "DATA_MATCH" && fields[1] != "AMBIGUOUS") {
                std::fprintf(stderr, "install-extract: unknown status at row %zu\n", lineNumber);
                return 1;
            }
            ++unresolved;
            continue;
        }
        Record r;
        r.destination = fields[0]; r.archive = fields[2]; r.entry = fields[3];
        if ((r.archive != "STTNG.ZIP" && r.archive != "STTNGINS.ZIP") ||
            !safeRelative(r.entry) || !decimal(fields[4], r.size) ||
            !hex32(fields[5], r.crc) || r.size > 64ull * 1024 * 1024 ||
            !destinations.emplace(fold(r.destination), true).second) {
            std::fprintf(stderr, "install-extract: unsafe or invalid MATCH row %zu\n", lineNumber);
            return 1;
        }
        records.push_back(std::move(r));
    }
    if (!manifest.eof() || records.empty()) {
        std::fprintf(stderr, "install-extract: read failure or no MATCH entries\n"); return 1;
    }
    std::map<std::string, reluva::stafu::ZipArchive> archives;
    for (const auto& record : records) {
        if (archives.count(record.archive)) continue;
        std::string error;
        if (!archives[record.archive].open(cd / record.archive, error)) {
            std::fprintf(stderr, "install-extract: %s: %s\n", record.archive.c_str(), error.c_str());
            return 1;
        }
    }
    // Validate every manifest reference before creating the destination.
    for (const auto& record : records) {
        const auto* entry = archives.at(record.archive).find(record.entry);
        if (!entry || entry->uncompressedSize != record.size || entry->crc32 != record.crc) {
            std::fprintf(stderr, "install-extract: archive metadata mismatch for %s\n",
                         record.destination.c_str());
            return 1;
        }
    }
    if (!fs::create_directory(destination, ec) || ec) {
        std::fprintf(stderr, "install-extract: cannot create destination: %s\n", ec.message().c_str());
        return 1;
    }
    size_t written = 0;
    for (const auto& record : records) {
        std::vector<uint8_t> bytes;
        std::string error;
        if (!archives.at(record.archive).read(record.entry, bytes, error) ||
            bytes.size() != record.size ||
            static_cast<uint32_t>(crc32(0, bytes.data(), static_cast<uInt>(bytes.size()))) != record.crc) {
            std::fprintf(stderr, "install-extract: verification failed for %s: %s\n",
                         record.destination.c_str(), error.c_str());
            return 1;
        }
        const fs::path output = destination / fs::path(record.destination);
        fs::create_directories(output.parent_path(), ec);
        if (ec) { std::fprintf(stderr, "install-extract: mkdir failed\n"); return 1; }
        if (fs::exists(output, ec) || ec) {
            std::fprintf(stderr, "install-extract: output already exists: %s\n",
                         output.string().c_str()); return 1;
        }
        std::ofstream file(output, std::ios::binary | std::ios::trunc);
        if (!file) { std::fprintf(stderr, "install-extract: cannot write %s\n", output.string().c_str()); return 1; }
        file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        file.close();
        if (!file) { std::fprintf(stderr, "install-extract: write failed: %s\n", output.string().c_str()); return 1; }
        ++written;
    }
    std::printf("PASS: extracted %zu verified files; skipped %zu unresolved manifest rows.\n",
                written, unresolved);
    std::puts("Developer-only extraction complete. Launcher configuration unchanged.");
    return 0;
}
