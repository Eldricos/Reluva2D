// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Bounded GIF87a/GIF89a structure parser. Does not decompress LZW pixels.
#include "engines/stafu/spr_img_probe.h"
#include <cstddef>
#include <cstdint>
#include <string>
namespace reluva::stafu {
struct SprGifStructure {
 bool valid=false,trailer=false;
 size_t images=0,extensions=0,imageSubBlocks=0,extensionSubBlocks=0;
 size_t imageDataBytes=0,extensionDataBytes=0,trailerOffset=0,trailingBytes=0,trailingNonzero=0;
 uint8_t firstLzwMinimum=0;
 std::string error;
};
inline SprGifStructure inspectSprGifStructure(const uint8_t* b,size_t n){
 SprGifStructure r;
 const auto h=probeSprImg(b,n);
 if(!h.gifSignature||!h.logicalScreen||!h.globalTableFits){r.error="Invalid or truncated GIF logical screen / global palette";return r;}
 if(h.width==0||h.height==0){r.error="Zero logical screen dimension";return r;}
 size_t p=h.firstBlockOffset;
 auto fail=[&](const char* reason){r.error=reason;return r;};
 auto subBlocks=[&](bool image)->bool {
  for(;;){
   if(p>=n){r.error="Truncated sub-block length";return false;}
   const size_t len=b[p++];
   if(len==0)return true;
   if(len>n-p){r.error="Truncated sub-block payload";return false;}
   if(image){++r.imageSubBlocks;r.imageDataBytes+=len;}
   else{++r.extensionSubBlocks;r.extensionDataBytes+=len;}
   p+=len;
  }
 };
 while(p<n){
  const uint8_t kind=b[p++];
  if(kind==0x3b){
   r.trailer=true;r.trailerOffset=p-1;r.trailingBytes=n-p;
   for(;p<n;++p)r.trailingNonzero+=b[p]!=0;
   r.valid=true;return r;
  }
  if(kind==0x21){
   if(p>=n)return fail("Truncated extension label");
   ++p;++r.extensions;
   if(!subBlocks(false))return r;
   continue;
  }
  if(kind==0x2c){
   if(n-p<9)return fail("Truncated image descriptor");
   const uint16_t width=uint16_t(uint16_t(b[p+4])|(uint16_t(b[p+5])<<8));
   const uint16_t height=uint16_t(uint16_t(b[p+6])|(uint16_t(b[p+7])<<8));
   const uint8_t flags=b[p+8];p+=9;
   if(width==0||height==0)return fail("Zero image descriptor dimension");
   if(flags&0x80u){
    const size_t tableBytes=size_t(3)*(size_t(1)<<(1+(flags&7u)));
    if(tableBytes>n-p)return fail("Truncated local color table");
    p+=tableBytes;
   }
   if(p>=n)return fail("Missing LZW minimum code size");
   const uint8_t minimum=b[p++];
   if(minimum<2||minimum>8)return fail("LZW minimum code size outside GIF range");
   if(r.images==0)r.firstLzwMinimum=minimum;
   ++r.images;
   if(!subBlocks(true))return r;
   continue;
  }
  return fail("Unknown GIF block marker");
 }
 return fail("Missing GIF trailer");
}
}
