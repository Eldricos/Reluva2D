// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

// Tests for the New Worlds .cat reader against archives built here, following
// docs/formats/newworlds/cat.md, so no game data is needed.

#include "engines/newworlds/cat_archive.h"
#include "engines/newworlds/string_table.h"
#include "engines/newworlds/texture.h"
#include "engines/newworlds/geo.h"
#include "engines/newworlds/gmf.h"
#include "engines/newworlds/model_textures.h"
#include "engines/newworlds/rasterizer.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace reluva;
using namespace reluva::newworlds;

static int failures = 0;

#define CHECK(cond)                                                     \
    do {                                                                \
        if (!(cond)) {                                                  \
            std::fprintf(stderr, "%s:%d: CHECK failed: %s\n", __FILE__, \
                         __LINE__, #cond);                              \
            ++failures;                                                 \
        }                                                               \
    } while (0)

namespace {

struct Item {
    std::string path; // empty for containers
    uint16_t tag;
    std::string payload;
    std::vector<Item> children;
};

Item file(std::string path, uint16_t tag, std::string payload) { return {std::move(path), tag, std::move(payload), {}}; }
Item container(uint16_t tag, std::vector<Item> children) { return {"", tag, "", std::move(children)}; }

void put16(std::vector<uint8_t>& b, uint16_t v) {
    b.push_back(uint8_t(v));
    b.push_back(uint8_t(v >> 8));
}
void put32(std::vector<uint8_t>& b, uint32_t v) {
    for (int i = 0; i < 4; ++i)
        b.push_back(uint8_t(v >> (8 * i)));
}
void set32(std::vector<uint8_t>& b, size_t at, uint32_t v) {
    for (int i = 0; i < 4; ++i)
        b[at + size_t(i)] = uint8_t(v >> (8 * i));
}

uint32_t chunkLength(const Item& it) {
    if (it.children.empty() && !isContainerTag(it.tag))
        return 6 + uint32_t(it.payload.size());
    uint32_t n = 6;
    for (const Item& c : it.children)
        n += chunkLength(c);
    return n;
}

struct Builder {
    std::vector<uint8_t> table, data;
    uint32_t count = 0, index = 0;

    void add(const std::vector<Item>& items) {
        for (const Item& it : items) {
            const bool isContainer = isContainerTag(it.tag);
            // The real offset is data position + 12 (two chunk headers).
            put32(table, uint32_t(data.size()) + 12);
            put16(table, uint16_t(index++));
            table.push_back(0);
            table.push_back(0x15);
            put16(data, it.tag);
            if (isContainer) {
                put32(data, chunkLength(it) - 6);
                table.resize(table.size() + 256, 0xcd); // junk, as in real files
                ++count;
                add(it.children);
                ++index; // a folder uses an extra index after its children
            } else {
                put32(data, uint32_t(it.payload.size()));
                data.insert(data.end(), it.payload.begin(), it.payload.end());
                const size_t base = table.size();
                table.resize(base + 326, 0);
                std::memcpy(&table[base], it.path.data(), it.path.size());
                set32(table, base + 0x116, uint32_t(it.payload.size()));
                set32(table, base + 0x11e, 966294866); // 2000-08-14 23:14:26 UTC
                ++count;
            }
        }
    }

    std::vector<uint8_t> build() const {
        std::vector<uint8_t> body;
        put16(body, 5);
        put32(body, uint32_t(table.size() + 4));
        put32(body, count);
        body.insert(body.end(), table.begin(), table.end());
        body.insert(body.end(), data.begin(), data.end());
        std::vector<uint8_t> out;
        put16(out, 1);
        put32(out, uint32_t(body.size()));
        out.insert(out.end(), body.begin(), body.end());
        return out;
    }
};

std::filesystem::path writeArchive(const std::filesystem::path& dir, const std::vector<uint8_t>& bytes) {
    std::filesystem::create_directories(dir);
    auto p = dir / "test.cat";
    std::ofstream(p, std::ios::binary).write(reinterpret_cast<const char*>(bytes.data()), std::streamsize(bytes.size()));
    return p;
}

const char* kPrefix = "c:\\prjs\\game\\stc\\bin\\rsc\\dat\\pc_win32\\";

std::vector<Item> sampleItems() {
    return {
        file(std::string(kPrefix) + "str\\eng\\miss\\rom01.stb", cattag::Strings, "strings!"),
        container(cattag::TextureSet,
                  {file(std::string(kPrefix) + "lbp\\txr\\rom\\rb_muzzl.mmb", cattag::Mmb, "8 bytes."),
                   file(std::string(kPrefix) + "lbp\\txr\\rom\\rb_muzzl.mp0", cattag::Texture, "pixels")}),
        container(cattag::FontSet,
                  {container(cattag::FontPart, {file(std::string(kPrefix) + "gui\\font.fif", cattag::Fif, "font")})}),
        file("c:\\prjs\\STNWSFX\\alert003.wav", cattag::Wave, "RIFF...."),
    };
}

void testRead(const std::filesystem::path& tmp) {
    Builder b;
    b.add(sampleItems());
    CatArchive cat;
    Error err = cat.open(writeArchive(tmp, b.build()));
    CHECK(!err);
    if (err) {
        std::fprintf(stderr, "  %s\n", err.message.c_str());
        return;
    }

    const auto& e = cat.entries();
    CHECK(e.size() == 8);
    CHECK(e[0].name == "str/eng/miss/rom01.stb" && e[0].tag == cattag::Strings && e[0].parent == -1);
    CHECK(e[0].modified == 966294866);
    CHECK(e[1].container && e[1].parent == -1);
    CHECK(e[2].name == "lbp/txr/rom/rb_muzzl.mmb" && e[2].parent == 1);
    CHECK(e[3].parent == 1);
    CHECK(e[4].container && e[5].container && e[5].parent == 4);
    CHECK(e[6].name == "gui/font.fif" && e[6].parent == 5);
    CHECK(e[7].name == "stnwsfx/alert003.wav" && e[7].parent == -1);

    const CatEntry* wav = cat.find("STNWSFX\\Alert003.WAV");
    CHECK(wav == &e[7]);
    std::vector<uint8_t> data;
    CHECK(wav && !cat.read(*wav, data));
    CHECK(std::string(data.begin(), data.end()) == "RIFF....");
    const CatEntry* mp0 = cat.find("lbp/txr/rom/rb_muzzl.mp0");
    CHECK(mp0 && !cat.read(*mp0, data) && std::string(data.begin(), data.end()) == "pixels");
    CHECK(cat.find("missing.stb") == nullptr);
}

void testRejectsCorruption(const std::filesystem::path& tmp) {
    Builder b;
    b.add(sampleItems());
    const std::vector<uint8_t> good = b.build();
    CatArchive cat;

    std::vector<uint8_t> bad = good;
    bad[0] = 2; // wrong root tag
    CHECK(cat.open(writeArchive(tmp, bad)));

    bad = good;
    bad.push_back(0); // root size no longer matches the file
    CHECK(cat.open(writeArchive(tmp, bad)));

    bad = good;
    bad[0x10] += 1; // first entry's offset off by one: breaks the chunk chain
    CHECK(cat.open(writeArchive(tmp, bad)));

    CHECK(!cat.open(writeArchive(tmp, good)));
}

void testNormalizeName() {
    CHECK(CatArchive::normalizeName("c:\\prjs\\game\\stc\\bin\\rsc\\dat\\pc_win32\\geo\\rom\\rb_mid.geo") ==
          "geo/rom/rb_mid.geo");
    CHECK(CatArchive::normalizeName("c:\\prjs\\STNWSFX\\alert003.wav") == "stnwsfx/alert003.wav");
    CHECK(CatArchive::normalizeName("") == "");
}

// Laid out like the start of rom01.stb: count, then (id, UTF-16 text, 0).
std::vector<uint8_t> stringTableBytes(const std::vector<std::pair<uint32_t, std::u16string>>& strings,
                                      uint32_t count) {
    std::vector<uint8_t> b;
    put32(b, count);
    for (const auto& [id, str] : strings) {
        put32(b, id);
        for (char16_t c : str)
            put16(b, uint16_t(c));
        put16(b, 0);
    }
    return b;
}

void testStringTable() {
    const std::vector<std::pair<uint32_t, std::u16string>> strings = {
        {1, u" 'Et Tu?'"},
        {23, u"Stardate 8988.4.  The type 6 world Sakuntula has significant deposits of dilithium."},
        {22, u"~STELLAR CARTOGRAPHY~"},
        {5, u""},
    };
    StringTable t;
    CHECK(!t.parse(stringTableBytes(strings, 4)));
    CHECK(t.strings().size() == 4);
    CHECK(t.find(1) && *t.find(1) == " 'Et Tu?'");
    CHECK(t.find(22) && *t.find(22) == "~STELLAR CARTOGRAPHY~");
    CHECK(t.find(5) && t.find(5)->empty());
    CHECK(t.find(99) == nullptr);

    CHECK(t.parse(stringTableBytes(strings, 5)));    // count too high
    CHECK(t.parse(stringTableBytes(strings, 3)));    // bytes left over
    auto cut = stringTableBytes(strings, 4);
    cut.resize(cut.size() - 1);
    CHECK(t.parse(cut));                             // last string unterminated
}

// Format 6: u32 6, u32 palette count, palette (blue, green, red, 0),
// u32 0, u32 width, u32 height, indices, alpha.
// keyIndex >= 0: colour-keyed layout (u32 1, u32 key, width, height).
std::vector<uint8_t> textureBytes(uint32_t w, uint32_t h, uint32_t format = 6, int keyIndex = -1) {
    std::vector<uint8_t> b;
    put32(b, format);
    put32(b, 2);
    b.insert(b.end(), {0x10, 0x20, 0xf0, 0}); // index 0: red 0xf0, green 0x20, blue 0x10
    b.insert(b.end(), {0xff, 0x00, 0x00, 0}); // index 1: pure blue
    if (keyIndex >= 0) {
        put32(b, 1);
        put32(b, uint32_t(keyIndex));
    } else {
        put32(b, 0);
    }
    put32(b, w);
    put32(b, h);
    for (uint32_t i = 0; i < w * h; ++i)
        b.push_back(uint8_t(i % 2));          // indices alternate 0, 1
    if (format == 6)
        for (uint32_t i = 0; i < w * h; ++i)
            b.push_back(uint8_t(i * 10));     // alpha
    return b;
}

void testTexture() {
    Texture t;
    CHECK(!decodeTexture(textureBytes(4, 2), t));
    CHECK(t.width == 4 && t.height == 2 && t.rgba.size() == 32);
    CHECK(t.rgba[0] == 0xf0 && t.rgba[1] == 0x20 && t.rgba[2] == 0x10 && t.rgba[3] == 255); // pixel 0
    CHECK(t.rgba[4] == 0x00 && t.rgba[5] == 0x00 && t.rgba[6] == 0xff && t.rgba[7] == 255); // pixel 1
    CHECK(t.alpha.size() == 8 && t.alpha[0] == 0 && t.alpha[1] == 10 && t.alpha[7] == 70); // separate layer

    // Format 1: one layer, opaque.
    CHECK(!decodeTexture(textureBytes(4, 2, 1), t));
    CHECK(t.format == 1 && t.width == 4 && t.height == 2 && !t.colourKeyed);
    CHECK(t.rgba[4] == 0x00 && t.rgba[6] == 0xff && t.rgba[7] == 255 && t.alpha.empty());

    // Colour key: pixels using the key index become transparent.
    CHECK(!decodeTexture(textureBytes(5, 3, 1, 1), t));
    CHECK(t.colourKeyed && t.keyIndex == 1 && t.width == 5 && t.height == 3);
    CHECK(t.rgba[3] == 255 && t.rgba[7] == 0);            // index 0 opaque, index 1 keyed out
    CHECK(!decodeTexture(textureBytes(4, 2, 6, 0), t));
    CHECK(t.rgba[3] == 0 && t.rgba[7] == 255 && t.alpha[1] == 10); // key is separate from the alpha layer

    CHECK(decodeTexture(textureBytes(4, 2, 7), t)); // unknown format
    auto bad = textureBytes(4, 2);
    bad.pop_back();
    CHECK(decodeTexture(bad, t));                  // wrong size
    CHECK(decodeTexture(textureBytes(0, 2), t));   // zero width
}

void putF(std::vector<uint8_t>& b, float f) {
    uint32_t v;
    std::memcpy(&v, &f, 4);
    put32(b, v);
}

// A two-sided triangle laid out as in docs/formats/newworlds/geo.md: face 0
// in the file header, face 1 its back, one edge group (or none, as in
// terrain tiles).
// Two triangles back to back. With twoParts the back is part 1, with its
// own copy of the vertices.
std::vector<uint8_t> triangleGeo(bool withEdges = true, bool oddAttachment = false, bool twoParts = false) {
    const float pos[3][2] = {{0, 0}, {100, 0}, {0, 200}};
    const float uv[3][2] = {{0, 0}, {65536, 0}, {0, 65536}};
    const uint32_t parts = twoParts ? 2 : 1;
    std::vector<uint8_t> b;
    auto corner = [&](uint32_t v) {
        put32(b, v);
        putF(b, uv[v][0]);
        putF(b, uv[v][1]);
        for (int i = 0; i < 5; ++i)
            put32(b, 0);
    };
    put32(b, 0); put32(b, 2); put32(b, 3); put32(b, 0); // header = face 0: 2 faces, 3 corners
    corner(0); corner(1); corner(2);
    put32(b, 0); put32(b, 0); put32(b, 3); put32(b, parts - 1); // face 1, the back
    corner(2); corner(1); corner(0);
    put32(b, parts - 1); put32(b, 1); put32(b, parts); put32(b, 0); // bounds, no attachments
    putF(b, 0); putF(b, 0); putF(b, 224);
    put32(b, 1); put32(b, 0); put32(b, 0);
    if (oddAttachment) // odd-length data puts the part data off 4-byte alignment
        b.insert(b.end(), {0x1c, 0x00, 0x0c});
    put32(b, 0); put32(b, 1); put32(b, parts);          // part data
    for (uint32_t part = 0; part < parts; ++part) {
        if (part > 0)
            put32(b, 0);                                // end of the previous part
        put32(b, part); put32(b, 3);                    // part 1 starts with 1, as in ptank_l
        for (auto& p : pos) {
            putF(b, p[0]); putF(b, p[1]); putF(b, 0);
        }
        put32(b, 1); put32(b, 2);                       // two normals
        putF(b, 0); putF(b, 0); putF(b, 1);
        putF(b, 0); putF(b, 0); putF(b, -1);
        if (withEdges && !twoParts) {
            put32(b, 1); put32(b, 1); put32(b, 3);      // one group of three edges
            const uint32_t edges[3][2] = {{0, 1}, {1, 2}, {2, 0}};
            for (auto& e : edges) {
                put32(b, 0); put32(b, e[0]); put32(b, e[1]); put32(b, 0); put32(b, 1);
            }
        } else {
            put32(b, 0);
        }
    }
    return b;
}

void testGeo() {
    GeoModel m;
    Error err = parseGeo(triangleGeo(), m);
    CHECK(!err);
    if (err) {
        std::fprintf(stderr, "  %s\n", err.message.c_str());
        return;
    }
    CHECK(m.parts.size() == 1 && m.parts[0].vertices.size() == 3 && m.parts[0].vertices[2].y == 200.0f);
    CHECK(m.faces.size() == 2 && m.faces[0].corners.size() == 3 && m.faces[1].corners[0].vertex == 2);
    CHECK(m.faces[0].corners[1].vertex == 1 && m.faces[0].corners[1].u == 1.0f);
    const auto& part = m.parts[0];
    CHECK(part.edges.size() == 3 && part.edges[1].v1 == 2 && part.edges[1].faceB == 1);
    CHECK(part.normalLists.size() == 1 && part.normalLists[0].size() == 2 && m.radius == 224.0f);

    CHECK(!parseGeo(triangleGeo(false), m) && m.parts[0].edges.empty()); // terrain-style: no edges

    err = parseGeo(triangleGeo(true, true), m);
    CHECK(!err && m.parts[0].vertices.size() == 3 && m.parts[0].vertices[1].x == 100.0f);

    err = parseGeo(triangleGeo(false, false, true), m);
    CHECK(!err && m.parts.size() == 2 && m.faces[1].part == 1 && m.parts[1].vertices.size() == 3 &&
          m.parts[1].unknown == 1);

    auto bad = triangleGeo();
    bad[bad.size() - 16] = 7; // last edge now starts at vertex 7
    CHECK(parseGeo(bad, m));
}

// A 2 x 1 map: 2 tiles, 1 block, both cells using it.
std::vector<uint8_t> tinyGmf() {
    std::vector<uint8_t> b;
    auto put16 = [&](uint16_t v) { b.push_back(uint8_t(v)); b.push_back(uint8_t(v >> 8)); };
    put16(2); put16(2); put16(1); put16(2); put16(1);
    for (int i = 0; i < 2; ++i) { // type 21, contents unknown
        put16(21); put16(4); put16(0); put32(b, 0);
    }
    for (uint32_t i = 0; i < 2; ++i) { // type 20: number, 0, model, flags, u16, 8 x 8 grid
        put16(20); put16(18 + 64); put16(0);
        put32(b, i); put32(b, 0); put32(b, 6 + i); put32(b, 0x18000 + 0x4000 * i); put16(0);
        for (int c = 0; c < 64; ++c)
            b.push_back(uint8_t(c == 0 ? 2 : 11));
    }
    put16(19); put16(22); put16(0); // block: 6 bytes, then four tile numbers
    for (int i = 0; i < 6; ++i)
        b.push_back(0);
    for (uint32_t t : {0u, 1u, 1u, 0u})
        put32(b, t);
    put32(b, 3); put32(b, 0x00a1b277); put32(b, 0xffffffffu); put32(b, 0xffffffffu); // lists
    for (uint32_t cell = 0; cell < 2; ++cell) {                                     // grid
        put16(0); put16(50); put16(0); put32(b, cell * 2); // one object, then none
    }
    // The colour table: magenta, then one colour (B, G, R), then the end.
    const uint8_t palette[8] = {0xff, 0x00, 0xff, 0x00, 0xf9, 0xc6, 0x42, 0x00};
    b.insert(b.end(), palette, palette + 8);
    b.insert(b.end(), 1024 - 8, 0);
    return b;
}

void testGmf() {
    GmfMap m;
    Error err = parseGmf(tinyGmf(), m);
    CHECK(!err);
    if (err) {
        std::fprintf(stderr, "  %s\n", err.message.c_str());
        return;
    }
    CHECK(m.width == 2 && m.height == 1 && m.tiles.size() == 2 && m.blocks.size() == 1 && m.cells.size() == 2);
    CHECK(m.tiles[1].model == 7 && m.tiles[0].layers[0] == 2 && m.tiles[0].layers[1] == 11);
    CHECK(m.tiles[0].rotation == 2 && m.tiles[1].rotation == 3);
    CHECK(m.blocks[0][2] == 1 && m.cells[1].block == 0 && m.cells[1].value == 50);
    CHECK(m.scenery.size() == 1 && m.scenery[0].bx == 0 && m.scenery[0].x == 7 && m.scenery[0].y == 7);
    CHECK(m.scenery[0].type == 1 && m.scenery[0].turn == 4 && m.scenery[0].variant == 2 && m.scenery[0].extra == 50);
    CHECK(m.palette.size() == 2 && m.palette[1][0] == 0x42 && m.palette[1][1] == 0xc6 && m.palette[1][2] == 0xf9);
    auto bad = tinyGmf();
    bad[bad.size() - 1024 - 10] = 5; // the last cell uses block 5 of 1
    CHECK(parseGmf(bad, m));
}

void testModelTextures() {
    std::vector<TextureFile> tex;
    for (const char* name : {"k_bop_a", "k_bop_b", "k_bop_g", "kcs", "kcs_g", "kcs_l", "k_bop2_a", "kwb"})
        tex.push_back({name, "kln", {0, 0}});
    tex.push_back({"kwb", "fed", {0, 0}}); // same name elsewhere: the model's own folder wins
    for (const char* name : {"fhf_g", "fpdt", "fpdt_s", "fvpy", "fvpy", "landpad"})
        tex.push_back({name, "fed", {0, 0}});
    auto names = [&](const char* model, size_t needed) {
        std::string out;
        for (const TextureFile* t : findModelTextures(model, tex, needed))
            out += (out.empty() ? "" : " ") + (t ? t->stem + (t->folder == "kln" ? "" : "@" + t->folder) : "?");
        return out;
    };
    CHECK(names("geo/kln/k_bop.geo", 3) == "k_bop_a k_bop_b k_bop_g"); // not k_bop2_a
    CHECK(names("geo/kln/k_bop_l.geo", 3) == "k_bop_a k_bop_b k_bop_g"); // falls back without _l
    CHECK(names("geo/kln/kcs_l.geo", 1) == "kcs_l");                     // its own texture first
    CHECK(names("geo/kln/kcs_s.geo", 1) == "?");                          // outlines don't fall back
    CHECK(names("geo/fed/kwb.geo", 1) == "kwb@fed");
    CHECK(names("geo/fed/fhf.geo", 3) == "? ? fhf_g@fed");       // the light map is the last number
    CHECK(names("geo/fed/fpdt.geo", 2) == "fpdt@fed ?");          // fpdt_s is the outline's
    CHECK(names("geo/fed/fvpy_a.geo", 1) == "fvpy@fed");          // duplicates count once
    CHECK(names("geo/fed/landpd_d.geo", 2) == "landpad@fed ?");   // a shortened name
}

void testRasterizer() {
    Surface surf;
    surf.create(16, 16, PixelFormat::RGBA32);
    Rasterizer r;
    r.begin(surf);
    // Two triangles making the square 2..12; every pixel inside is drawn once.
    std::vector<int> hits(16 * 16, 0);
    auto count = [&](float, float, uint8_t* px) {
        ++hits[size_t(px - surf.pixels.data()) / 4];
        return true;
    };
    const ScreenVertex a{2, 2, 1, 0, 0}, b{12, 2, 1, 1, 0}, c{12, 12, 1, 1, 1}, d{2, 12, 1, 0, 1};
    r.drawTriangle(a, b, c, count);
    r.begin(surf); // fresh depth so the second triangle isn't hidden by equal depth
    r.drawTriangle(a, c, d, count);
    bool exact = true;
    for (int y = 0; y < 16; ++y)
        for (int x = 0; x < 16; ++x)
            exact &= hits[size_t(y * 16 + x)] == ((x >= 2 && x < 12 && y >= 2 && y < 12) ? 1 : 0);
    CHECK(exact);

    // The nearer triangle wins whichever is drawn first; u/v interpolate.
    for (int order = 0; order < 2; ++order) {
        r.begin(surf);
        const ScreenVertex n0{0, 0, 1.0f, 0, 0}, n1{16, 0, 1.0f, 1, 0}, n2{0, 16, 1.0f, 0, 1};
        const ScreenVertex f0{0, 0, 0.5f, 0, 0}, f1{16, 0, 0.5f, 0, 0}, f2{0, 16, 0.5f, 0, 0};
        auto paint = [](uint8_t value) {
            return [value](float, float, uint8_t* px) {
                px[0] = value;
                return true;
            };
        };
        if (order == 0) {
            r.drawTriangle(n0, n1, n2, paint(1));
            r.drawTriangle(f0, f1, f2, paint(2));
        } else {
            r.drawTriangle(f0, f1, f2, paint(2));
            r.drawTriangle(n0, n1, n2, paint(1));
        }
        CHECK(surf.row(2)[2 * 4] == 1);
    }
    float seenU = -1;
    r.begin(surf);
    r.drawTriangle({0, 0, 1, 0, 0}, {16, 0, 1, 1, 0}, {0, 16, 1, 0, 0},
                   [&](float u, float, uint8_t* px) {
                       if (px == surf.row(0) + 8 * 4)
                           seenU = u;
                       return true;
                   });
    CHECK(std::fabs(seenU - 8.5f / 16) < 1e-4f);
}

} // namespace

int main() {
    auto tmp = std::filesystem::temp_directory_path() / "reluva_cat_tests";
    testNormalizeName();
    testRead(tmp);
    testRejectsCorruption(tmp);
    testStringTable();
    testTexture();
    testGeo();
    testGmf();
    testModelTextures();
    testRasterizer();
    std::filesystem::remove_all(tmp);
    if (failures)
        std::fprintf(stderr, "%d check(s) failed\n", failures);
    else
        std::printf("All tests passed\n");
    return failures ? 1 : 0;
}
