// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#pragma once

#include "common/events.h"
#include "common/system.h"
#include "common/types.h"
#include "engines/newworlds/cat_archive.h"
#include "engines/newworlds/geo.h"
#include "engines/newworlds/gmf.h"
#include "engines/newworlds/model_textures.h"
#include "engines/newworlds/rasterizer.h"
#include "engines/newworlds/texture.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

// Data viewers used by the New Worlds engine until the game itself runs.
// Each draws into the engine's 640x480 RGBA32 screen and reports its state in
// the window title.

namespace reluva::newworlds {

// Browses every texture.
//   Left/Right: previous/next   PageUp/PageDown: jump 10   Home/End: first/last
//   F: filter (all, failed, colour-keyed, with alpha)
class TextureViewer {
public:
    TextureViewer(System& sys, Surface& screen, const Archives& archives);
    size_t count() const { return textures_.size(); }
    void activate(); // redraw and retitle after switching to this viewer
    void handleKey(uint32_t key);

private:
    struct TextureRef {
        EntryRef ref;
        bool ok = false, colourKeyed = false, alpha = false;
    };

    void classify();
    void applyFilter(int filter);
    void show(int index);
    void fill(int x0, int y0, int w, int h, uint8_t r, uint8_t g, uint8_t b);
    void drawCheckerboard();
    void drawTexture(const Texture& tex);

    System& sys_;
    Surface& screen_;
    const Archives& archives_;
    std::vector<TextureRef> textures_;
    std::vector<size_t> visible_; // indices into textures_ that pass the filter
    bool classified_ = false;
    int filter_ = 0;
    int current_ = 0;
};

// Shows every model with its textures. A model's textures are the ones named
// after it (k_bop: k_bop_a, k_bop_b, k_bop_g), in name order; each face picks
// one with the last value of its last corner. See docs/formats/newworlds/geo.md.
//   T: textures on/off   G: alpha layers (as glow, ignored, as transparency)
//   Left/Right: previous/next   PageUp/PageDown: jump 10   Home/End: first/last
//   Mouse drag or W/A/S/D: rotate   wheel or +/-: zoom   R: reset the view
//   Space: spin   P: show one part at a time   U: texture-coordinate checks
//   C: colour by texture number instead of texturing   H: smooth or flat shading
//   L: outlines (solid, solid + outlines, outlines only)
//   B: back faces (hidden, shown, only back faces)   F: filter (all, multi-part, failed)
class ModelViewer {
public:
    ModelViewer(System& sys, Surface& screen, const Archives& archives);
    size_t count() const { return models_.size(); }
    void activate();
    void handleEvent(const Event& ev);
    void update(uint64_t nowMs); // redraws when something changed

private:
    void classify();
    void applyFilter(int filter);
    void show(int index);
    void resetView();
    void render();
    void updateTitle();
    void drawLine(float x0, float y0, float x1, float y1);
    void loadTextures(const std::string& modelName);

    System& sys_;
    Surface& screen_;
    const Archives& archives_;
    Rasterizer raster_;
    std::vector<EntryRef> models_;
    std::vector<uint32_t> partCounts_; // per model; 0 when it doesn't decode
    std::vector<size_t> visible_;
    std::vector<TextureFile> textureFiles_; // every full-size texture
    std::vector<Texture> modelTextures_;    // the current model's, in face order
    std::vector<std::string> modelTextureNames_;
    bool classified_ = false;
    int filter_ = 0;
    int current_ = 0;

    GeoModel model_;
    std::string status_; // decode error, if any
    float centre_[3] = {0, 0, 0};
    float size_ = 1; // distance from the centre to the farthest vertex

    float yaw_ = 0, pitch_ = 0, zoom_ = 1;
    int soloPart_ = -1; // -1: all parts
    bool spin_ = false, uvCheck_ = false, colourByTail_ = false, textured_ = true, smooth_ = true;
    int outlines_ = 0, faces_ = 0; // faces_: 0 front only, 1 both sides, 2 back only
    int alphaMode_ = 0;            // see kAlphaNames
    bool dragging_ = false;
    int dragX_ = 0, dragY_ = 0;
    bool dirty_ = true;
    uint64_t lastMs_ = 0;
};

// Shows the planet maps (.gmf), built from their ground tiles: a map is a
// grid of blocks, a block 2 x 2 tiles, and a tile one of the planet's ground
// models. See docs/formats/newworlds/gmf.md.
//   Left/Right: previous/next map   Mouse drag: rotate   wheel or +/-: zoom
//   W/A/S/D: move   Q/E: turn   R: reset the view   L: outlines   K: walls
//   O: placed objects (trees, rocks) on/off   H: smooth or flat shading
//   T: textures on/off   G: colouring (the models' own textures; the map's
//   terrain codes in its colour table; textures tinted by resource deposit)
class MapViewer {
public:
    MapViewer(System& sys, Surface& screen, const Archives& archives);
    size_t count() const { return maps_.size(); }
    void activate();
    void handleEvent(const Event& ev);
    void update(uint64_t nowMs); // redraws when something changed

private:
    using Vec3 = GeoModel::Vec3;
    struct TileModel {
        std::string name;              // e.g. "geo/map/ice/ice_07.geo"
        std::string error;             // when it doesn't decode
        GeoModel geo;
        std::vector<Vec3> faceNormals; // per face, unit length
        std::vector<Texture> textures; // in the order the faces number them
        std::vector<std::string> textureNames;
    };

    // A tile's edge heights after its rotation, at 5 points along each side
    // (east, north, west, south); a side without vertices on it is missing.
    struct TileEdges {
        bool present[4] = {};
        float z[4][5] = {};
    };

    // A placed object, ready to draw.
    struct Placed {
        Vec3 position;
        float angle; // anticlockwise, in radians
        const TileModel* model;
    };

    void show(int index);
    void resetView();
    void render();
    void updateTitle();
    void loadModel(size_t entry, TileModel& m); // an entry of the map's archive
    const TileModel& tileModel(uint32_t index);
    float groundHeight(float x, float y);
    void placeScenery();
    void makeUnderlay();
    const TileEdges& tileEdges(uint32_t tile);
    int tileAt(int tx, int ty) const; // tile record at a tile position, or -1
    static unsigned terrainCode(const GmfMap::Tile& tile, int cx, int cy);

    System& sys_;
    Surface& screen_;
    const Archives& archives_;
    Rasterizer raster_;
    std::vector<EntryRef> maps_;
    std::vector<TextureFile> textureFiles_;
    bool shown_ = false;
    int current_ = 0;

    GmfMap map_;
    std::string status_;                 // decode error, if any
    std::vector<size_t> archiveModels_;  // entry indices of the map archive's .geo files, in order
    std::map<uint32_t, TileModel> tileModels_;
    std::map<uint32_t, TileEdges> tileEdges_;
    Texture wallTexture_; // the map archive's last cliff texture, if any
    std::map<int, TileModel> sceneryModels_; // by object type
    std::vector<Placed> placed_;
    Texture underlay_; // ground showing through tiles with GmfMap::Tile::underlay

    float targetX_ = 0, targetY_ = 0, yaw_ = 0, pitch_ = 0, distance_ = 1;
    bool textured_ = true, outlines_ = false, walls_ = true, scenery_ = true, smooth_ = true;
    int colouring_ = 0; // see kColouringNames
    bool dragging_ = false;
    int dragX_ = 0, dragY_ = 0;
    bool dirty_ = true;
    unsigned drawnTiles_ = 0, drawnFaces_ = 0, drawnObjects_ = 0;
};

} // namespace reluva::newworlds
