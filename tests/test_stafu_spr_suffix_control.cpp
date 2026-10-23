// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_suffix_control.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>
int main() {
    using namespace reluva::stafu;
    const uint8_t a[]={0x11,0x22,0x33,0x00};
    const uint8_t b[]={0x44,0x22,0x33,0x00};
    const auto r=inspectSprSuffixControl(a,4,b,4);
    assert(r.matchedBits>=24&&r.trimmedZero);
    const auto x=shuffledSprBytes(a,4,123u);
    const auto y=shuffledSprBytes(a,4,123u);
    assert(x==y&&x.size()==4);
    std::vector<uint8_t> sorted=x;
    std::sort(sorted.begin(),sorted.end());
    assert((sorted==std::vector<uint8_t>{0,0x11,0x22,0x33}));
    assert(shuffledSprBytes(nullptr,4).empty());
    assert(shuffledSprBytes(a,0).empty());
}
