// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

// STAFU-only developer commands. Kept separate from New Worlds tools.
// No decompression or modification of source files.

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

uint16_t le16(const unsigned char* p) {
    return static_cast<uint16_t>(p[0] | (uint16_t(p[1]) << 8));
}
uint32_t le32(const unsigned char* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) |
           (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}
void dump(const unsigned char* data, size_t count, uint64_t offset) {
    for (size_t i = 0; i < count; i += 16) {
        std::printf("%08llx: ", static_cast<unsigned long long>(offset + i));
        for (size_t j = 0; j < 16; ++j)
            if (i + j < count) std::printf("%02x ", unsigned(data[i + j]));
            else std::printf("   ");
        std::printf(" ");
        for (size_t j = 0; j < 16 && i + j < count; ++j) {
            const unsigned char c = data[i + j];
            std::putchar(c >= 32 && c < 127 ? c : '.');
        }
        std::putchar('\n');
    }
}
bool readAt(std::ifstream& in, uint64_t offset, unsigned char* data, size_t count) {
    in.clear();
    in.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    if (!in) return false;
    in.read(reinterpret_cast<char*>(data), static_cast<std::streamsize>(count));
    return static_cast<size_t>(in.gcount()) == count;
}

int probe(const char* name) {
    const std::filesystem::path path(name);
    std::error_code ec;
    const uintmax_t fileSize = std::filesystem::file_size(path, ec);
    if (ec) {
        std::fprintf(stderr, "Cannot stat %s: %s\n", name, ec.message().c_str());
        return 1;
    }
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::fprintf(stderr, "Cannot open %s\n", name);
        return 1;
    }
    std::printf("File: %s\nSize: %llu bytes\n",
                name, static_cast<unsigned long long>(fileSize));
    std::array<unsigned char, 64> head{};
    const size_t headLen = static_cast<size_t>(std::min<uintmax_t>(head.size(), fileSize));
    if (!readAt(in, 0, head.data(), headLen)) return 1;
    std::puts("First bytes:");
    dump(head.data(), headLen, 0);

    if (headLen >= 4 && std::memcmp(head.data(), "PK\x03\x04", 4) == 0) {
        std::puts("Header: ZIP local file header signature (PK 03 04)");
        if (headLen >= 30) {
            std::printf("First ZIP entry: flags=0x%04x method=%u compressed=%u uncompressed=%u name_len=%u extra_len=%u\n",
                        unsigned(le16(head.data() + 6)), unsigned(le16(head.data() + 8)),
                        unsigned(le32(head.data() + 18)), unsigned(le32(head.data() + 22)),
                        unsigned(le16(head.data() + 26)), unsigned(le16(head.data() + 28)));
        }
    } else if (headLen >= 4 && std::memcmp(head.data(), "PK\x05\x06", 4) == 0) {
        std::puts("Header: ZIP end-of-central-directory signature (empty archive candidate)");
    } else {
        std::puts("Header: no standard ZIP signature at offset zero; format unknown");
    }

    // EOCD may have a 65535-byte comment; inspect only the bounded tail.
    constexpr uint64_t kTailWindow = 65535 + 22;
    const size_t tailLen = static_cast<size_t>(std::min<uintmax_t>(fileSize, kTailWindow));
    std::string tail(tailLen, '\0');
    const uint64_t tailOffset = fileSize - tailLen;
    if (!readAt(in, tailOffset, reinterpret_cast<unsigned char*>(tail.data()), tailLen)) return 1;
    bool found = false;
    if (tailLen >= 22) {
        for (size_t i = tailLen - 22;; --i) {
            const auto* p = reinterpret_cast<const unsigned char*>(tail.data() + i);
            if (std::memcmp(p, "PK\x05\x06", 4) == 0 &&
                i + 22 + le16(p + 20) == tailLen) {
                std::printf("ZIP EOCD: offset=%llu disk=%u entries=%u central_size=%u central_offset=%u comment_len=%u\n",
                            static_cast<unsigned long long>(tailOffset + i),
                            unsigned(le16(p + 4)), unsigned(le16(p + 10)),
                            unsigned(le32(p + 12)), unsigned(le32(p + 16)),
                            unsigned(le16(p + 20)));
                found = true;
                break;
            }
            if (i == 0) break;
        }
    }
    if (!found) std::puts("ZIP EOCD: not found in standard tail window");
    std::puts("Last bytes:");
    const size_t display = std::min<size_t>(64, tailLen);
    dump(reinterpret_cast<const unsigned char*>(tail.data() + tailLen - display),
         display, fileSize - display);
    return 0;
}

} // namespace

