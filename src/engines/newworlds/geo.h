// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 The Reluva contributors

#pragma once

#include "common/types.h"

#include <cstdint>
#include <string>
#include <vector>

namespace reluva::newworlds {

// A .geo model (chunk tag 0x17). See docs/formats/newworlds/geo.md; parts of
// the format are still unknown and are skipped.
struct GeoModel {
    struct Vec3 {
        float x, y, z;
    };
    struct Corner {
        uint32_t vertex;    // index into the face's part's vertices
        float u, v;         // texture coordinates, 0..1
        uint32_t extra[5];  // extra[0]: the corner's normal, in the part's second
                            // normal list; in parts above 0, extra[4] is the part
                            // number except in a face's last corner
    };
    struct Face {
        uint32_t groupA = 0, groupB = 0; // header values: the previous face's part, and unknown
        uint32_t part = 0;               // which part the face belongs to
        std::vector<Corner> corners;
    };
    struct Edge {
        uint32_t v0, v1;       // the part's vertices
        uint32_t faceA, faceB; // the faces on either side (0-based, model-wide)
    };
    // A separately numbered piece of the model (most models have one).
    struct Part {
        uint32_t unknown = 0; // first value: 0, or 1 in ptank_l's part 1
        std::vector<Vec3> vertices;
        std::vector<std::vector<Vec3>> normalLists;
        std::vector<Edge> edges;
    };

    float centreX = 0, centreY = 0, radius = 0;
    std::vector<Face> faces;
    std::vector<Part> parts;
};

Error parseGeo(const std::vector<uint8_t>& data, GeoModel& out);

} // namespace reluva::newworlds
