// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#include "engines/newworlds/view3d.h"
#include "engines/newworlds/viewers.h"

#include <algorithm>
#include <cmath>

namespace reluva::newworlds {

namespace {

constexpr int kFilterCount = 3;
constexpr const char* kFilterNames[kFilterCount] = {"all", "multi-part", "failed"};
// How a texture's alpha layer (format 6) is used. In k_bop_g it marks the
// lights, so by default it's a glow map: alpha keeps texels lit.
constexpr const char* kAlphaNames[3] = {"alpha as glow", "alpha ignored", "alpha as transparency"};
constexpr const char* kOutlineNames[3] = {"solid", "solid + outlines", "outlines"};
// Front faces are clockwise on screen (models store them clockwise), so by
// default anticlockwise faces are the back faces and are hidden.
constexpr const char* kFaceNames[3] = {"", "showing back faces", "showing back faces only"};

constexpr float kPi = 3.14159265f;
constexpr float kMaxPitch = 89.0f * kPi / 180.0f;
constexpr float kKeyTurn = 10.0f * kPi / 180.0f;
constexpr float kSpinPerMs = 0.0008f;

// Part colours (R, G, B), cycled when a model has more parts.
constexpr uint8_t kPartColours[8][3] = {
    {170, 180, 195}, {225, 140, 70}, {90, 170, 230}, {120, 200, 110},
    {215, 95, 120},  {200, 190, 90}, {160, 120, 220}, {90, 200, 190},
};

} // namespace

ModelViewer::ModelViewer(System& sys, Surface& screen, const Archives& archives)
    : sys_(sys), screen_(screen), archives_(archives) {
    for (size_t a = 0; a < archives_.size(); ++a) {
        const auto& entries = archives_[a]->entries();
        for (size_t i = 0; i < entries.size(); ++i)
            if (entries[i].tag == cattag::Geometry)
                models_.push_back({a, i});
    }
    textureFiles_ = listTextureFiles(archives_);
}

void ModelViewer::activate() {
    if (!classified_) {
        classify();
        classified_ = true;
        resetView();
        applyFilter(0);
        return;
    }
    dirty_ = true;
    updateTitle();
}

// Decode every model once so the filters know which is which.
void ModelViewer::classify() {
    std::vector<uint8_t> data;
    GeoModel m;
    partCounts_.assign(models_.size(), 0);
    for (size_t i = 0; i < models_.size(); ++i) {
        CatArchive& cat = *archives_[models_[i].archive];
        if (!cat.read(cat.entries()[models_[i].entry], data) && !parseGeo(data, m))
            partCounts_[i] = uint32_t(m.parts.size());
    }
}

void ModelViewer::applyFilter(int filter) {
    filter_ = filter;
    visible_.clear();
    for (size_t i = 0; i < models_.size(); ++i) {
        const uint32_t parts = partCounts_[i];
        if (filter == 0 || (filter == 1 && parts > 1) || (filter == 2 && parts == 0))
            visible_.push_back(i);
    }
    if (visible_.empty()) {
        model_ = GeoModel{};
        status_ = "none (press F for the next filter)";
        dirty_ = true;
        updateTitle();
        return;
    }
    show(0);
}

void ModelViewer::show(int index) {
    current_ = index;
    soloPart_ = -1;
    status_.clear();
    const EntryRef& ref = models_[visible_[size_t(index)]];
    CatArchive& cat = *archives_[ref.archive];
    std::vector<uint8_t> data;
    if (Error err = cat.read(cat.entries()[ref.entry], data))
        status_ = err.message;
    else if (Error err2 = parseGeo(data, model_))
        status_ = err2.message;
    if (!status_.empty())
        model_ = GeoModel{};
    loadTextures(cat.entries()[ref.entry].name);

    // Frame the model: centre of its bounding box, distance to the farthest vertex.
    float lo[3] = {1e30f, 1e30f, 1e30f}, hi[3] = {-1e30f, -1e30f, -1e30f};
    for (const auto& part : model_.parts)
        for (const auto& v : part.vertices) {
            const float c[3] = {v.x, v.y, v.z};
            for (int i = 0; i < 3; ++i) {
                lo[i] = std::min(lo[i], c[i]);
                hi[i] = std::max(hi[i], c[i]);
            }
        }
    size_ = 1;
    if (lo[0] <= hi[0]) {
        for (int i = 0; i < 3; ++i)
            centre_[i] = (lo[i] + hi[i]) / 2;
        float far2 = 0;
        for (const auto& part : model_.parts)
            for (const auto& v : part.vertices) {
                const Vec d = Vec{v.x, v.y, v.z} - Vec{centre_[0], centre_[1], centre_[2]};
                far2 = std::max(far2, dot(d, d));
            }
        size_ = std::max(std::sqrt(far2), 1e-3f);
    }
    dirty_ = true;
    updateTitle();
}

void ModelViewer::loadTextures(const std::string& modelName) {
    modelTextures_.clear();
    modelTextureNames_.clear();
    size_t needed = 0; // the highest texture number + 1
    for (const auto& face : model_.faces)
        needed = std::max<size_t>(needed, face.corners.back().extra[4] + 1);
    std::vector<uint8_t> data;
    for (const TextureFile* t : findModelTextures(modelName, textureFiles_, needed)) {
        // Unknown or unreadable textures stay empty, keeping the numbering;
        // their faces are drawn untextured.
        Texture tex;
        if (t) {
            CatArchive& cat = *archives_[t->ref.archive];
            if (cat.read(cat.entries()[t->ref.entry], data) || decodeTexture(data, tex))
                tex = Texture{};
        }
        modelTextures_.push_back(std::move(tex));
        modelTextureNames_.push_back(t ? t->stem : "?");
    }
}

void ModelViewer::resetView() {
    yaw_ = 35.0f * kPi / 180.0f;
    pitch_ = 25.0f * kPi / 180.0f;
    zoom_ = 1;
    dirty_ = true;
}

void ModelViewer::handleEvent(const Event& ev) {
    switch (ev.type) {
    case EventType::MouseDown:
        if (ev.button == MouseButton::Left) {
            dragging_ = true;
            dragX_ = ev.mouseX;
            dragY_ = ev.mouseY;
        }
        return;
    case EventType::MouseUp:
        if (ev.button == MouseButton::Left)
            dragging_ = false;
        return;
    case EventType::MouseMove:
        if (dragging_) {
            yaw_ += float(ev.mouseX - dragX_) * 0.01f;
            pitch_ = std::clamp(pitch_ + float(ev.mouseY - dragY_) * 0.01f, -kMaxPitch, kMaxPitch);
            dragX_ = ev.mouseX;
            dragY_ = ev.mouseY;
            dirty_ = true;
        }
        return;
    case EventType::MouseWheel:
        zoom_ = std::clamp(zoom_ * std::pow(0.9f, float(ev.wheelY)), 0.05f, 20.0f);
        dirty_ = true;
        return;
    case EventType::KeyDown: break;
    default: return;
    }

    const int n = int(visible_.size());
    switch (ev.key) {
    case 'f': applyFilter((filter_ + 1) % kFilterCount); return;
    case 'a': yaw_ -= kKeyTurn; break;
    case 'd': yaw_ += kKeyTurn; break;
    case 'w': pitch_ = std::min(pitch_ + kKeyTurn, kMaxPitch); break;
    case 's': pitch_ = std::max(pitch_ - kKeyTurn, -kMaxPitch); break;
    case '=':
    case '+': zoom_ = std::max(zoom_ * 0.8f, 0.05f); break;
    case '-': zoom_ = std::min(zoom_ * 1.25f, 20.0f); break;
    case 'r': resetView(); break;
    case key::Space: spin_ = !spin_; break;
    case 'u': uvCheck_ = !uvCheck_; break;
    case 'c': colourByTail_ = !colourByTail_; break;
    case 't': textured_ = !textured_; break;
    case 'g': alphaMode_ = (alphaMode_ + 1) % 3; break;
    case 'l': outlines_ = (outlines_ + 1) % 3; break;
    case 'b': faces_ = (faces_ + 1) % 3; break;
    case 'h': smooth_ = !smooth_; break;
    case 'p':
        if (!model_.parts.empty())
            soloPart_ = soloPart_ + 1 >= int(model_.parts.size()) ? -1 : soloPart_ + 1;
        break;
    default:
        if (n == 0)
            return;
        switch (ev.key) {
        case key::Right: show((current_ + 1) % n); return;
        case key::Left: show((current_ + n - 1) % n); return;
        case key::PageDown: show(std::min(current_ + 10, n - 1)); return;
        case key::PageUp: show(std::max(current_ - 10, 0)); return;
        case key::Home: show(0); return;
        case key::End: show(n - 1); return;
        default: return;
        }
    }
    dirty_ = true;
    updateTitle();
}

void ModelViewer::update(uint64_t nowMs) {
    if (spin_ && lastMs_) {
        yaw_ += float(nowMs - lastMs_) * kSpinPerMs;
        dirty_ = true;
    }
    lastMs_ = nowMs;
    if (dirty_) {
        render();
        dirty_ = false;
    }
}

void ModelViewer::updateTitle() {
    std::string title = std::string("New Worlds models [") + kFilterNames[filter_] + "] ";
    if (visible_.empty()) {
        sys_.setWindowTitle(title + status_ + " (Tab: textures)");
        return;
    }
    const EntryRef& ref = models_[visible_[size_t(current_)]];
    title += std::to_string(current_ + 1) + "/" + std::to_string(visible_.size()) + ": " +
             archives_[ref.archive]->entries()[ref.entry].name;
    if (!status_.empty()) {
        sys_.setWindowTitle(title + " (" + status_ + ")");
        return;
    }
    title += " (" + std::to_string(model_.faces.size()) + " faces, " + std::to_string(model_.parts.size()) +
             (model_.parts.size() == 1 ? " part" : " parts");
    if (soloPart_ >= 0)
        title += "; showing part " + std::to_string(soloPart_);
    if (std::all_of(modelTextureNames_.begin(), modelTextureNames_.end(),
                    [](const std::string& n) { return n == "?"; })) {
        title += "; no textures found";
    } else {
        title += "; textures";
        for (size_t i = 0; i < modelTextureNames_.size(); ++i)
            title += (i ? ", " : " ") + modelTextureNames_[i];
        if (!textured_)
            title += " (off)";
    }
    title += ")";
    if (outlines_)
        title += std::string(" [") + kOutlineNames[outlines_] + "]";
    if (faces_)
        title += std::string(" [") + kFaceNames[faces_] + "]";
    if (colourByTail_)
        title += " [colour: texture number]";
    if (!smooth_)
        title += " [flat shading]";
    if (alphaMode_) // glow is the default
        title += std::string(" [") + kAlphaNames[alphaMode_] + "]";
    if (uvCheck_)
        title += " [UV check]";
    sys_.setWindowTitle(title);
}

void ModelViewer::render() {
    const int w = screen_.w, h = screen_.h;
    for (int y = 0; y < h; ++y) { // dark blue-grey gradient
        uint8_t* row = screen_.row(y);
        const uint8_t v = uint8_t(20 + 30 * y / h);
        for (int x = 0; x < w; ++x, row += 4) {
            row[0] = v;
            row[1] = uint8_t(v + 4);
            row[2] = uint8_t(v + 12);
            row[3] = 255;
        }
    }
    if (!status_.empty() && !visible_.empty()) { // didn't decode: dark red
        for (int y = 0; y < h; ++y) {
            uint8_t* row = screen_.row(y);
            for (int x = 0; x < w; ++x, row += 4)
                row[0] = 80, row[1] = 0, row[2] = 0;
        }
        return;
    }

    // Orbit camera around the model's centre, z up.
    const OrbitCamera cam(Vec{centre_[0], centre_[1], centre_[2]}, yaw_, pitch_, size_ * 2.6f * zoom_, w, h);
    const Vec light = normalize(Vec{-0.4f, 0.5f, -0.75f}); // towards the light, in view space

    raster_.begin(screen_);
    std::vector<std::vector<ScreenVertex>> outlines;
    std::vector<ViewVertex> poly;
    std::vector<ScreenVertex> projected;
    // With alpha as transparency, faces whose texture has an alpha layer are
    // blended over the rest, so they're drawn in a second pass.
    for (int pass = 0; pass < 2; ++pass)
    for (const auto& face : model_.faces) {
        if (soloPart_ >= 0 && int(face.part) != soloPart_)
            continue;
        // The last value of the face's last corner picks its texture.
        const uint32_t textureIndex = face.corners.back().extra[4];
        const Texture* tex = nullptr;
        if (textured_ && !colourByTail_ && textureIndex < modelTextures_.size() &&
            !modelTextures_[textureIndex].rgba.empty())
            tex = &modelTextures_[textureIndex];
        const bool hasAlpha = tex && !tex->alpha.empty();
        const bool blended = hasAlpha && alphaMode_ == 2;
        if (blended != (pass == 1))
            continue;
        const auto& verts = model_.parts[face.part].vertices;

        // Face normal (Newell's method), in view space.
        Vec normal{0, 0, 0};
        poly.clear();
        for (size_t i = 0; i < face.corners.size(); ++i) {
            const auto& a = verts[face.corners[i].vertex];
            const auto& b = verts[face.corners[(i + 1) % face.corners.size()].vertex];
            normal.x += (a.y - b.y) * (a.z + b.z);
            normal.y += (a.z - b.z) * (a.x + b.x);
            normal.z += (a.x - b.x) * (a.y + b.y);
            poly.push_back(cam.toView(Vec{a.x, a.y, a.z}, face.corners[i].u, face.corners[i].v));
        }
        normal = normalize(normal);
        // Light levels per corner: from the corner's own normal (its first
        // extra value numbers the part's second normal list) when shading
        // smoothly, else from the face's.
        const auto& normalLists = model_.parts[face.part].normalLists;
        for (size_t i = 0; i < face.corners.size(); ++i) {
            const uint32_t k = face.corners[i].extra[0];
            Vec n = normal;
            if (smooth_ && normalLists.size() > 1 && k < normalLists[1].size())
                n = Vec{normalLists[1][k].x, normalLists[1][k].y, normalLists[1][k].z};
            const float d = std::fabs(dot(cam.directionToView(n), light));
            poly[i].s = tex ? 0.45f + 0.55f * d : 0.3f + 0.7f * d;
        }

        poly = clipNear(poly, cam.nearZ);
        if (poly.size() < 3)
            continue;
        projected.clear();
        for (const auto& p : poly)
            projected.push_back(cam.project(p));

        // Twice the projected area; positive when clockwise on the screen (y down).
        float area = 0;
        for (size_t i = 0; i < projected.size(); ++i) {
            const auto& a = projected[i];
            const auto& b = projected[(i + 1) % projected.size()];
            area += a.x * b.y - b.x * a.y;
        }
        const bool front = area > 0;
        if ((faces_ == 0 && !front) || (faces_ == 2 && front))
            continue;

        if (outlines_ != 2) {
            // Untextured: colour by part, or by the texture number.
            const uint8_t* base = kPartColours[(colourByTail_ ? textureIndex : face.part) % 8];
            const bool check = uvCheck_;
            auto shade = [&](float u, float v, float intensity, uint8_t* out) {
                float k = intensity;
                if (check && ((int(std::floor(u * 8)) + int(std::floor(v * 8))) & 1))
                    k *= 0.55f;
                const uint8_t* c = base;
                float a = 1;
                if (tex) { // nearest texel, wrapping
                    int tx = int(std::floor(u * float(tex->width))) % tex->width;
                    int ty = int(std::floor(v * float(tex->height))) % tex->height;
                    tx += tx < 0 ? tex->width : 0;
                    ty += ty < 0 ? tex->height : 0;
                    const size_t texel = size_t(ty) * size_t(tex->width) + size_t(tx);
                    c = &tex->rgba[texel * 4];
                    if (c[3] == 0)
                        return false; // colour key: nothing drawn
                    const float layer = hasAlpha ? float(tex->alpha[texel]) / 255.0f : 1.0f;
                    a = blended ? layer : 1.0f;
                    if (hasAlpha && alphaMode_ == 0) // glow: alpha keeps the texel lit
                        k = std::max(k, layer);
                    if (a <= 0)
                        return false;
                }
                for (int i = 0; i < 3; ++i)
                    out[i] = uint8_t(std::min(255.0f, c[i] * k * a + float(out[i]) * (1 - a)));
                out[3] = 255;
                return a > 0.98f; // only (nearly) opaque pixels hide what's behind
            };
            for (size_t i = 1; i + 1 < projected.size(); ++i)
                raster_.drawTriangle(projected[0], projected[i], projected[i + 1], shade);
        }
        if (outlines_ != 0)
            outlines.push_back(projected);
    }
    for (const auto& o : outlines)
        for (size_t i = 0; i < o.size(); ++i)
            drawLine(o[i].x, o[i].y, o[(i + 1) % o.size()].x, o[(i + 1) % o.size()].y);
}

void ModelViewer::drawLine(float x0, float y0, float x1, float y1) {
    const float dx = x1 - x0, dy = y1 - y0;
    const int steps = int(std::ceil(std::max(std::fabs(dx), std::fabs(dy))));
    if (steps > 4 * (screen_.w + screen_.h))
        return; // nearly behind the camera; not worth drawing
    for (int i = 0; i <= steps; ++i) {
        const float t = steps ? float(i) / float(steps) : 0;
        const int x = int(x0 + dx * t), y = int(y0 + dy * t);
        if (x < 0 || y < 0 || x >= screen_.w || y >= screen_.h)
            continue;
        uint8_t* p = screen_.row(y) + x * 4;
        p[0] = 240;
        p[1] = 240;
        p[2] = 200;
    }
}

} // namespace reluva::newworlds
