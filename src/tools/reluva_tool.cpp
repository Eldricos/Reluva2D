// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

// reluva-tool: developer utilities for examining game data files.
//
//   reluva-tool hexdump <file> [offset] [length]   offset may be hex (0x..) or
//                                                  negative (from the end)
//   reluva-tool cat-probe <file> [-q]              check a .cat file against the documented
//                                                  layout and work out unknown entry kinds
//   reluva-tool cat-list <file>                    list a .cat file's contents
//   reluva-tool cat-extract <file> <dir> [filter]  extract files whose name contains filter
//   reluva-tool stb-dump <file.stb>                print a string table
//   reluva-tool tex-survey <file.cat>...           group every texture by header layout
//   reluva-tool tex-keys <file.cat>...             compare each colour key with the image corners
//   reluva-tool chunks <file>                      try to read any file as a (u16 tag, u32 size) chunk tree
//   reluva-tool words <file> [offset] [count]      show 4-byte values as hex, integer and float
//   reluva-tool table <file> [per row] [offset]    show 4-byte values compactly, N per row
//   reluva-tool geo-info <file.geo> [out.obj]      decode a model; optionally export it as OBJ
//   reluva-tool geo-survey <file.cat>...           decode every model, group failures by reason
//   reluva-tool geo-textures [-a] <file.cat>...    show which textures each model gets (by default
//                                                  only models with too few, with similar names)
//   reluva-tool gmf-info <file.gmf> [record]       walk a map's records, summarise them, and dump
//                                                  one record (or where the walk stops)
//   reluva-tool geo-faces <file.geo>               walk a model's faces, showing each header and
//                                                  corner, and dump the bytes where the walk stops
//   reluva-tool lbp-info <file> [out]              decode a texture, show its key and colours,
//                                                  write <out>.bmp and <out>_alpha.bmp

#include "engines/newworlds/cat_archive.h"
#include "engines/newworlds/string_table.h"
#include "engines/newworlds/texture.h"
#include "engines/newworlds/geo.h"
#include "engines/newworlds/gmf.h"
#include "engines/newworlds/model_textures.h"

#include <map>
#include <memory>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

using Bytes = std::vector<uint8_t>;

bool readFile(const char* path, Bytes& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::fprintf(stderr, "cannot open %s\n", path);
        return false;
    }
    out.assign(std::istreambuf_iterator<char>(in), {});
    return true;
}

uint16_t rd16(const Bytes& d, size_t o) { return o + 2 <= d.size() ? uint16_t(d[o] | d[o + 1] << 8) : 0; }
uint32_t rd32(const Bytes& d, size_t o) {
    return o + 4 <= d.size() ? uint32_t(d[o]) | uint32_t(d[o + 1]) << 8 | uint32_t(d[o + 2]) << 16 |
                                   uint32_t(d[o + 3]) << 24
                             : 0;
}

void hexdump(const Bytes& d, size_t off, size_t len) {
    size_t end = std::min(d.size(), off + len);
    for (size_t row = off; row < end; row += 16) {
        std::printf("%08zx: ", row);
        for (size_t i = 0; i < 16; ++i) {
            if (row + i < end)
                std::printf("%02x", d[row + i]);
            else
                std::printf("  ");
            if (i % 2)
                std::printf(" ");
        }
        std::printf(" ");
        for (size_t i = 0; i < 16 && row + i < end; ++i) {
            uint8_t c = d[row + i];
            std::putchar(c >= 32 && c < 127 ? c : '.');
        }
        std::printf("\n");
    }
}

// Unix time -> "YYYY-MM-DD HH:MM" (UTC), without platform time functions.
std::string formatTime(uint32_t t) {
    int64_t days = t / 86400, secs = t % 86400;
    int64_t z = days + 719468, era = z / 146097, doe = z - era * 146097;
    int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100), mp = (5 * doy + 2) / 153;
    int64_t day = doy - (153 * mp + 2) / 5 + 1, month = mp < 10 ? mp + 3 : mp - 9;
    int64_t year = yoe + era * 400 + (month <= 2);
    char buf[32];
    std::snprintf(buf, sizeof buf, "%04d-%02d-%02d %02d:%02d", int(year), int(month), int(day),
                  int(secs / 3600), int(secs / 60 % 60));
    return buf;
}

long long parseNumber(const char* s) { return std::strtoll(s, nullptr, 0); }

int cmdHexdump(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: reluva-tool hexdump <file> [offset] [length]\n");
        return 2;
    }
    Bytes d;
    if (!readFile(argv[2], d))
        return 1;
    long long off = argc > 3 ? parseNumber(argv[3]) : 0;
    if (off < 0)
        off += static_cast<long long>(d.size());
    size_t len = argc > 4 ? size_t(parseNumber(argv[4])) : 256;
    hexdump(d, size_t(std::max(0LL, off)), len);
    return 0;
}

// Current understanding (see docs/formats/newworlds/cat.md): the file is a
// tree of chunks, each a u16 tag + u32 size + payload.
//   0x00 tag 1, size = rest of file
//   0x06 tag 5, size = table of contents: u32 count, then one entry per item
//        file entries are 334 bytes, folder entries 264
//   then the items themselves, each a chunk; folders (tag 0x29) contain the
//   chunks of the entries that follow them
// An entry's offset + the table size gives the absolute position of its chunk.
constexpr size_t kPathLen = 260;
constexpr size_t kFileEntrySize = 334;
constexpr size_t kFolderEntrySize = 264;
constexpr uint16_t kFolderTag = 0x29;

std::string readName(const Bytes& d, size_t p) {
    std::string n;
    for (size_t k = p; k < p + kPathLen && k < d.size() && d[k]; ++k) {
        if (d[k] < 32 || d[k] >= 127)
            return "<binary>";
        n += char(d[k]);
    }
    return n;
}

std::string extensionOf(const std::string& name) {
    size_t dot = name.rfind('.');
    return dot == std::string::npos || name.find('\\', dot) != std::string::npos ? "" : name.substr(dot);
}

// Rules for walking the table. Start with what is known; learn the rest.
struct TagRule {
    uint16_t tag;
    size_t entrySize;
    bool container;
    bool learned;
};

struct Walker {
    const Bytes& d;
    uint32_t tocSize;
    uint8_t archiveByte;
    std::vector<TagRule> rules;

    TagRule rule(uint16_t tag) const {
        for (const TagRule& r : rules)
            if (r.tag == tag)
                return r;
        return tag == kFolderTag ? TagRule{tag, kFolderEntrySize, true, false} : TagRule{tag, kFileEntrySize, false, false};
    }
    void learn(const TagRule& r) {
        for (TagRule& x : rules)
            if (x.tag == r.tag) {
                x = r;
                return;
            }
        rules.push_back(r);
    }
    size_t chunkOf(size_t entryPos) const { return size_t(rd32(d, entryPos)) + tocSize; }
    bool validEntry(size_t q, int prevIndex, size_t expectedChunk) const {
        return q + 8 <= d.size() && int(rd16(d, q + 4)) > prevIndex && d[q + 6] == 0 && d[q + 7] == archiveByte &&
               chunkOf(q) == expectedChunk;
    }
};

