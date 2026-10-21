// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_sequence_index.h"
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
    // One record: unknown chunk, EMIT value, empty KRAM, PMOC header + 3 opaque bytes.
    std::vector<uint8_t> bytes(79,0);
    tag(bytes,0,"TRPS");put32(bytes,4,79);put32(bytes,8,0x100);
    tag(bytes,12,"TSIL");put32(bytes,16,67);put32(bytes,20,1);
    put32(bytes,24,16);
    tag(bytes,28,"TATS");put32(bytes,32,8);
    tag(bytes,36,"EMIT");put32(bytes,40,12);put32(bytes,44,600);
    tag(bytes,48,"KRAM");put32(bytes,52,8);
    tag(bytes,56,"PMOC");put32(bytes,60,23);
    put32(bytes,64,18);put32(bytes,68,16);put32(bytes,72,0x2000d);
    bytes[76]=1;bytes[77]=2;bytes[78]=3;
    reluva::stafu::SprSequenceIndex index;
    std::string error;
    assert(index.build(bytes,error)&&error.empty());
    assert(index.records().size()==1);
    const auto& record=index.records()[0];
    assert(record.start==28&&record.end==79&&record.events.size()==4);
    assert(record.events[0].tag=="TATS"&&!record.events[0].hasRawControlValue);
    assert(record.events[1].tag=="EMIT"&&record.events[1].hasRawControlValue);
    assert(record.events[1].rawControlValue==600);
    assert(record.events[2].tag=="KRAM"&&record.events[2].payloadSize==0);
    assert(!record.events[2].hasRawControlValue);
    const auto& image=record.events[3];
    assert(image.hasImageHeader&&image.headerA==18&&image.headerB==16);
    assert(image.marker==0x2000d&&image.imageDataOffset==76&&image.imageDataSize==3);
    auto invalid=bytes;
    put32(invalid,60,100);
    assert(!index.build(invalid,error)&&!error.empty());
    assert(index.records().empty());
    assert(index.build(bytes,error)&&index.records().size()==1);
    return 0;
}
