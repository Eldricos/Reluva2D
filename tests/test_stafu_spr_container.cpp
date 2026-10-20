// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_container.h"
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
namespace {
void put32(std::vector<uint8_t>& b,size_t p,uint32_t n) {
    for(unsigned i=0;i<4;++i)b[p+i]=uint8_t(n>>(8*i));
}
void tag(std::vector<uint8_t>& b,size_t p,const char* t) {
    for(unsigned i=0;i<4;++i)b[p+i]=uint8_t(t[i]);
}
}
int main() {
    // 12-byte header, 12-byte TSIL prefix, two offsets, two 8-byte chunks.
    std::vector<uint8_t> bytes(48,0);
    tag(bytes,0,"TRPS");put32(bytes,4,48);put32(bytes,8,0x100);
    tag(bytes,12,"TSIL");put32(bytes,16,36);put32(bytes,20,2);
    put32(bytes,24,20);put32(bytes,28,28);
    tag(bytes,32,"TATS");put32(bytes,36,8);
    tag(bytes,40,"TIXE");put32(bytes,44,8);
    reluva::stafu::SprContainer parser;
    std::string error;
    assert(parser.parse(bytes,error));
    assert(parser.records().size()==2);
    assert(parser.records()[0].start==32);
    assert(parser.records()[1].chunks[0].tag=="TIXE");
    assert(parser.versionOrFlags()==0x100);
    auto bad=bytes;
    put32(bad,28,20);assert(!parser.parse(bad,error));assert(parser.records().empty());
    bad=bytes;put32(bad,36,100);assert(!parser.parse(bad,error));
    bad=bytes;put32(bad,4,47);assert(!parser.parse(bad,error));
    bad=bytes;put32(bad,20,9999);assert(!parser.parse(bad,error));
    bad=bytes;put32(bad,28,100);assert(!parser.parse(bad,error));
    assert(parser.parse(bytes,error));
    return 0;
}
