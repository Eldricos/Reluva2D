// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Milestone 211: bridge palette verified against the original DOS runtime.
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <utility>

namespace reluva::stafu {
inline constexpr size_t kBridgePaletteHalfBytes = 384;
inline constexpr size_t kBridgePaletteBytes = 768;
inline constexpr size_t kBridgePixels = 640u * 480u;

inline uint8_t bridgeExpandVga6(uint8_t value) {
    return static_cast<uint8_t>((unsigned(value) * 255u + 31u) / 63u);
}

inline bool assembleBridgePalette(const std::vector<uint8_t>& bridgeRm,
                                  const std::vector<uint8_t>& standardPal,
                                  std::array<uint8_t, kBridgePaletteBytes>& palette) {
    if (bridgeRm.size() != kBridgePaletteHalfBytes + kBridgePixels ||
        standardPal.size() != kBridgePaletteHalfBytes)
        return false;
    std::array<uint8_t, kBridgePaletteBytes> candidate{};
    for (size_t i = 0; i < kBridgePaletteHalfBytes; ++i) {
        candidate[i] = bridgeRm[i];
        candidate[kBridgePaletteHalfBytes + i] = standardPal[i];
    }
    for (uint8_t component : candidate)
        if (component > 63) return false;
    palette = candidate;
    return true;
}

inline bool renderBridgeRgba(const std::vector<uint8_t>& bridgeRm,
                            const std::array<uint8_t, kBridgePaletteBytes>& palette,
                            std::vector<uint8_t>& rgba) {
    if (bridgeRm.size() != kBridgePaletteHalfBytes + kBridgePixels) return false;
    std::vector<uint8_t> candidate(kBridgePixels * 4);
    for (size_t i = 0; i < kBridgePixels; ++i) {
        const size_t color = size_t(bridgeRm[kBridgePaletteHalfBytes + i]) * 3;
        const size_t pixel = i * 4;
        candidate[pixel] = bridgeExpandVga6(palette[color]);
        candidate[pixel + 1] = bridgeExpandVga6(palette[color + 1]);
        candidate[pixel + 2] = bridgeExpandVga6(palette[color + 2]);
        candidate[pixel + 3] = 255;
    }
    rgba = std::move(candidate);
    return true;
}
} // namespace reluva::stafu