int stafuCompareComputerFieldLayout(const char* installed, const char* cd);
int stafuInspectComputerGroupSurroundings(const char* installed, const char* cd);
int stafuValidateComputerLayout(const char* installed, const char* cd);
int stafuInspectComputerSourceDisplacement(const char* installed, const char* cd);
int stafuCorrelateComputerPrimary(const char* installed, const char* cd);
int stafuInspectComputerRunReferences(const char* installed, const char* cd);
int stafuInspectComputerRunBoundaries(const char* installed, const char* cd);
int stafuCompareComputerRegions(const char* installed, const char* cd);
int stafuInspectComputerBytePatterns(const char* installed, const char* cd);
int stafuAnalyzeComputerReferences(const char* installed, const char* cd);
int stafuScanComputerArticles(const char* installed, const char* cd);
int stafuInspectComputerHeaders(const char* installed, const char* cd);
int stafuInspectComputerStrings(const char* installed, const char* cd);
int stafuInspectComputerRecords(const char* installed, const char* cd);
int stafuComputerIndex(const char* installed, const char* cd);
int stafuInspectIndex(const char* installed, const char* cd, const char* name);
int stafuInspectReferences(const char* installed, const char* cd, const char* name);
int stafuInspectRanges(const char* installed, const char* cd, const char* name);
int stafuInspectOffsets(const char* installed, const char* cd, const char* name);
int stafuInspectResource(const char* installed, const char* cd, const char* name);
int stafuResourceTest(const char* installed, const char* cd, const char* name);
int stafuInstallAnalyze(const char* installed, const char* cd);
int stafuInstallManifest(const char* installed, const char* cd);
int stafuInstallExtract(const char* manifest, const char* cd, const char* destination);
int stafuInstallValidate(const char* installed, const char* cd);
int stafuInstallStage(const char* installed, const char* cd, const char* destination);
int stafuInstallFromCd(const char* cd, const char* destination);
int stafuArchiveTest(const char* path, const char* entry);
int stafuArchiveList(const char* path);
int stafuArchiveCompare(const char* zipPath, const char* dirPath);