int cmdCatProbe(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: reluva-tool cat-probe <file.cat> [-q]\n");
        return 2;
    }
    Bytes d;
    if (!readFile(argv[2], d))
        return 1;
    const bool quiet = argc > 3 && std::strcmp(argv[3], "-q") == 0;

    const uint32_t tocSize = rd32(d, 8);
    const uint32_t count = rd32(d, 0x0c);
    const size_t tocEnd = 0x0c + size_t(tocSize);
    if (!quiet) {
        std::printf("file size %zu; root chunk tag %u size %u (%s)\n", d.size(), rd16(d, 0), rd32(d, 2),
                    rd32(d, 2) + 6 == d.size() ? "matches" : "MISMATCH");
        std::printf("table chunk tag %u size %u, ends at 0x%zx; %u entries\n\n", rd16(d, 6), tocSize, tocEnd, count);
        std::printf("%4s %6s %9s %4s %8s  %-16s %s\n", "idx", "at", "chunk at", "tag", "size", "modified (UTC)",
                    "name (indented by folder)");
    }

    Walker w{d, tocSize, d.size() > 0x17 ? d[0x17] : uint8_t(0), {}};
    std::vector<size_t> folderEnds;
    struct TagStat {
        uint16_t tag;
        unsigned count = 0, sizeMismatches = 0;
        std::vector<std::string> extensions;
    };
    std::vector<TagStat> stats;
    auto statFor = [&](uint16_t tag) -> TagStat& {
        for (TagStat& s : stats)
            if (s.tag == tag)
                return s;
        stats.push_back(TagStat{tag, 0, 0, {}});
        return stats.back();
    };

    size_t pos = 0x10, expectedChunk = tocEnd;
    size_t prevPos = 0, prevChunk = 0;
    uint32_t prevSize = 0;
    uint16_t prevTag = 0;
    int prevIndex = -1;
    unsigned entries = 0, indexGaps = 0;
    for (unsigned i = 0; i < count; ++i) {
        if (!w.validEntry(pos, prevIndex, expectedChunk)) {
            // The previous entry's rule was wrong. Try the other kind (file /
            // container) and any entry size, and keep the one that lets the
            // chain continue.
            // Take the nearest entry that fits either explanation: a farther
            // match can be a later entry that skips a container's children.
            bool recovered = false;
            const TagRule old = w.rule(prevTag);
            size_t bestQ = 0, bestWant = 0;
            bool bestContainer = false;
            for (int attempt = 0; attempt < 2 && i > 0; ++attempt) {
                const bool container = attempt == 0 ? old.container : !old.container;
                const size_t want = container ? prevChunk + 6 : prevChunk + 6 + prevSize;
                for (size_t q = prevPos + 8; q < prevPos + 4096 && q + 8 <= d.size() && (!bestQ || q < bestQ); ++q) {
                    if (w.validEntry(q, prevIndex, want)) {
                        bestQ = q;
                        bestWant = want;
                        bestContainer = container;
                        break;
                    }
                }
            }
            if (bestQ) {
                TagRule r{prevTag, bestQ - prevPos, bestContainer, true};
                w.learn(r);
                if (!quiet)
                    std::printf("  -> learned: tag 0x%x entries are %zu bytes, %s\n", prevTag, r.entrySize,
                                bestContainer ? "container" : "file");
                if (bestContainer && !old.container) // re-open what we treated as a file
                    folderEnds.push_back(prevChunk + 6 + prevSize);
                if (!bestContainer && old.container && !folderEnds.empty())
                    folderEnds.pop_back();
                pos = bestQ;
                expectedChunk = bestWant;
                recovered = true;
            }
            if (!recovered) {
                std::printf("entry #%u: no valid entry follows the one at 0x%zx (tag 0x%x)\n", i, prevPos, prevTag);
                hexdump(d, pos > 0x20 ? pos - 0x20 : 0, 0x80);
                std::printf("RESULT: FAIL at entry #%u after a tag 0x%x entry\n", i, prevTag);
                return 1;
            }
        }

        const int index = rd16(d, pos + 4);
        if (prevIndex >= 0 && index != prevIndex + 1)
            indexGaps += unsigned(index - prevIndex - 1);
        const size_t chunk = w.chunkOf(pos);
        const uint16_t tag = rd16(d, chunk);
        const uint32_t size = rd32(d, chunk + 2);
        const TagRule r = w.rule(tag);
        while (!folderEnds.empty() && chunk >= folderEnds.back())
            folderEnds.pop_back();
        const std::string indent(folderEnds.size() * 2, ' ');
        TagStat& st = statFor(tag);
        ++st.count;

        if (r.container) {
            if (!quiet)
                std::printf("%4d %6zx %9zx %4x %8u  %-16s %s<container>\n", index, pos, chunk, tag, size, "",
                            indent.c_str());
            folderEnds.push_back(chunk + 6 + size);
            expectedChunk = chunk + 6;
        } else {
            const size_t p = pos + 8;
            const std::string name = readName(d, p);
            const uint32_t metaSize = rd32(d, p + 0x116);
            const bool sizeOk = metaSize == size;
            st.sizeMismatches += !sizeOk;
            std::string ext = extensionOf(name);
            bool seen = false;
            for (const std::string& e : st.extensions)
                seen |= e == ext;
            if (!seen && !ext.empty())
                st.extensions.push_back(ext);
            if (!quiet)
                std::printf("%4d %6zx %9zx %4x %8u%s %-16s %s%s\n", index, pos, chunk, tag, size, sizeOk ? " " : "*",
                            formatTime(rd32(d, p + 0x11e)).c_str(), indent.c_str(), name.c_str());
            expectedChunk = chunk + 6 + size;
        }
        prevPos = pos;
        prevChunk = chunk;
        prevSize = size;
        prevTag = tag;
        prevIndex = index;
        pos += r.entrySize;
        ++entries;
    }

    const bool tableOk = pos == tocEnd;
    const long long trailing = (long long)d.size() - (long long)expectedChunk;
    if (!quiet) {
        std::printf("\n%u entries, %u index gaps; table walk ended at 0x%zx, table chunk ends at 0x%zx\n", entries,
                    indexGaps, pos, tocEnd);
        std::printf("last chunk ends at %zu, file size %zu\n", expectedChunk, d.size());
        std::printf("* = stored size differs from the size in the entry's copied metadata\n\n");
    }
    std::printf("tags:");
    for (const TagStat& s : stats) {
        const TagRule r = w.rule(s.tag);
        std::printf(" 0x%x x%u", s.tag, s.count);
        if (r.container)
            std::printf("(container");
        else
            std::printf("(");
        for (size_t k = 0; k < s.extensions.size(); ++k)
            std::printf("%s%s", k || r.container ? " " : "", s.extensions[k].c_str());
        if (s.sizeMismatches)
            std::printf(" *%u", s.sizeMismatches);
        std::printf(")");
    }
    std::printf("\n");
    for (const TagRule& r : w.rules)
        if (r.learned)
            std::printf("learned: tag 0x%x entry %zu bytes, %s\n", r.tag, r.entrySize, r.container ? "container" : "file");
    const bool ok = tableOk && trailing >= 0;
    std::printf("RESULT: %s (%u entries, table %s, %lld trailing bytes)\n", ok ? "OK" : "FAIL", entries,
                tableOk ? "exact" : "MISMATCH", trailing);
    return ok ? 0 : 1;
}


