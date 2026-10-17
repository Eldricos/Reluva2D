// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#include "engines/newworlds/model_textures.h"

#include <algorithm>
#include <cstddef>
#include <string>

namespace reluva::newworlds {

void splitName(const std::string& name, std::string& folder, std::string& stem, std::string& ext) {
    const size_t slash = name.rfind('/');
    const size_t dot = name.rfind('.');
    const size_t start = slash == std::string::npos ? 0 : slash + 1;
    stem = name.substr(start, (dot == std::string::npos || dot < start ? name.size() : dot) - start);
    ext = dot == std::string::npos || dot < start ? std::string() : name.substr(dot + 1);
    folder.clear();
    if (slash != std::string::npos && slash > 0) {
        const size_t prev = name.rfind('/', slash - 1);
        const size_t from = prev == std::string::npos ? 0 : prev + 1;
        folder = name.substr(from, slash - from);
    }
}

std::vector<TextureFile> listTextureFiles(const Archives& archives) {
    std::vector<TextureFile> out;
    for (size_t a = 0; a < archives.size(); ++a) {
        const auto& entries = archives[a]->entries();
        for (size_t i = 0; i < entries.size(); ++i) {
            if (entries[i].tag != cattag::Texture)
                continue;
            TextureFile t;
            std::string ext;
            splitName(entries[i].name, t.folder, t.stem, ext);
            if (ext == "lbp" || ext == "mp0") {
                t.ref = {a, i};
                out.push_back(std::move(t));
            }
        }
    }
    std::stable_sort(out.begin(), out.end(), [](const TextureFile& x, const TextureFile& y) { return x.stem < y.stem; });
    return out;
}

namespace {

bool endsWith(const std::string& s, const char* suffix) {
    const size_t n = std::char_traits<char>::length(suffix);
    return s.size() >= n && s.compare(s.size() - n, n, suffix) == 0;
}

// "landpd" in "landpad": the letters in order, starting the same way.
bool abbreviates(const std::string& shortName, const std::string& full) {
    if (shortName.size() < 3 || full.size() <= shortName.size() || full.compare(0, 2, shortName, 0, 2) != 0)
        return false;
    size_t i = 0;
    for (char c : full)
        if (i < shortName.size() && c == shortName[i])
            ++i;
    return i == shortName.size();
}

} // namespace

// A model's own textures are named after it: k_bop uses k_bop_a, k_bop_b and
// k_bop_g. A variant such as k_bop_l with no textures of its own uses those
// of the name without the suffix (except the flat _s outlines), and a
// shortened name (landpd_d) those of a texture it abbreviates (landpad).
// Textures in the model's own folder (kln for geo/kln/k_bop.geo) are
// preferred, and _s textures belong to the _s models only.
//
// The faces number the plain textures from 0 in name order; the _g light map
// is the last number (k_bop: 0 k_bop_a, 1 k_bop_b, 2 k_bop_g). Numbers in
// between are shared textures with other names, which aren't known.
std::vector<const TextureFile*> findModelTextures(const std::string& modelName,
                                                  const std::vector<TextureFile>& textures, size_t needed) {
    std::string folder, base, ext;
    splitName(modelName, folder, base, ext);
    const bool outline = endsWith(base, "_s");
    std::vector<const TextureFile*> found;
    auto collect = [&](auto&& matches) {
        for (int sameFolder = 1; sameFolder >= 0 && found.empty(); --sameFolder)
            for (const TextureFile& t : textures)
                if ((!sameFolder || t.folder == folder) && (outline || !endsWith(t.stem, "_s")) && matches(t))
                    found.push_back(&t);
    };
    while (true) {
        collect([&](const TextureFile& t) {
            return t.stem == base || (t.stem.size() > base.size() + 1 && t.stem.compare(0, base.size(), base) == 0 &&
                                      t.stem[base.size()] == '_' &&
                                      t.stem.find('_', base.size() + 1) == std::string::npos);
        });
        // Otherwise drop a short suffix (_l, _d, _ad, ...) and try again; but
        // not _s, the flat outlines, which don't use the full model's textures.
        const size_t us = base.rfind('_');
        if (!found.empty() || us == std::string::npos || base.size() - us > 3 || base.compare(us, 3, "_s") == 0)
            break;
        base.resize(us);
    }
    if (found.empty() && !outline) // a shortened name: landpd -> landpad, its textures too
        collect([&](const TextureFile& t) {
            const size_t us = t.stem.find('_');
            return t.folder == folder && abbreviates(base, t.stem.substr(0, us));
        });

    // One of each name (some are stored both as .lbp and as a mipmap set).
    std::vector<const TextureFile*> plain, glow;
    for (const TextureFile* t : found) {
        auto& list = endsWith(t->stem, "_g") ? glow : plain;
        if (std::none_of(list.begin(), list.end(), [&](const TextureFile* x) { return x->stem == t->stem; }))
            list.push_back(t);
    }
    std::vector<const TextureFile*> out(std::max(needed, plain.size() + glow.size()), nullptr);
    std::copy(plain.begin(), plain.end(), out.begin());
    std::copy(glow.begin(), glow.end(), out.end() - std::ptrdiff_t(glow.size()));
    return out;
}

} // namespace reluva::newworlds