int stafuToolMain(int argc, char** argv) {
    if (argc == 5 && std::strcmp(argv[2], "computer-field-layout") == 0)
        return stafuCompareComputerFieldLayout(argv[3], argv[4]);
    if (argc == 5 && std::strcmp(argv[2], "computer-group-surroundings") == 0)
        return stafuInspectComputerGroupSurroundings(argv[3], argv[4]);
    if (argc == 5 && std::strcmp(argv[2], "computer-layout-validate") == 0)
        return stafuValidateComputerLayout(argv[3], argv[4]);
    if (argc == 5 && std::strcmp(argv[2], "computer-source-displacement") == 0)
        return stafuInspectComputerSourceDisplacement(argv[3], argv[4]);
    if (argc == 5 && std::strcmp(argv[2], "computer-primary-correlation") == 0)
        return stafuCorrelateComputerPrimary(argv[3], argv[4]);
    if (argc == 5 && std::strcmp(argv[2], "computer-run-references") == 0)
        return stafuInspectComputerRunReferences(argv[3], argv[4]);
    if (argc == 5 && std::strcmp(argv[2], "computer-run-boundaries") == 0)
        return stafuInspectComputerRunBoundaries(argv[3], argv[4]);
    if (argc == 5 && std::strcmp(argv[2], "computer-region-compare") == 0)
        return stafuCompareComputerRegions(argv[3], argv[4]);
    if (argc == 5 && std::strcmp(argv[2], "computer-byte-patterns") == 0)
        return stafuInspectComputerBytePatterns(argv[3], argv[4]);
    if (argc == 5 && std::strcmp(argv[2], "computer-reference-layout") == 0)
        return stafuAnalyzeComputerReferences(argv[3], argv[4]);
    if (argc == 5 && std::strcmp(argv[2], "computer-article-scan") == 0)
        return stafuScanComputerArticles(argv[3], argv[4]);
    if (argc == 5 && std::strcmp(argv[2], "computer-header-compare") == 0)
        return stafuInspectComputerHeaders(argv[3], argv[4]);
    if (argc == 5 && std::strcmp(argv[2], "computer-string-inspect") == 0)
        return stafuInspectComputerStrings(argv[3], argv[4]);
    if (argc == 5 && std::strcmp(argv[2], "computer-record-inspect") == 0)
        return stafuInspectComputerRecords(argv[3], argv[4]);
    if (argc == 5 && std::strcmp(argv[2], "computer-index") == 0)
        return stafuComputerIndex(argv[3], argv[4]);
    if (argc == 6 && std::strcmp(argv[2], "index-inspect") == 0)
        return stafuInspectIndex(argv[3], argv[4], argv[5]);
    if (argc == 6 && std::strcmp(argv[2], "reference-inspect") == 0)
        return stafuInspectReferences(argv[3], argv[4], argv[5]);
    if (argc == 6 && std::strcmp(argv[2], "range-inspect") == 0)
        return stafuInspectRanges(argv[3], argv[4], argv[5]);
    if (argc == 6 && std::strcmp(argv[2], "offset-inspect") == 0)
        return stafuInspectOffsets(argv[3], argv[4], argv[5]);
    if (argc == 6 && std::strcmp(argv[2], "resource-inspect") == 0)
        return stafuInspectResource(argv[3], argv[4], argv[5]);
    if (argc == 5 && std::strcmp(argv[2], "install-from-cd") == 0)
        return stafuInstallFromCd(argv[3], argv[4]);
    if (argc == 6 && std::strcmp(argv[2], "install-stage") == 0)
        return stafuInstallStage(argv[3], argv[4], argv[5]);
    if (argc == 4 && std::strcmp(argv[2], "install-validate") == 0)
        return stafuInstallValidate(argv[3], nullptr);
    if (argc == 5 && std::strcmp(argv[2], "install-validate") == 0)
        return stafuInstallValidate(argv[3], argv[4]);
    if (argc == 6 && std::strcmp(argv[2], "install-extract") == 0)
        return stafuInstallExtract(argv[3], argv[4], argv[5]);
    if (argc == 5 && std::strcmp(argv[2], "install-manifest") == 0)
        return stafuInstallManifest(argv[3], argv[4]);
    if (argc == 5 && std::strcmp(argv[2], "install-analyze") == 0)
        return stafuInstallAnalyze(argv[3], argv[4]);
    if (argc == 6 && std::strcmp(argv[2], "resource-test") == 0)
        return stafuResourceTest(argv[3], argv[4], argv[5]);
    if (argc == 5 && std::strcmp(argv[2], "archive-test") == 0)
        return stafuArchiveTest(argv[3], argv[4]);
    if (argc == 4 && std::strcmp(argv[2], "archive-list") == 0)
        return stafuArchiveList(argv[3]);
    if (argc == 5 && std::strcmp(argv[2], "archive-compare") == 0)
        return stafuArchiveCompare(argv[3], argv[4]);
    if (argc == 4 && std::strcmp(argv[2], "archive-probe") == 0)
        return probe(argv[3]);
    std::fprintf(stderr,
        "STAFU usage:\n"
        "  reluva-tool -stafu install-from-cd <cd> <new-destination>\n"
        "  reluva-tool -stafu install-stage <installed> <cd> <new-destination>\n"
        "  reluva-tool -stafu install-validate <installed> [cd]\n"
        "  reluva-tool -stafu install-extract <manifest.csv> <cd> <new-destination>\n"
        "  reluva-tool -stafu install-manifest <installed> <cd>\n"
        "  reluva-tool -stafu install-analyze <installed> <cd>\n"
        "  reluva-tool -stafu resource-test <installed> <cd> <entry>\n"
        "  reluva-tool -stafu resource-inspect <installed> <cd> <entry>\n"
        "  reluva-tool -stafu offset-inspect <installed> <cd> <entry>\n"
        "  reluva-tool -stafu range-inspect <installed> <cd> <entry>\n"
        "  reluva-tool -stafu reference-inspect <installed> <cd> <entry>\n"
        "  reluva-tool -stafu index-inspect <installed> <cd> <entry>\n"
        "  reluva-tool -stafu computer-index <installed> <cd>\n"
        "  reluva-tool -stafu computer-record-inspect <installed> <cd>\n"
        "  reluva-tool -stafu computer-string-inspect <installed> <cd>\n"
        "  reluva-tool -stafu computer-header-compare <installed> <cd>\n"
        "  reluva-tool -stafu computer-article-scan <installed> <cd>\n"
        "  reluva-tool -stafu computer-reference-layout <installed> <cd>\n"
        "  reluva-tool -stafu computer-byte-patterns <installed> <cd>\n"
        "  reluva-tool -stafu computer-region-compare <installed> <cd>\n"
        "  reluva-tool -stafu computer-run-boundaries <installed> <cd>\n"
        "  reluva-tool -stafu computer-run-references <installed> <cd>\n"
        "  reluva-tool -stafu computer-primary-correlation <installed> <cd>\n"
        "  reluva-tool -stafu computer-source-displacement <installed> <cd>\n"
        "  reluva-tool -stafu computer-layout-validate <installed> <cd>\n"
        "  reluva-tool -stafu computer-group-surroundings <installed> <cd>\n"
        "  reluva-tool -stafu computer-field-layout <installed> <cd>\n"
        "  reluva-tool -stafu archive-probe <file>\n"
        "  reluva-tool -stafu archive-test <archive.zip> <entry>\n"
        "  reluva-tool -stafu archive-list <file>\n"
        "  reluva-tool -stafu archive-compare <archive.zip> <archive.dir>\n");
    return 2;
}
