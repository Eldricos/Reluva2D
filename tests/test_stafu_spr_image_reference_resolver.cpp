// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_image_reference_resolver.h"
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
namespace {
void put32(std::vector<uint8_t>& b,size_t p,uint32_t n) {
    for(unsigned i=0;i<4;++i)b[p+i]=static_cast<uint8_t>(n>>(8*i));
}
void tag(std::vector<uint8_t>& b,size_t p,const char* s) {
    for(unsigned i=0;i<4;++i)b[p+i]=static_cast<uint8_t>(s[i]);
}
std::vector<uint8_t> fixture(const char* secondTag,uint32_t distance) {
    std::vector<uint8_t> b(103,0);
    tag(b,0,"TRPS");put32(b,4,103);put32(b,8,0x100);
    tag(b,12,"TSIL");put32(b,16,91);put32(b,20,1);put32(b,24,16);
    tag(b,28,"TATS");put32(b,32,8);
    tag(b,36,"EMIT");put32(b,40,12);put32(b,44,600);
    tag(b,48,"KRAM");put32(b,52,8);
    tag(b,56,"PMOC");put32(b,60,23);
    put32(b,64,18);put32(b,68,16);put32(b,72,0x2000d);
    b[76]=1;b[77]=2;b[78]=3;
    tag(b,79,secondTag);put32(b,83,24);
    put32(b,87,18);put32(b,91,16);put32(b,95,0x30000);
    put32(b,99,distance);
    return b;
}
}
int main() {
    using Resolver=reluva::stafu::SprImageReferenceResolver;
    reluva::stafu::SprSequenceIndex index;
    std::string error;
    auto b=fixture("PMOC",23);
    assert(index.build(b,error));
    auto r=Resolver::resolve(b,index,index.records()[0].events.back());
    assert(r.status==Resolver::Status::Resolved);
    assert(r.targetOffset==76&&r.size==3&&r.data[0]==1&&r.data[2]==3);
    assert(r.metadataCompatible);
    auto plain=Resolver::resolve(b,index,index.records()[0].events[3]);
    assert(plain.status==Resolver::Status::NotReference);
    b=fixture("MOCS",23);
    assert(index.build(b,error));
    r=Resolver::resolve(b,index,index.records()[0].events.back());
    assert(r.status==Resolver::Status::MetadataMismatch);
    assert(r.targetOffset==76&&!r.metadataCompatible&&r.data==nullptr);
    b=fixture("PMOC",22);
    assert(index.build(b,error));
    r=Resolver::resolve(b,index,index.records()[0].events.back());
    assert(r.status==Resolver::Status::MissingTarget);
    b=fixture("PMOC",100);
    assert(index.build(b,error));
    r=Resolver::resolve(b,index,index.records()[0].events.back());
    assert(r.status==Resolver::Status::Underflow);
    return 0;
}
