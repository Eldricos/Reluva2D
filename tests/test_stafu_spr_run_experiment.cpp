// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_run_experiment.h"
#include <cassert>
#include <cstdint>
int main() {
    using namespace reluva::stafu;
    // Gamma: 1, 010, 011 = runs 1,2,3 (6 alternating symbols).
    const uint8_t gamma[]={0xa6}; // 10100110: 1 | 010 | 011 | 0
    const auto a=trySprRunDecode(gamma,1,0,SprRunCode::Gamma);
    assert(a.valid&&a.complete&&a.runs==3&&a.outputSymbols==6);
    assert(a.alternatingSymbols.size()==6);
    assert(a.alternatingSymbols[0]==0&&a.alternatingSymbols[1]==1);
    assert(a.alternatingSymbols[3]==0);
    // Rice k=0: 1 | 01 | 001 => runs 1,2,3.
    const uint8_t rice[]={0xa4}; // 10100100: 1 | 01 | 001 | 00
    const auto b=trySprRunDecode(rice,1,0,SprRunCode::Rice,0);
    assert(b.valid&&b.complete&&b.runs==3&&b.outputSymbols==6);
    assert(!trySprRunDecode(gamma,1,0,SprRunCode::Gamma,0,5).valid);
    assert(!trySprRunDecode(nullptr,1,0,SprRunCode::Gamma).valid);
    assert(!trySprRunDecode(gamma,1,9,SprRunCode::Gamma).valid);
    assert(!trySprRunDecode(gamma,1,0,SprRunCode::Rice,9).valid);
}
