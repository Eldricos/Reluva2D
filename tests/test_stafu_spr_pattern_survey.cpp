// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_pattern_survey.h"
#include <cassert>
#include <cstdint>
int main() {
    using namespace reluva::stafu;
    const uint8_t bytes[]={0xaa,0xaa,0xaa,0xaa};
    const auto row=surveySprPattern(bytes,4,0,8,0);
    assert(row.tokens==4&&row.distinct==1&&row.adjacentEqual==3&&row.mostFrequent==4);
    const auto shifted=surveySprPattern(bytes,4,1,8,0);
    assert(shifted.tokens==3&&shifted.distinct==1);
    assert(surveySprPattern(bytes,4,0,0,0).tokens==0);
    assert(surveySprPattern(bytes,4,0,8,8).tokens==0);
    assert(surveySprPattern(nullptr,4,0,8,0).tokens==0);
    assert(surveySprPattern(bytes,4,100,8,0).tokens==0);
    const auto repeat=longestSprRepeat(bytes,4,0,8,32);
    assert(repeat.period==2&&repeat.repeats==16);
    assert(longestSprRepeat(bytes,4,32).repeats==0);
    return 0;
}
