// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_image_payload.h"
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
namespace {
void put32(std::vector<uint8_t>& b,size_t p,uint32_t n) {
    for(unsigned i=0;i<4;++i)b[p+i]=static_cast<uint8_t>(n>>(8*i));
}
void tag(std::vector<uint8_t>& b,size_t p,const char* t) {
    for(unsigned i=0;i<4;++i)b[p+i]=static_cast<uint8_t>(t[i]);
}
}
int main() {
    // One record: PMOC (three opaque bytes), EMIT (four bytes), MOCS (zero opaque bytes).
    std::vector<uint8_t> bytes(87,0);
    tag(bytes,0,"TRPS");put32(bytes,4,87);put32(bytes,8,0x100);
    tag(bytes,12,"TSIL");put32(bytes,16,75);put32(bytes,20,1);put32(bytes,24,16);
    tag(bytes,28,"PMOC");put32(bytes,32,23);
    put32(bytes,36,18);put32(bytes,40,16);put32(bytes,44,0x2000d);
    bytes[48]=0x10;bytes[49]=0x20;bytes[50]=0x30;
    tag(bytes,51,"EMIT");put32(bytes,55,12);put32(bytes,59,466);
    tag(bytes,63,"MOCS");put32(bytes,67,20);
    put32(bytes,71,20);put32(bytes,75,24);put32(bytes,79,0x1000d);
    // Last four bytes belong to a separate unknown chunk? Instead make MOCS payload 16.
    // Correct the MOCS chunk to 24 bytes with four opaque bytes.
    put32(bytes,67,24);
    bytes[83]=1;bytes[84]=2;bytes[85]=3;bytes[86]=4;
    reluva::stafu::SprSequenceIndex index;
    std::string error;
    assert(index.build(bytes,error)&&error.empty());
    const auto& events=index.records()[0].events;
    assert(events.size()==3);
    reluva::stafu::SprImagePayloadAccessor::View view;
    using Access=reluva::stafu::SprImagePayloadAccessor;
    assert(Access::get(bytes,events[0],view));
    assert(view.offset==48&&view.size==3&&view.data==bytes.data()+48);
    assert(view.data[0]==0x10&&view.data[1]==0x20&&view.data[2]==0x30);
    assert(view.headerA==18&&view.headerB==16&&view.marker==0x2000d);
    assert(!Access::get(bytes,events[1],view)&&view.data==nullptr&&view.size==0);
    assert(Access::get(bytes,events[2],view)&&view.offset==83&&view.size==4);
    assert(view.data[3]==4&&view.marker==0x1000d);
    auto tampered=events[0];
    tampered.imageDataSize=99999;
    assert(!Access::get(bytes,tampered,view)&&!view.data);
    tampered=events[0];tampered.imageDataOffset=0;
    assert(!Access::get(bytes,tampered,view));
    tampered=events[0];tampered.headerA=999;
    assert(!Access::get(bytes,tampered,view));
    auto shortened=bytes;shortened.resize(50);
    assert(!Access::get(shortened,events[0],view));
    auto changed=bytes;changed[28]='X';
    assert(!Access::get(changed,events[0],view));
    return 0;
}