int cmdCatList(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: reluva-tool cat-list <file.cat>\n");
        return 2;
    }
    reluva::newworlds::CatArchive cat;
    if (reluva::Error err = cat.open(argv[2])) {
        std::fprintf(stderr, "%s\n", err.message.c_str());
        return 1;
    }
    const auto& entries = cat.entries();
    for (const auto& e : entries) {
        int depth = 0;
        for (int p = e.parent; p >= 0; p = entries[size_t(p)].parent)
            ++depth;
        std::printf("%4x %9u  %-16s %*s%s\n", e.tag, e.size, e.container ? "" : formatTime(e.modified).c_str(),
                    depth * 2, "", e.container ? "<container>" : e.name.c_str());
    }
    return 0;
}

int cmdCatExtract(int argc, char** argv) {
    if (argc < 4) {
        std::fprintf(stderr, "usage: reluva-tool cat-extract <file.cat> <output dir> [name filter]\n");
        return 2;
    }
    reluva::newworlds::CatArchive cat;
    if (reluva::Error err = cat.open(argv[2])) {
        std::fprintf(stderr, "%s\n", err.message.c_str());
        return 1;
    }
    const std::filesystem::path outDir = argv[3];
    const std::string filter = argc > 4 ? argv[4] : "";
    std::vector<uint8_t> data;
    unsigned written = 0;
    for (const auto& e : cat.entries()) {
        if (e.container || e.name.empty() || e.name.find(filter) == std::string::npos)
            continue;
        // Names come from the archive: refuse anything that could escape outDir.
        const std::filesystem::path rel(e.name);
        bool safe = !rel.is_absolute() && !rel.has_root_name();
        for (const auto& part : rel)
            safe &= part != "..";
        if (!safe) {
            std::fprintf(stderr, "skipping unsafe name %s\n", e.name.c_str());
            continue;
        }
        if (reluva::Error err = cat.read(e, data)) {
            std::fprintf(stderr, "%s\n", err.message.c_str());
            return 1;
        }
        const std::filesystem::path out = outDir / rel;
        std::error_code ec;
        std::filesystem::create_directories(out.parent_path(), ec);
        std::ofstream f(out, std::ios::binary);
        f.write(reinterpret_cast<const char*>(data.data()), std::streamsize(data.size()));
        if (!f) {
            std::fprintf(stderr, "cannot write %s\n", out.string().c_str());
            return 1;
        }
        ++written;
    }
    std::printf("extracted %u files to %s\n", written, outDir.string().c_str());
    return 0;
}


int cmdStbDump(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: reluva-tool stb-dump <file.stb>\n");
        return 2;
    }
    Bytes d;
    if (!readFile(argv[2], d))
        return 1;
    reluva::newworlds::StringTable table;
    reluva::Error err = table.parse(d);
    for (const auto& s : table.strings())
        std::printf("%6u  %s\n", s.id, s.text.c_str());
    if (err) {
        std::fprintf(stderr, "%s\n", err.message.c_str());
        return 1;
    }
    std::printf("-- %zu strings, layout matches exactly\n", table.strings().size());
    return 0;
}

// 8-bit paletted BMP, rows top-down. swapRB picks which palette byte is red.
bool writeBmp(const std::string& path, int w, int h, const uint8_t* palette, int paletteCount, const uint8_t* pixels,
              bool swapRB) {
    const int stride = (w + 3) & ~3;
    const uint32_t paletteBytes = 256 * 4, dataOffset = 14 + 40 + paletteBytes;
    const uint32_t fileSize = dataOffset + uint32_t(stride * h);
    Bytes out;
    auto p16 = [&](uint32_t v) { out.push_back(uint8_t(v)); out.push_back(uint8_t(v >> 8)); };
    auto p32 = [&](uint32_t v) { for (int i = 0; i < 4; ++i) out.push_back(uint8_t(v >> (8 * i))); };
    out.push_back('B');
    out.push_back('M');
    p32(fileSize); p32(0); p32(dataOffset);
    p32(40); p32(uint32_t(w)); p32(uint32_t(-h)); p16(1); p16(8); p32(0); p32(uint32_t(stride * h));
    p32(2835); p32(2835); p32(256); p32(0);
    for (int i = 0; i < 256; ++i) {
        uint8_t c0 = 0, c1 = 0, c2 = 0;
        if (i < paletteCount) {
            c0 = palette[i * 4];
            c1 = palette[i * 4 + 1];
            c2 = palette[i * 4 + 2];
        }
        // BMP palettes are stored blue, green, red, 0.
        uint8_t r = swapRB ? c2 : c0, b = swapRB ? c0 : c2;
        out.push_back(b);
        out.push_back(c1);
        out.push_back(r);
        out.push_back(0);
    }
    for (int y = 0; y < h; ++y) {
        out.insert(out.end(), pixels + size_t(y) * size_t(w), pixels + size_t(y + 1) * size_t(w));
        out.resize(out.size() + size_t(stride - w), 0);
    }
    std::ofstream f(path, std::ios::binary);
    f.write(reinterpret_cast<const char*>(out.data()), std::streamsize(out.size()));
    return bool(f);
}

int cmdLbpInfo(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: reluva-tool lbp-info <file> [output prefix]\n");
        return 2;
    }
    Bytes d;
    if (!readFile(argv[2], d))
        return 1;
    reluva::newworlds::Texture tex;
    if (reluva::Error err = reluva::newworlds::decodeTexture(d, tex)) {
        std::printf("doesn't decode: %s\nfirst bytes:\n", err.message.c_str());
        hexdump(d, 0, 64);
        return 1;
    }
    const uint32_t paletteCount = rd32(d, 4);
    const size_t header = 8 + size_t(paletteCount) * 4;
    const size_t pixels = size_t(tex.width) * size_t(tex.height);
    const size_t indices = header + (tex.colourKeyed ? 16 : 12);
    auto colour = [&](unsigned i) { // palette entries are blue, green, red, 0
        char buf[32];
        const uint8_t* e = &d[8 + size_t(i) * 4];
        std::snprintf(buf, sizeof buf, "#%02x%02x%02x", e[2], e[1], e[0]);
        return std::string(buf);
    };

    std::printf("format %u, %d x %d, palette %u entries\n", tex.format, tex.width, tex.height, paletteCount);
    if (tex.colourKeyed)
        std::printf("colour key: index %u = %s\n", tex.keyIndex,
                    tex.keyIndex < paletteCount ? colour(tex.keyIndex).c_str() : "(outside the palette)");
    std::printf("header after the palette:\n");
    hexdump(d, header, tex.colourKeyed ? 16 : 12);

    unsigned hist[256] = {};
    for (size_t i = 0; i < pixels; ++i)
        ++hist[d[indices + i]];
    std::vector<unsigned> order;
    for (unsigned i = 0; i < 256; ++i)
        if (hist[i])
            order.push_back(i);
    std::sort(order.begin(), order.end(), [&](unsigned x, unsigned y) { return hist[x] > hist[y]; });
    std::printf("most used indices:\n");
    for (size_t k = 0; k < order.size() && k < 6; ++k)
        std::printf("  index %3u %s  %6u pixels%s\n", order[k], order[k] < paletteCount ? colour(order[k]).c_str() : "?",
                    hist[order[k]], tex.colourKeyed && order[k] == tex.keyIndex ? "  <- key" : "");

    std::printf("palette entries that look like key colours (magenta or bright green):\n ");
    for (unsigned i = 0; i < paletteCount; ++i) {
        const uint8_t* e = &d[8 + size_t(i) * 4];
        const uint8_t r = e[2], g = e[1], b = e[0];
        if ((r > 200 && b > 200 && g < 60) || (g > 150 && r < 40 && b < 40))
            std::printf(" %u=%s(%u px)", i, colour(i).c_str(), hist[i]);
    }
    std::printf("\n");

    if (argc > 3) {
        const std::string prefix = argv[3];
        Bytes alpha(pixels);
        for (size_t i = 0; i < pixels; ++i)
            alpha[i] = tex.alpha.empty() ? tex.rgba[i * 4 + 3] : tex.alpha[i];
        uint8_t grey[256 * 4];
        for (int i = 0; i < 256; ++i)
            grey[i * 4] = grey[i * 4 + 1] = grey[i * 4 + 2] = uint8_t(i), grey[i * 4 + 3] = 0;
        const bool ok = writeBmp(prefix + ".bmp", tex.width, tex.height, d.data() + 8, int(paletteCount),
                                 d.data() + indices, true) &&
                        writeBmp(prefix + "_alpha.bmp", tex.width, tex.height, grey, 256, alpha.data(), false);
        std::printf(ok ? "wrote %s.bmp and %s_alpha.bmp\n" : "failed writing %s.bmp\n", prefix.c_str(), prefix.c_str());
        return ok ? 0 : 1;
    }
    return 0;
}

