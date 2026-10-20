// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/computer_browser.h"
#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>
namespace {
int failures=0;
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);++failures;}}while(0)
void w16(std::vector<uint8_t>& b,size_t p,uint16_t v) {
    b[p]=uint8_t(v);b[p+1]=uint8_t(v>>8);
}
void w32(std::vector<uint8_t>& b,size_t p,uint32_t v) {
    for(size_t i=0;i<4;++i)b[p+i]=uint8_t(v>>(8*i));
}
std::vector<uint8_t> fixture() {
    // Four records, base=24, primary offsets 0,40,80,120.
    std::vector<uint8_t> b(200,0);
    w32(b,0,4);w32(b,4,0);w32(b,8,40);w32(b,12,80);w32(b,16,120);
    const size_t base=24;
    w16(b,base,2);w32(b,base+20,40);w32(b,base+24,80);
    w16(b,base+40,1);w32(b,base+40+20,120);
    w16(b,base+80,1);w32(b,base+80+20,120);
    return b;
}
}
int main() {
    reluva::stafu::ComputerNavigationIndex graph;
    std::string error;
    CHECK(graph.parse(fixture(),error));
    CHECK(graph.sharedTargets()==1);
    reluva::stafu::ComputerBrowser browser(graph);
    CHECK(browser.depth()==1&&browser.current()==0);
    CHECK(!browser.back());
    CHECK(!browser.select(0));
    CHECK(!browser.select(3));
    CHECK(browser.select(1)&&browser.current()==1);
    CHECK(browser.select(1)&&browser.current()==3);
    CHECK(!browser.select(1));
    CHECK(browser.back()&&browser.current()==1);
    CHECK(browser.back()&&browser.current()==0);
    CHECK(browser.select(2)&&browser.current()==2);
    CHECK(browser.select(1)&&browser.current()==3);
    CHECK(browser.path().size()==3&&browser.path()[1]==2);
    CHECK(browser.home()&&browser.current()==0&&browser.depth()==1);
    CHECK(!browser.back());
    return failures?1:0;
}
