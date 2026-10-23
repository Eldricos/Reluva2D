// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_pair_relation.h"
#include "engines/stafu/spr_sequence_suffix.h"
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
    assert(same.valid&&same.exact&&same.prefixBytes==4&&same.suffixBytes==0);
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
    const uint8_t e[]={0x10,0x12,0x34,0x56};
    const uint8_t f[]={0x20,0x12,0x34,0x56};
    const uint8_t g[]={0x30,0x12,0x34,0x56};
    const std::vector<SprSequencePayload> group={{e,4},{f,4},{g,4}};
    const auto common=compareSprSequenceSuffix(group);
    assert(common.valid&&common.sharedBits>=24&&common.starts.size()==3);
    const uint8_t shiftedBytes[]={0xff,0x10,0x12,0x34,0x56};
    const std::vector<SprSequencePayload> shiftedGroup{{e,4},{f,4},{shiftedBytes,5}};
    const auto aligned=compareSprSequenceSuffix(shiftedGroup);
    assert(aligned.valid&&aligned.sharedBits>=24&&
           aligned.starts[2]>=aligned.starts[0]+8);
    const std::vector<SprSequencePayload> identicalGroup{{a,4},{a,4},{a,4}};
    const auto identical=compareSprSequenceSuffix(identicalGroup);
    assert(identical.valid&&identical.sharedBits==32);
    const std::vector<SprSequencePayload> invalidGroup{{nullptr,4},{a,4}};
    assert(!compareSprSequenceSuffix(invalidGroup).valid);
    const std::vector<SprSequencePayload> pairGroup{{a,4},{b,4}};
    assert(!compareSprSequenceSuffix(pairGroup,0).valid);
    const std::vector<SprSequencePayload> singleGroup{{a,4}};
    assert(!compareSprSequenceSuffix(singleGroup).valid);
    assert(!compareSprPair(nullptr,4,a,4).valid);
    assert(!compareSprPair(a,0,a,4).valid);
}
