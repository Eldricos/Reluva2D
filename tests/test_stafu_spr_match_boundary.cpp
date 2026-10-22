// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_match_boundary.h"
#include <cassert>
#include <cstdint>
int main() {
    using reluva::stafu::sprBitWindow;
    const uint8_t data[]={0x80,0x01};
    assert(sprBitWindow(data,2,0,0,3)=="|100");
    assert(sprBitWindow(data,2,8,0,2)=="00|00");
    assert(sprBitWindow(data,2,16,0,3)=="001|");
    assert(sprBitWindow(data,2,0,1,3)=="|000");
    assert(sprBitWindow(nullptr,2,0,0).empty());
    assert(sprBitWindow(data,2,17,0).empty());
    return 0;
}