// Decode every texture (tag 0x16) in the given archives and group them by
// what their header holds, to find the layouts the decoder doesn't handle.
int cmdTexSurvey(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: reluva-tool tex-survey <file.cat>...\n");
        return 2;
    }
    struct Group {
        unsigned count = 0;
        std::map<std::string, unsigned> extensions;
        std::vector<std::string> examples;
        Bytes firstBytes;
    };
    std::map<std::string, Group> groups;
    unsigned total = 0, decoded = 0;
    std::vector<uint8_t> data;
    for (int a = 2; a < argc; ++a) {
        reluva::newworlds::CatArchive cat;
        if (reluva::Error err = cat.open(argv[a])) {
            std::fprintf(stderr, "%s\n", err.message.c_str());
            return 1;
        }
        for (const auto& e : cat.entries()) {
            if (e.tag != reluva::newworlds::cattag::Texture)
                continue;
            if (cat.read(e, data))
                continue;
            ++total;
            reluva::newworlds::Texture tex;
            const bool ok = !reluva::newworlds::decodeTexture(data, tex);
            decoded += ok;

            // Describe the header: format, palette count, the three u32s
            // after the palette, and data size relative to width x height.
            char key[160];
            const uint32_t format = data.size() >= 4 ? rd32(data, 0) : 0;
            const uint32_t pal = data.size() >= 8 ? rd32(data, 4) : 0;
            const size_t hdr = 8 + size_t(pal) * 4;
            if (pal > 256 || hdr + 12 > data.size()) {
                std::snprintf(key, sizeof key, "%s format %u, u32@4 %u: palette doesn't fit", ok ? "OK  " : "FAIL",
                              format, pal);
            } else {
                const uint32_t f0 = rd32(data, hdr), w = rd32(data, hdr + 4), h = rd32(data, hdr + 8);
                if (f0 == 1 && hdr + 16 <= data.size()) {
                    // Colour-keyed: u32 1, key index, width, height.
                    const uint32_t kw = rd32(data, hdr + 8), kh = rd32(data, hdr + 12);
                    const size_t layers = format == 6 ? 2 : 1;
                    const bool fits = size_t(kw) * kh * layers == data.size() - hdr - 16;
                    std::snprintf(key, sizeof key, "%s format %u, palette %u, colour-keyed, size %s",
                                  ok ? "OK  " : "FAIL", format, pal, fits ? "fits" : "doesn't fit");
                    goto grouped;
                }
                const size_t rest = data.size() - hdr - 12;
                char ratio[48];
                if (w && h && w <= 4096 && h <= 4096 && rest % (size_t(w) * h) == 0)
                    std::snprintf(ratio, sizeof ratio, "data = %zu x w*h", rest / (size_t(w) * h));
                else if (w && h && w <= 4096 && h <= 4096)
                    std::snprintf(ratio, sizeof ratio, "data = %.3f x w*h", double(rest) / (double(w) * h));
                else
                    std::snprintf(ratio, sizeof ratio, "w/h not plausible");
                std::snprintf(key, sizeof key, "%s format %u, palette %u, u32 after palette %u, %s", ok ? "OK  " : "FAIL",
                              format, pal, f0, ratio);
            }
        grouped:
            Group& g = groups[key];
            ++g.count;
            ++g.extensions[extensionOf(e.name)];
            if (g.examples.size() < 3)
                g.examples.push_back(std::string(argv[a]) + ": " + e.name + " (" + std::to_string(data.size()) + " bytes)");
            if (g.firstBytes.empty())
                g.firstBytes.assign(data.begin(), data.begin() + std::min<size_t>(data.size(), 48));
        }
    }
    std::printf("%u textures, %u decode with the current rules\n\n", total, decoded);
    for (const auto& [key, g] : groups) {
        std::printf("%5u  %s\n       extensions:", g.count, key.c_str());
        for (const auto& [ext, n] : g.extensions)
            std::printf(" %s x%u", ext.empty() ? "(none)" : ext.c_str(), n);
        std::printf("\n");
        for (const std::string& ex : g.examples)
            std::printf("       e.g. %s\n", ex.c_str());
        if (key.rfind("FAIL", 0) == 0)
            hexdump(g.firstBytes, 0, g.firstBytes.size());
        std::printf("\n");
    }
    return 0;
}


// For every colour-keyed texture, compare the key index with the corner
// pixels, which in sprites and interface images are nearly always background.
int cmdTexKeys(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: reluva-tool tex-keys <file.cat>...\n");
        return 2;
    }
    unsigned total = 0, keyIsCorner = 0, cornerLooksKey = 0;
    std::vector<uint8_t> d;
    std::printf("%-44s %3s %4s %-8s %6s  %-14s %s\n", "texture", "fmt", "key", "colour", "px", "corners", "corner colour");
    for (int a = 2; a < argc; ++a) {
        reluva::newworlds::CatArchive cat;
        if (reluva::Error err = cat.open(argv[a])) {
            std::fprintf(stderr, "%s\n", err.message.c_str());
            return 1;
        }
        for (const auto& e : cat.entries()) {
            if (e.tag != reluva::newworlds::cattag::Texture || cat.read(e, d))
                continue;
            reluva::newworlds::Texture tex;
            if (reluva::newworlds::decodeTexture(d, tex) || !tex.colourKeyed)
                continue;
            ++total;
            const uint32_t pal = rd32(d, 4);
            const size_t idx = 8 + size_t(pal) * 4 + 16;
            const size_t w = size_t(tex.width), h = size_t(tex.height);
            const uint8_t corners[4] = {d[idx], d[idx + w - 1], d[idx + (h - 1) * w], d[idx + h * w - 1]};
            auto colour = [&](unsigned i) {
                char buf[16];
                if (i >= pal)
                    return std::string("?");
                const uint8_t* c = &d[8 + size_t(i) * 4];
                std::snprintf(buf, sizeof buf, "#%02x%02x%02x", c[2], c[1], c[0]);
                return std::string(buf);
            };
            unsigned keyPixels = 0;
            for (size_t i = 0; i < w * h; ++i)
                keyPixels += d[idx + i] == tex.keyIndex;
            const bool allSame = corners[0] == corners[1] && corners[0] == corners[2] && corners[0] == corners[3];
            const bool match = allSame && corners[0] == tex.keyIndex;
            keyIsCorner += match;
            const uint8_t* cc = corners[0] < pal ? &d[8 + size_t(corners[0]) * 4] : nullptr;
            const bool looksKey = cc && ((cc[2] > 200 && cc[0] > 200 && cc[1] < 60) || (cc[1] > 150 && cc[2] < 40 && cc[0] < 40));
            cornerLooksKey += looksKey;
            char cornerText[24];
            std::snprintf(cornerText, sizeof cornerText, "%u %u %u %u", corners[0], corners[1], corners[2], corners[3]);
            std::string name = e.name.size() > 44 ? "..." + e.name.substr(e.name.size() - 41) : e.name;
            std::printf("%-44s %3u %4u %-8s %6u  %-14s %s%s%s\n", name.c_str(), tex.format, tex.keyIndex,
                        colour(tex.keyIndex).c_str(), keyPixels, cornerText, colour(corners[0]).c_str(),
                        match ? "  key = corners" : "", looksKey ? "  (key-like colour)" : "");
        }
    }
    std::printf("\n%u colour-keyed textures: key index = all four corners in %u; corner colour is magenta/green in %u\n",
                total, keyIsCorner, cornerLooksKey);
    return 0;
}


