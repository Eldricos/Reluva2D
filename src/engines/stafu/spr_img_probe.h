// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstddef>
#include <cstdint>
namespace reluva::stafu {
struct SprImgProbe {
 bool gifSignature=false,logicalScreen=false,globalTableFits=false,firstBlockValid=false;
 uint16_t width=0,height=0;
 uint8_t packed=0,background=0,aspect=0,firstBlock=0;
 size_t tableBytes=0,firstBlockOffset=0;
};
inline SprImgProbe probeSprImg(const uint8_t* b,size_t n){
 SprImgProbe r;
 if(!b||n<6)return r;
 r.gifSignature=(b[0]=='G'&&b[1]=='I'&&b[2]=='F'&&b[3]=='8'&&(b[4]=='7'||b[4]=='9')&&b[5]=='a');
 if(!r.gifSignature||n<13)return r;
 r.logicalScreen=true;
 r.width=uint16_t(uint16_t(b[6])|(uint16_t(b[7])<<8));
 r.height=uint16_t(uint16_t(b[8])|(uint16_t(b[9])<<8));
 r.packed=b[10];r.background=b[11];r.aspect=b[12];
 r.tableBytes=(r.packed&0x80u)?size_t(3)*(size_t(1)<<(1+(r.packed&7u))):0;
 if(r.tableBytes>n-13)return r;
 r.globalTableFits=true;r.firstBlockOffset=13+r.tableBytes;
 if(r.firstBlockOffset<n){
  r.firstBlock=b[r.firstBlockOffset];
  r.firstBlockValid=r.firstBlock==0x2cu||r.firstBlock==0x21u||r.firstBlock==0x3bu;
 }
 return r;
}
}
