// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_byte_shift.h"
#include <cassert>
#include <cstdint>
#include <vector>

int main() {
    using reluva::stafu::surveySprByteShifts;
    const std::vector<uint8_t> base={10,20,30,40,50,60,70,80,90,100,110,120};
    auto same=surveySprByteShifts(base.data(),base.size(),base.data(),base.size());
    assert(!same.empty()&&same[0].shift==0&&same[0].length==base.size());
    // Three-byte insertion in B after a two-byte common prefix.
    auto inserted=base;
    inserted.insert(inserted.begin()+2,{201,202,203});
    auto plus=surveySprByteShifts(base.data(),base.size(),inserted.data(),inserted.size());
    assert(!plus.empty()&&plus[0].shift==3&&plus[0].startA==2&&plus[0].startB==5&&plus[0].length==10);
    // Three-byte deletion in B after a two-byte common prefix.
    auto deleted=base;
    deleted.erase(deleted.begin()+2,deleted.begin()+5);
    auto minus=surveySprByteShifts(base.data(),base.size(),deleted.data(),deleted.size());
    assert(!minus.empty()&&minus[0].shift==-3&&minus[0].startA==5&&minus[0].startB==2&&minus[0].length==7);
    // Unrelated arrays and empty arrays must not produce invented matches.
    const std::vector<uint8_t> unrelated(base.size(),255);
    auto noMatch=surveySprByteShifts(base.data(),base.size(),unrelated.data(),unrelated.size());
    assert(!noMatch.empty()&&noMatch[0].length==0);
    auto empty=surveySprByteShifts(nullptr,0,nullptr,0);
    assert(empty.size()==65&&empty[0].length==0);
    assert(surveySprByteShifts(nullptr,1,base.data(),base.size()).empty());
    assert(surveySprByteShifts(base.data(),base.size(),base.data(),base.size(),-1).empty());
}
