// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_middle_blocks.h"
#include <cassert>
#include <cstdint>
#include <vector>
int main() {
    using reluva::stafu::findSprMiddleBlocks;
    std::vector<uint8_t> a;
    for(int i=0;i<96;++i)a.push_back(static_cast<uint8_t>(i+1));
    auto b=a;
    b.insert(b.begin()+20,{201,202,203});
    auto blocks=findSprMiddleBlocks(a.data(),a.size(),b.data(),b.size(),4,0,8,16);
    bool found=false;
    for(const auto& m:blocks)
        if(m.shift==3&&m.a==20&&m.b==23&&m.length==76)found=true;
    assert(found);
    auto equal=findSprMiddleBlocks(a.data(),a.size(),a.data(),a.size(),4,4,8,16);
    assert(!equal.empty()&&equal[0].shift==0&&equal[0].length==88);
    auto none=findSprMiddleBlocks(a.data(),a.size(),b.data(),b.size(),99,0);
    assert(none.empty());
    assert(findSprMiddleBlocks(nullptr,1,b.data(),b.size(),0,0).empty());
    assert(findSprMiddleBlocks(a.data(),a.size(),b.data(),b.size(),0,0,-1).empty());
}