// A payload counts as a container if it splits exactly into chunks whose
// tags look plausible. Small tag limit keeps random data from qualifying.
bool tilesIntoChunks(const Bytes& d, size_t begin, size_t end) {
    if (end - begin < 6)
        return false;
    size_t p = begin;
    while (p < end) {
        if (p + 6 > end)
            return false;
        const uint16_t tag = rd16(d, p);
        const uint32_t size = rd32(d, p + 2);
        if (tag > 0x400 || size > end - p - 6)
            return false;
        p += 6 + size;
    }
    return p == end;
}

std::string describeWords(const Bytes& d, size_t p, size_t end, int count) {
    std::string out;
    char buf[64];
    for (int i = 0; i < count && p + 4 <= end; ++i, p += 4) {
        const uint32_t v = rd32(d, p);
        float f;
        std::memcpy(&f, &v, 4);
        const bool plausibleFloat = (v & 0x7f800000) != 0 && (v & 0x7f800000) != 0x7f800000 && f > -1e6f &&
                                    f < 1e6f && (f > 1e-4f || f < -1e-4f);
        if (plausibleFloat)
            std::snprintf(buf, sizeof buf, " %gf", double(f));
        else
            std::snprintf(buf, sizeof buf, " %u", v);
        out += buf;
    }
    return out;
}

void printChunks(const Bytes& d, size_t begin, size_t end, int depth) {
    size_t p = begin;
    while (p < end) {
        const uint16_t tag = rd16(d, p);
        const uint32_t size = rd32(d, p + 2);
        const size_t payload = p + 6, payloadEnd = payload + size;
        const bool container = tilesIntoChunks(d, payload, payloadEnd);
        std::printf("%*s%06zx tag 0x%04x size %u%s", depth * 2, "", p, tag, size, container ? "  {" : "");
        if (!container)
            std::printf("  :%s%s", describeWords(d, payload, payloadEnd, 6).c_str(), size > 24 ? " ..." : "");
        std::printf("\n");
        if (container)
            printChunks(d, payload, payloadEnd, depth + 1);
        p = payloadEnd;
    }
}

int cmdChunks(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: reluva-tool chunks <file>\n");
        return 2;
    }
    Bytes d;
    if (!readFile(argv[2], d))
        return 1;
    std::printf("%s: %zu bytes\n", argv[2], d.size());
    if (!tilesIntoChunks(d, 0, d.size())) {
        std::printf("doesn't divide into chunks from the start; first words:%s\n",
                    describeWords(d, 0, d.size(), 12).c_str());
        // Maybe there's a header: report the first offset from which it does.
        for (size_t start = 1; start < std::min<size_t>(d.size(), 64); ++start)
            if (tilesIntoChunks(d, start, d.size())) {
                std::printf("but it does from offset %zu:\n", start);
                printChunks(d, start, d.size(), 1);
                return 0;
            }
        return 1;
    }
    printChunks(d, 0, d.size(), 0);
    return 0;
}

int cmdWords(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: reluva-tool words <file> [offset] [count]\n");
        return 2;
    }
    Bytes d;
    if (!readFile(argv[2], d))
        return 1;
    size_t off = argc > 3 ? size_t(parseNumber(argv[3])) : 0;
    const size_t count = argc > 4 ? size_t(parseNumber(argv[4])) : 64;
    std::printf("%8s  %8s  %11s  %14s  %s\n", "offset", "hex", "u32", "float", "u16 pair");
    for (size_t i = 0; i < count && off + 4 <= d.size(); ++i, off += 4) {
        const uint32_t v = rd32(d, off);
        float f;
        std::memcpy(&f, &v, 4);
        std::printf("%8zx  %08x  %11u  %14g  %u, %u\n", off, v, v, double(f), v & 0xffff, v >> 16);
    }
    return 0;
}


int cmdTable(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: reluva-tool table <file> [values per row] [offset]\n");
        return 2;
    }
    Bytes d;
    if (!readFile(argv[2], d))
        return 1;
    const size_t perRow = argc > 3 ? std::max<size_t>(1, size_t(parseNumber(argv[3]))) : 8;
    size_t off = argc > 4 ? size_t(parseNumber(argv[4])) : 0;
    for (; off + 4 <= d.size(); off += perRow * 4) {
        std::printf("%6zx:", off);
        for (size_t i = 0; i < perRow && off + i * 4 + 4 <= d.size(); ++i) {
            const uint32_t v = rd32(d, off + i * 4);
            float f;
            std::memcpy(&f, &v, 4);
            const bool isFloat = (v & 0x7f800000) != 0 && (v & 0x7f800000) != 0x7f800000 && f > -1e7f && f < 1e7f &&
                                 (f > 1e-6f || f < -1e-6f);
            if (isFloat)
                std::printf(" %10.4gf", double(f));
            else
                std::printf(" %11u", v);
        }
        std::printf("\n");
    }
    if (d.size() % 4)
        std::printf("(%zu trailing bytes)\n", d.size() % 4);
    return 0;
}


bool writeObj(const reluva::newworlds::GeoModel& m, const std::string& path) {
    std::ofstream f(path);
    f << "# exported by reluva-tool geo-info\n";
    // OBJ numbers vertices across the whole file, so offset each part's.
    std::vector<size_t> base;
    size_t next = 1;
    for (const auto& part : m.parts) {
        base.push_back(next);
        next += part.vertices.size();
        for (const auto& v : part.vertices)
            f << "v " << v.x << ' ' << v.y << ' ' << v.z << '\n';
    }
    size_t vt = 1;
    for (size_t pi = 0; pi < m.parts.size(); ++pi) {
        f << "g part" << pi << '\n';
        for (const auto& face : m.faces) {
            if (face.part != pi)
                continue;
            for (const auto& c : face.corners)
                f << "vt " << c.u << ' ' << 1.0f - c.v << '\n';
            f << 'f';
            for (const auto& c : face.corners)
                f << ' ' << base[pi] + c.vertex << '/' << vt++;
            f << '\n';
        }
    }
    return bool(f);
}

