#include "engines/stafu/bridge_palette.h"
#include <cassert>
#include <vector>

int main() {
    using namespace reluva::stafu;
    std::vector<uint8_t> bridge(384 + kBridgePixels, 0);
    std::vector<uint8_t> standard(384, 0);
    bridge[0] = 43; bridge[1] = 26; bridge[2] = 19;
    standard[0] = 63; standard[1] = 31; standard[2] = 0;
    bridge[384] = 0;
    bridge[385] = 128;
    bridge[386] = 255;
    standard[381] = 1; standard[382] = 2; standard[383] = 3;
    std::array<uint8_t, 768> palette{};
    assert(assembleBridgePalette(bridge, standard, palette));
    assert(palette[0] == 43 && palette[1] == 26 && palette[2] == 19);
    assert(palette[384] == 63 && palette[385] == 31 && palette[386] == 0);
    assert(palette[765] == 1 && palette[766] == 2 && palette[767] == 3);
    std::vector<uint8_t> rgba;
    assert(renderBridgeRgba(bridge, palette, rgba));
    assert(rgba.size() == kBridgePixels * 4);
    assert(rgba[0] == bridgeExpandVga6(43) && rgba[3] == 255);
    assert(rgba[4] == 255 && rgba[5] == bridgeExpandVga6(31) && rgba[6] == 0);
    assert(rgba[8] == bridgeExpandVga6(1) && rgba[9] == bridgeExpandVga6(2));
    // The 384-byte palette header must never be treated as image pixels.
    // Check the first two adjacent pixels, the last pixel of row zero,
    // and the first pixel of row one without any geometry rotation.
    bridge[0] = 12; // Palette component, not the first pixel index.
    bridge[384] = 128;
    bridge[384 + 639] = 255;
    bridge[384 + 640] = 0;
    assert(renderBridgeRgba(bridge, palette, rgba));
    assert(rgba[0] == 255 && rgba[1] == bridgeExpandVga6(31));
    assert(rgba[4] == 255 && rgba[5] == bridgeExpandVga6(31));
    assert(rgba[639 * 4] == bridgeExpandVga6(1));
    assert(rgba[640 * 4] == bridgeExpandVga6(43));
    assert(rgba[640 * 4 + 1] == bridgeExpandVga6(26));
    auto bad = standard; bad[0] = 64;
    assert(!assembleBridgePalette(bridge, bad, palette));
    bad = standard; bad.pop_back();
    assert(!assembleBridgePalette(bridge, bad, palette));
    bridge.pop_back();
    assert(!assembleBridgePalette(bridge, standard, palette));
    assert(!renderBridgeRgba(bridge, palette, rgba));
}
