// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/computer_navigation_index.h"
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
namespace {
int failures=0;
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);++failures;}}while(0)
void w16(std::vector<uint8_t>& b,size_t p,uint16_t v) {
    b[p]=uint8_t(v);b[p+1]=uint8_t(v>>8);
}
void w32(std::vector<uint8_t>& b,size_t p,uint32_t v) {
    for(size_t i=0;i<4;++i)b[p+i]=uint8_t(v>>(8*i));
}
std::vector<uint8_t> fixture() {
    // 3 records, base=0x14, metadata offsets 0, 40, 80.
    std::vector<uint8_t> b(160,0);
    w32(b,0,3);w32(b,4,0);w32(b,8,40);w32(b,12,80);
    const size_t base=20;
    w16(b,base,2);w16(b,base+2,2);w32(b,base+16,0x1234);
    w32(b,base+20,40);w32(b,base+24,80);
    w16(b,base+40,1);w32(b,base+40+20,80);
    w16(b,base+80,1);w32(b,base+80+20,40);
    return b;
}
}
int main() {
    using reluva::stafu::ComputerNavigationIndex;
    std::string error;
    ComputerNavigationIndex index;
    auto b=fixture();
    CHECK(index.parse(b,error));
    CHECK(index.nodes().size()==3);
    CHECK(index.edges()==4);
    CHECK(index.sharedTargets()==2);
    CHECK(index.reachableFromRoot()==3);
    CHECK(index.hasCycle());
    CHECK(index.nodes()[0].headerB==2);
    CHECK(index.nodes()[0].opaqueWord==0x1234);
    CHECK(index.nodes()[0].children[0]==1);
    CHECK(index.nodes()[0].children[1]==2);
    w32(b,20+20,0xdeadbeef);
    CHECK(!index.parse(b,error));
    CHECK(index.nodes().empty());
    b=fixture();w16(b,20,20);
    CHECK(!index.parse(b,error));
    b=fixture();w16(b,20+40,0);w16(b,20+80,0);
    CHECK(index.parse(b,error));
    CHECK(!index.hasCycle());
    CHECK(index.sharedTargets()==0);
    CHECK(index.reachableFromRoot()==3);
    return failures?1:0;
}