int cmdGeoInfo(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: reluva-tool geo-info <file.geo> [out.obj]\n");
        return 2;
    }
    Bytes d;
    if (!readFile(argv[2], d))
        return 1;
    reluva::newworlds::GeoModel m;
    if (reluva::Error err = reluva::newworlds::parseGeo(d, m)) {
        std::printf("doesn't decode: %s\n", err.message.c_str());
        return 1;
    }
    size_t corners = 0;
    for (const auto& face : m.faces)
        corners += face.corners.size();
    std::printf("%zu faces (%zu corners), %zu part(s)\n", m.faces.size(), corners, m.parts.size());
    for (size_t pi = 0; pi < m.parts.size(); ++pi) {
        const auto& part = m.parts[pi];
        float lo[3] = {1e30f, 1e30f, 1e30f}, hi[3] = {-1e30f, -1e30f, -1e30f};
        for (const auto& v : part.vertices) {
            const float c[3] = {v.x, v.y, v.z};
            for (int i = 0; i < 3; ++i) {
                lo[i] = std::min(lo[i], c[i]);
                hi[i] = std::max(hi[i], c[i]);
            }
        }
        size_t faces = 0;
        for (const auto& face : m.faces)
            faces += face.part == pi;
        std::string lists;
        for (const auto& list : part.normalLists)
            lists += (lists.empty() ? "" : "+") + std::to_string(list.size());
        std::printf("  part %zu (%u): %zu faces, %zu vertices, %zu edges, normal lists of %s; x %g..%g, y %g..%g, "
                    "z %g..%g\n",
                    pi, part.unknown, faces, part.vertices.size(), part.edges.size(),
                    lists.empty() ? "none" : lists.c_str(), double(lo[0]),
                    double(hi[0]), double(lo[1]), double(hi[1]), double(lo[2]), double(hi[2]));
    }
    std::printf("centre %g, %g; radius %g\n", double(m.centreX), double(m.centreY), double(m.radius));
    // How often each value of the faces' second header word occurs (face 0's
    // header is the file header, so it's left out).
    std::map<uint32_t, unsigned> seconds;
    for (size_t i = 1; i < m.faces.size(); ++i)
        ++seconds[m.faces[i].groupB];
    std::printf("faces' second header value (value x count):");
    for (const auto& [value, n] : seconds)
        std::printf(" %u x%u", value, n);
    std::printf("\n");
    // The last value of each face's last corner: a texture number?
    std::map<uint32_t, unsigned> tails;
    for (const auto& face : m.faces)
        ++tails[face.corners.back().extra[4]];
    std::printf("last value of each face's last corner (value x count):");
    for (const auto& [value, n] : tails)
        std::printf(" %u x%u", value, n);
    std::printf("\n");

    // Test: is the second value an index into the face's part's first normal
    // list, giving the face's normal? Compare with the normal worked out from
    // the corners (Newell's method), either way round.
    unsigned same = 0, opposite = 0, other = 0, outOfRange = 0;
    for (size_t i = 1; i < m.faces.size(); ++i) {
        const auto& face = m.faces[i];
        const auto& part = m.parts[face.part];
        if (part.normalLists.empty() || face.groupB >= part.normalLists[0].size()) {
            ++outOfRange;
            continue;
        }
        double n[3] = {0, 0, 0};
        for (size_t k = 0; k < face.corners.size(); ++k) {
            const auto& a = part.vertices[face.corners[k].vertex];
            const auto& b = part.vertices[face.corners[(k + 1) % face.corners.size()].vertex];
            n[0] += double(a.y - b.y) * double(a.z + b.z);
            n[1] += double(a.z - b.z) * double(a.x + b.x);
            n[2] += double(a.x - b.x) * double(a.y + b.y);
        }
        const double len = std::sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        const auto& s = part.normalLists[0][face.groupB];
        const double d = len > 0 ? (n[0] * s.x + n[1] * s.y + n[2] * s.z) / len : 0;
        if (d > 0.99)
            ++same;
        else if (d < -0.99)
            ++opposite;
        else
            ++other;
    }
    std::printf("second value as an index into the part's first normal list: %u match the face's normal, "
                "%u the opposite, %u neither, %u out of range\n",
                same, opposite, other, outOfRange);
    if (argc > 3) {
        if (!writeObj(m, argv[3])) {
            std::fprintf(stderr, "cannot write %s\n", argv[3]);
            return 1;
        }
        std::printf("wrote %s\n", argv[3]);
    }
    return 0;
}

int cmdGeoFaces(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: reluva-tool geo-faces <file.geo>\n");
        return 2;
    }
    Bytes d;
    if (!readFile(argv[2], d))
        return 1;
    // Same walk as parseGeo, but any fourth header word is accepted and shown.
    const uint32_t faceCount = rd32(d, 4);
    std::printf("%zu bytes, %u faces\n", d.size(), faceCount);
    size_t p = 0;
    for (uint32_t i = 0; i < faceCount; ++i) {
        const uint32_t n = rd32(d, p + 8);
        if (n < 3 || n > 256 || p + 16 + size_t(n) * 32 > d.size()) {
            std::printf("face %u at 0x%zx: bad header (%u, %u, %u, %u); bytes around it:\n", i, p, rd32(d, p),
                        rd32(d, p + 4), n, rd32(d, p + 12));
            hexdump(d, p >= 0x60 ? p - 0x60 : 0, 0x160);
            return 1;
        }
        std::printf("face %u at 0x%zx: (%u, %u, %u, %u)\n", i, p, rd32(d, p), rd32(d, p + 4), n, rd32(d, p + 12));
        p += 16;
        for (uint32_t k = 0; k < n; ++k, p += 32) {
            float u, v;
            const uint32_t ub = rd32(d, p + 4), vb = rd32(d, p + 8);
            std::memcpy(&u, &ub, 4);
            std::memcpy(&v, &vb, 4);
            std::printf("    v %-4u uv %9.4f %9.4f  rest %u %u %u %u %u\n", rd32(d, p), double(u / 65536),
                        double(v / 65536), rd32(d, p + 12), rd32(d, p + 16), rd32(d, p + 20), rd32(d, p + 24),
                        rd32(d, p + 28));
        }
    }
    std::printf("faces end at 0x%zx; next bytes:\n", p);
    hexdump(d, p, 0x100);
    return 0;
}

