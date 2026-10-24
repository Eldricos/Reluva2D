// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Bounded single-image, non-interlaced GIF palette-to-RGBA decoder.
#include "engines/stafu/spr_gif_structure.h"
#include "engines/stafu/spr_gif_lzw.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace reluva::stafu {
struct SprGifRgba {
 bool valid=false;
 uint16_t width=0,height=0;
 size_t paletteEntries=0;
 bool localPalette=false;
 std::vector<uint8_t> pixels; // RGBA, row-major
 std::string error;
};
inline SprGifRgba decodeSprGifRgba(const uint8_t* bytes,size_t size){
 SprGifRgba r;
 const auto structure=inspectSprGifStructure(bytes,size);
 if(!structure.valid){r.error=structure.error;return r;}
 if(structure.images!=1){r.error="Only single-image GIFs supported";return r;}
 const auto header=probeSprImg(bytes,size);
 size_t p=header.firstBlockOffset;
 const uint8_t* global=bytes+13;
 const size_t globalCount=header.tableBytes/3;
 auto fail=[&](const char* msg){r.error=msg;return r;};
 while(p<structure.trailerOffset){
  const uint8_t tag=bytes[p++];
  if(tag==0x21){
   const uint8_t label=bytes[p++];
   // Reject transparency until compositing semantics are implemented.
   if(label==0xf9)return fail("Graphic control extension unsupported");
   while(true){const size_t len=bytes[p++];if(!len)break;p+=len;}
   continue;
  }
  if(tag!=0x2c)return fail("Unexpected block");
  const uint16_t left=uint16_t(uint16_t(bytes[p])|(uint16_t(bytes[p+1])<<8));
  const uint16_t top=uint16_t(uint16_t(bytes[p+2])|(uint16_t(bytes[p+3])<<8));
  const uint16_t w=uint16_t(uint16_t(bytes[p+4])|(uint16_t(bytes[p+5])<<8));
  const uint16_t h=uint16_t(uint16_t(bytes[p+6])|(uint16_t(bytes[p+7])<<8));
  const uint8_t flags=bytes[p+8];p+=9;
  if(flags&0x40u)return fail("Interlaced GIF unsupported");
  if(left!=0||top!=0||w!=header.width||h!=header.height)
   return fail("Sub-rectangle image unsupported");
  const uint8_t* palette=global;
  size_t paletteCount=globalCount;
  if(flags&0x80u){
   paletteCount=size_t(1)<<(1+(flags&7u));
   palette=bytes+p;p+=paletteCount*3;r.localPalette=true;
  }
  if(paletteCount==0)return fail("Missing color palette");
  const unsigned minimum=bytes[p++];
  std::vector<uint8_t> compressed;
  while(true){
   const size_t count=bytes[p++];if(!count)break;
   compressed.insert(compressed.end(),bytes+p,bytes+p+count);p+=count;
  }
  const auto decoded=decodeSprGifLzw(compressed.data(),compressed.size(),minimum,size_t(w)*h);
  if(!decoded.valid){r.error=decoded.error;return r;}
  r.pixels.reserve(decoded.indices.size()*4);
  for(uint8_t index:decoded.indices){
   if(size_t(index)>=paletteCount)return fail("Palette index out of range");
   const size_t offset=size_t(index)*3;
   r.pixels.push_back(palette[offset]);
   r.pixels.push_back(palette[offset+1]);
   r.pixels.push_back(palette[offset+2]);
   r.pixels.push_back(255);
  }
  r.width=w;r.height=h;r.paletteEntries=paletteCount;r.valid=true;
  return r;
 }
 return fail("Missing image descriptor");
}
}
