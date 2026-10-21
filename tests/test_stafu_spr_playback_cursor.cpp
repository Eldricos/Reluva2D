// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/spr_playback_cursor.h"
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
    // Two records: first contains EMIT, KRAM, PMOC; second is empty.
    std::vector<uint8_t> bytes(76,0);
    tag(bytes,0,"TRPS");put32(bytes,4,76);put32(bytes,8,0x100);
    tag(bytes,12,"TSIL");put32(bytes,16,64);put32(bytes,20,2);
    put32(bytes,24,20);put32(bytes,28,56);
    tag(bytes,32,"EMIT");put32(bytes,36,12);put32(bytes,40,466);
    tag(bytes,44,"KRAM");put32(bytes,48,8);
    tag(bytes,52,"PMOC");put32(bytes,56,16);put32(bytes,60,7);
    tag(bytes,68,"TATS");put32(bytes,72,8);
    reluva::stafu::SprSequenceIndex index;
    std::string error;
    assert(index.build(bytes,error));
    reluva::stafu::SprPlaybackCursor cursor;
    assert(!cursor.bound()&&cursor.atEnd()&&!cursor.current());
    assert(!cursor.advance()&&cursor.position()==0);
    assert(!cursor.bind(index,2)&&!cursor.bound());
    assert(cursor.bind(index,0)&&cursor.bound());
    assert(cursor.size()==3&&cursor.position()==0);
    assert(cursor.current()&&cursor.current()->tag=="EMIT");
    assert(cursor.current()->hasRawControlValue&&cursor.current()->rawControlValue==466);
    assert(cursor.advance()&&cursor.current()&&cursor.current()->tag=="KRAM");
    assert(cursor.advance()&&cursor.current()&&cursor.current()->tag=="PMOC");
    assert(cursor.advance()&&cursor.atEnd()&&!cursor.current());
    assert(cursor.position()==3&&!cursor.advance()&&cursor.position()==3);
    assert(!cursor.bind(index,999)&&cursor.atEnd()&&cursor.position()==3);
    cursor.restart();
    assert(cursor.position()==0&&cursor.current()->tag=="EMIT");
    assert(cursor.bind(index,1)&&cursor.size()==1&&cursor.current()->tag=="TATS");
    cursor.unbind();
    assert(!cursor.bound()&&cursor.atEnd()&&!cursor.current()&&cursor.size()==0);
    return 0;
}