int cmdGeoTextures(int argc, char** argv) {
    int first = 2;
    const bool all = argc > 2 && std::strcmp(argv[2], "-a") == 0;
    if (all)
        ++first;
    if (argc <= first) {
        std::fprintf(stderr, "usage: reluva-tool geo-textures [-a] <file.cat>...\n");
        return 2;
    }
    namespace nw = reluva::newworlds;
    nw::Archives archives;
    for (int a = first; a < argc; ++a) {
        auto cat = std::make_unique<nw::CatArchive>();
        if (reluva::Error err = cat->open(argv[a])) {
            std::fprintf(stderr, "%s\n", err.message.c_str());
            return 1;
        }
        archives.push_back(std::move(cat));
    }
    const std::vector<nw::TextureFile> textures = nw::listTextureFiles(archives);
    unsigned models = 0, without = 0, short_ = 0;
    std::vector<uint8_t> data;
    for (const auto& cat : archives)
        for (const auto& e : cat->entries()) {
            if (e.tag != nw::cattag::Geometry)
                continue;
            ++models;
            // How many textures the faces number: the highest number + 1.
            uint32_t needed = 0;
            nw::GeoModel m;
            if (!cat->read(e, data) && !nw::parseGeo(data, m))
                for (const auto& face : m.faces)
                    needed = std::max(needed, face.corners.back().extra[4] + 1);
            const auto slots = nw::findModelTextures(e.name, textures, needed);
            const size_t found = size_t(std::count_if(slots.begin(), slots.end(), [](auto* t) { return t; }));
            const bool tooFew = found < needed;
            if (found == 0)
                ++without;
            else if (tooFew)
                ++short_;
            if (!all && !tooFew)
                continue;
            std::printf("%s: needs %u, found", e.name.c_str(), needed);
            for (const auto* t : slots)
                std::printf(" %s", t ? t->stem.c_str() : "?");
            if (slots.empty())
                std::printf(" none");
            if (tooFew) {
                // Textures in the same folder whose names start like the model's.
                std::string folder, stem, ext;
                nw::splitName(e.name, folder, stem, ext);
                const size_t prefix = std::min<size_t>(3, stem.size());
                std::printf("; similar:");
                unsigned shown = 0;
                for (const auto& t : textures)
                    if (t.folder == folder && t.stem.compare(0, prefix, stem, 0, prefix) == 0 && shown++ < 10)
                        std::printf(" %s", t.stem.c_str());
                if (!shown)
                    std::printf(" -");
            }
            std::printf("\n");
        }
    std::printf("%u models: %u without textures, %u with fewer than their faces number\n", models, without,
                short_);
    return 0;
}

// Map (.gmf) layout as far as known: u16 counts of type-21, type-20 and
// type-19 records, u16 grid width and height; those records (u16 type, u16
// length, u16 ?, then `length` bytes) in that order; then data of unknown
// layout. See docs/formats/newworlds/gmf.md.
int cmdGmfInfo(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: reluva-tool gmf-info <file.gmf> [record number]\n");
        return 2;
    }
    Bytes d;
    if (!readFile(argv[2], d))
        return 1;
    const long want = argc > 3 ? std::strtol(argv[3], nullptr, 0) : -1;
    const unsigned counts[3] = {rd16(d, 0), rd16(d, 2), rd16(d, 4)};
    const uint16_t types[3] = {21, 20, 19};
    std::printf("%zu bytes; %u type-21, %u type-20 and %u type-19 records; grid %u x %u\n", d.size(), counts[0],
                counts[1], counts[2], rd16(d, 6), rd16(d, 8));
    {
        reluva::newworlds::GmfMap map;
        const reluva::Error err = reluva::newworlds::parseGmf(d, map);
        std::printf("engine's reader: %s\n", err ? err.message.c_str() : "reads it");
        if (!err) {
            std::map<int, size_t> types;
            for (const auto& o : map.scenery)
                ++types[o.type];
            std::printf("placed objects: %zu;", map.scenery.size());
            for (const auto& [type, n] : types)
                std::printf(" type %d x%zu", type, n);
            std::printf("\n");
        }
    }

    struct Record {
        size_t offset;
        uint16_t type, length, third;
    };
    std::vector<Record> records;
    size_t p = 10;
    for (int k = 0; k < 3; ++k) {
        std::map<uint16_t, unsigned> lengths, thirds;
        const size_t start = p;
        for (unsigned i = 0; i < counts[k]; ++i) {
            const Record r{p, rd16(d, p), rd16(d, p + 2), rd16(d, p + 4)};
            if (p + 6 > d.size() || r.type != types[k] || p + 6 + r.length > d.size()) {
                std::printf("type-%u record %u at 0x%zx doesn't fit (type %u, length %u):\n", types[k], i, p, r.type,
                            r.length);
                hexdump(d, p, 0x40);
                return 1;
            }
            records.push_back(r);
            ++lengths[r.length];
            ++thirds[r.third];
            p += 6 + r.length;
        }
        std::printf("type %u: 0x%zx..0x%zx; lengths", types[k], start, p);
        for (const auto& [v, n] : lengths)
            std::printf(" %u x%u", v, n);
        std::printf("; third values");
        for (const auto& [v, n] : thirds)
            std::printf(" %u x%u", v, n);
        std::printf("\n");
    }
    std::printf("after the records: 0x%zx, %zu bytes (%.2f per grid cell)\n", p, d.size() - p,
                double(d.size() - p) / std::max(1, rd16(d, 6) * rd16(d, 8)));

    if (want >= 0 && size_t(want) < records.size()) {
        const Record& r = records[size_t(want)];
        std::printf("record %ld at 0x%zx: type %u, length %u, third %u\n", want, r.offset, r.type, r.length, r.third);
        hexdump(d, r.offset, 6 + size_t(r.length));
        return 0;
    }
    // Type 19: four u32 indices after 6 bytes, presumably into the tiles
    // (the type-21/20 records). Check they're in range.
    {
        const size_t first = counts[0] + counts[1];
        uint32_t maxIndex = 0;
        for (size_t i = first; i < records.size(); ++i)
            for (int k = 0; k < 4; ++k)
                maxIndex = std::max(maxIndex, rd32(d, records[i].offset + 6 + 6 + 4 * size_t(k)));
        std::printf("type-19 indices: highest %u (%u tiles)\n", maxIndex, counts[0]);
        // Type 20: u32 own number, u32, u32, u32, u16, then 8 x 8 byte grids.
        unsigned numbered = 0;
        for (size_t i = counts[0]; i < first; ++i)
            numbered += rd32(d, records[i].offset + 6) == i - counts[0];
        std::printf("type-20 records whose first value is their own number: %u of %u\n", numbered, counts[1]);
    }

    // Each tile's type-20 header values: own number, 0, model?, flags?, u16.
    std::printf("tiles (number: type-20 values 2-5):");
    for (size_t i = counts[0]; i < size_t(counts[0]) + counts[1]; ++i) {
        const size_t o = records[i].offset + 6;
        std::printf("%s %zu: %u %u 0x%x %u", (i - counts[0]) % 6 ? ";" : "\n ", i - counts[0], rd32(d, o + 4),
                    rd32(d, o + 8), rd32(d, o + 12), rd16(d, o + 16));
    }
    std::printf("\n");

    // After the records: a u32 count, then that many u32 entries (lists ending
    // ffffffff), then one 10-byte entry per grid cell (u16 block, u16 ?, u16 ?,
    // u32 where its list starts), then 1024 more bytes.
    {
        const uint32_t n = rd32(d, p);
        size_t separators = 0;
        const size_t end = p + 4 + size_t(n) * 4;
        if (end <= d.size()) {
            for (size_t o = p + 4; o < end; o += 4)
                separators += rd32(d, o) == 0xffffffffu;
            std::printf("after the records: u32 %u; %u u32s from there hold %zu ffffffff separators and end at 0x%zx, "
                        "leaving %zu bytes (%.2f per grid cell)\n",
                        n, n, separators, end, d.size() - end,
                        double(d.size() - end) / std::max(1, rd16(d, 6) * rd16(d, 8)));
            const unsigned w = rd16(d, 6), h = rd16(d, 8);
            if (d.size() - end >= size_t(w) * h * 10) {
                std::map<uint16_t, unsigned> seconds;
                std::printf("blocks on the grid (rows of %u; 0-9 then a-z, '+' for higher):\n", w);
                for (unsigned y = 0; y < h; ++y) {
                    std::printf("  ");
                    for (unsigned x = 0; x < w; ++x) {
                        const size_t e = end + (size_t(y) * w + x) * 10;
                        const unsigned b = rd16(d, e);
                        ++seconds[rd16(d, e + 2)];
                        std::printf("%c", b < 10 ? char('0' + b) : b < 36 ? char('a' + b - 10) : '+');
                    }
                    std::printf("\n");
                }
                std::printf("second values of the grid entries:");
                for (const auto& [v, n] : seconds)
                    std::printf(" %u x%u", v, n);
                std::printf("\nlast 1024 bytes start:\n");
                hexdump(d, end + size_t(w) * h * 10, 0x40);
            } else {
                std::printf("start of what's left:\n");
                hexdump(d, end, 0x80);
            }
        } else {
            std::printf("after the records: u32 %u (too many for the file to be that many u32s)\n", n);
        }
    }

    // A sample of each kind, and the start of what follows.
    for (int k = 0; k < 3; ++k) {
        size_t first = 0;
        for (int j = 0; j < k; ++j)
            first += counts[j];
        if (counts[k] == 0)
            continue;
        const size_t pick = first + counts[k] / 2;
        const Record& r = records[pick];
        std::printf("record %zu (a type-%u one from the middle) at 0x%zx:\n", pick, r.type, r.offset);
        if (r.type == 20) { // header values, then the 8 x 8 grids
            std::printf("  values %u %u %u %u %u; grids:\n", rd32(d, r.offset + 6), rd32(d, r.offset + 10),
                        rd32(d, r.offset + 14), rd32(d, r.offset + 18), rd16(d, r.offset + 22));
            const size_t grids = r.offset + 6 + 18;
            for (unsigned row = 0; row < 8; ++row) {
                std::printf("   ");
                for (size_t g = 0; g * 64 + 64 <= size_t(r.length) - 18; ++g) {
                    for (unsigned c = 0; c < 8; ++c)
                        std::printf(" %02x", d[grids + g * 64 + row * 8 + c]);
                    std::printf("  ");
                }
                std::printf("\n");
            }
            const size_t rest = grids + ((size_t(r.length) - 18) / 64) * 64;
            if (rest < r.offset + 6 + r.length) {
                std::printf("  after the grids:\n");
                hexdump(d, rest, r.offset + 6 + r.length - rest);
            }
            continue;
        }
        hexdump(d, r.offset, std::min<size_t>(6 + size_t(r.length), 0x80));
    }
    if (counts[2] > 1) {
        std::printf("first type-19 records, one per line:\n");
        const size_t first = counts[0] + counts[1];
        for (size_t i = first; i < first + std::min<size_t>(counts[2], 12); ++i)
            hexdump(d, records[i].offset, 6 + size_t(records[i].length));
    }
    std::printf("after the records:\n");
    hexdump(d, p, 0x80);
    return 0;
}

