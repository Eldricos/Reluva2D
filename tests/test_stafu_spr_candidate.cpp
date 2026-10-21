// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_candidate.h"
#include <cassert>
#include <cstdint>
int main() {
    using namespace reluva::stafu;
    const uint8_t pairs[]={3,0x7f,2,0x42};
    auto r=evaluateSprCandidate(pairs,sizeof(pairs),5,SprCandidate::PairCountValue);
    assert(r.valid&&r.complete&&r.output==5);
    assert(!evaluateSprCandidate(pairs,sizeof(pairs),4,SprCandidate::PairCountValue).valid);
    assert(!evaluateSprCandidate(pairs,3,100,SprCandidate::PairCountValue).valid);
    const uint8_t pack[]={2,10,11,12,0xfe,33};
    r=evaluateSprCandidate(pack,sizeof(pack),6,SprCandidate::PackBits);
    assert(r.valid&&r.output==6);
    assert(!evaluateSprCandidate(pack,sizeof(pack),5,SprCandidate::PackBits).valid);
    const uint8_t high[]={1,10,11,0x82,9};
    r=evaluateSprCandidate(high,sizeof(high),5,SprCandidate::HighBitRuns);
    assert(r.valid&&r.output==5);
    assert(!evaluateSprCandidate(high,4,100,SprCandidate::HighBitRuns).valid);
    assert(evaluateSprCandidate(nullptr,0,0,SprCandidate::PackBits).valid);
    return 0;
}
