// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_pair_relation.h"
#include <cassert>
#include <cstdint>
int main() {
    using namespace reluva::stafu;
    const uint8_t a[]={0x12,0x34,0x56,0x78};
    const uint8_t b[]={0x99,0x34,0x56,0x78};
    const auto r=compareSprPair(a,4,b,4);
    assert(r.valid&&!r.exact&&r.prefixBytes==0&&r.suffixBytes==3);
    assert(r.sharedSuffixBits>=24&&r.trimmedZero);
    assert(r.middleA==1&&r.middleB==1);
    assert(r.suffixStartBitA<=8&&r.suffixStartBitB<=8);
    const auto same=compareSprPair(a,4,a,4);
    assert(same.valid&&same.exact&&same.prefixBytes==4&&same.suffixBytes==4);
    assert(same.fullSmallerSuffix);
    assert(same.middleA==0&&same.middleB==0&&same.suffixBytes==0);
    assert(same.suffixOverlapsPrefix);
    const uint8_t c[]={0x00,0x12,0x34,0x56,0x78};
    const auto shifted=compareSprPair(a,4,c,5);
    assert(shifted.valid&&!shifted.exact&&shifted.suffixBytes==4);
    assert(shifted.fullSmallerSuffix);
    assert(shifted.middleA==0&&shifted.middleB==1);
    const uint8_t d[]={0x12,0x34,0xaa,0x78};
    const auto changed=compareSprPair(a,4,d,4);
    assert(changed.prefixBytes==2&&changed.suffixBytes==1);
    assert(changed.middleA==1&&changed.middleB==1);
    assert(!compareSprPair(nullptr,4,a,4).valid);
    assert(!compareSprPair(a,0,a,4).valid);
}