int cmdGeoSurvey(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: reluva-tool geo-survey <file.cat>...\n");
        return 2;
    }
    std::map<std::string, std::vector<std::string>> failures;
    unsigned total = 0, ok = 0;
    std::vector<uint8_t> d;
    for (int a = 2; a < argc; ++a) {
        reluva::newworlds::CatArchive cat;
        if (reluva::Error err = cat.open(argv[a])) {
            std::fprintf(stderr, "%s\n", err.message.c_str());
            return 1;
        }
        for (const auto& e : cat.entries()) {
            if (e.tag != reluva::newworlds::cattag::Geometry || cat.read(e, d))
                continue;
            ++total;
            reluva::newworlds::GeoModel m;
            reluva::Error err = reluva::newworlds::parseGeo(d, m);
            if (!err) {
                ++ok;
                continue;
            }
            // Group by message with numbers (decimal or 0x hex) removed.
            std::string key;
            const std::string& msg = err.message;
            for (size_t i = 0; i < msg.size();) {
                if (msg[i] >= '0' && msg[i] <= '9') {
                    if (msg.compare(i, 2, "0x") == 0)
                        i += 2;
                    while (i < msg.size() && std::isxdigit(static_cast<unsigned char>(msg[i])))
                        ++i;
                    key += '#';
                } else {
                    key += msg[i++];
                }
            }
            failures[key].push_back(std::string(argv[a]) + ": " + e.name + " (" + std::to_string(d.size()) +
                                    " bytes): " + err.message);
        }
    }
    std::printf("%u models, %u decode\n\n", total, ok);
    for (const auto& [key, list] : failures) {
        std::printf("%5zu  %s\n", list.size(), key.c_str());
        for (size_t i = 0; i < list.size() && i < 3; ++i)
            std::printf("       e.g. %s\n", list[i].c_str());
    }
    return 0;
}

} // namespace

int stafuToolMain(int argc, char** argv);

int main(int argc, char** argv) {
    if (argc >= 2 && std::strcmp(argv[1], "-stafu") == 0)
        return stafuToolMain(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "hexdump") == 0)
        return cmdHexdump(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "cat-probe") == 0)
        return cmdCatProbe(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "cat-list") == 0)
        return cmdCatList(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "cat-extract") == 0)
        return cmdCatExtract(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "stb-dump") == 0)
        return cmdStbDump(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "lbp-info") == 0)
        return cmdLbpInfo(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "tex-survey") == 0)
        return cmdTexSurvey(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "tex-keys") == 0)
        return cmdTexKeys(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "chunks") == 0)
        return cmdChunks(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "words") == 0)
        return cmdWords(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "table") == 0)
        return cmdTable(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "geo-info") == 0)
        return cmdGeoInfo(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "geo-survey") == 0)
        return cmdGeoSurvey(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "gmf-info") == 0)
        return cmdGmfInfo(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "geo-textures") == 0)
        return cmdGeoTextures(argc, argv);
    if (argc >= 2 && std::strcmp(argv[1], "geo-faces") == 0)
        return cmdGeoFaces(argc, argv);
    std::fprintf(stderr,
                 "usage:\n"
                 "  reluva-tool -stafu archive-probe <file>\n"
                 "  reluva-tool hexdump <file> [offset] [length]\n"
                 "  reluva-tool cat-probe <file.cat> [-q]\n"
                 "  reluva-tool cat-list <file.cat>\n"
                 "  reluva-tool cat-extract <file.cat> <output dir> [name filter]\n"
                 "  reluva-tool stb-dump <file.stb>\n"
                 "  reluva-tool tex-survey <file.cat>...\n"
                 "  reluva-tool tex-keys <file.cat>...\n"
                 "  reluva-tool chunks <file>\n"
                 "  reluva-tool words <file> [offset] [count]\n"
                 "  reluva-tool table <file> [values per row] [offset]\n"
                 "  reluva-tool geo-info <file.geo> [out.obj]\n"
                 "  reluva-tool geo-survey <file.cat>...\n"
                 "  reluva-tool geo-textures [-a] <file.cat>...\n"
                 "  reluva-tool gmf-info <file.gmf> [record number]\n"
                 "  reluva-tool geo-faces <file.geo>\n"
                 "  reluva-tool lbp-info <file> [output prefix]\n");
    return 2;
}
