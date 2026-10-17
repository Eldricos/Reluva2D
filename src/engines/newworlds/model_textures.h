// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#pragma once

#include "engines/newworlds/cat_archive.h"

#include <memory>
#include <string>
#include <vector>

namespace reluva::newworlds {

using Archives = std::vector<std::unique_ptr<CatArchive>>;

// Where an item lives: archive index and entry index.
struct EntryRef {
    size_t archive, entry;
};

// A full-size texture: an .lbp, or level 0 (.mp0) of a mipmap set.
struct TextureFile {
    std::string stem, folder; // "k_bop_a", "kln" for lbp/txr/kln/k_bop_a.mp0
    EntryRef ref;
};

// "lbp/txr/kln/k_bop_a.mp0" -> folder "kln", stem "k_bop_a", extension "mp0".
void splitName(const std::string& name, std::string& folder, std::string& stem, std::string& ext);

// Every full-size texture in the archives, sorted by stem.
std::vector<TextureFile> listTextureFiles(const Archives& archives);

// A model's textures, in the order its faces number them; `needed` is how
// many the faces number (the highest number + 1). Slots the rule can't fill
// (shared textures with other names) are null. See
// docs/formats/newworlds/geo.md#textures for the rule.
std::vector<const TextureFile*> findModelTextures(const std::string& modelName,
                                                  const std::vector<TextureFile>& textures, size_t needed);

} // namespace reluva::newworlds
