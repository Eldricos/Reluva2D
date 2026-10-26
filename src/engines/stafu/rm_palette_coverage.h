// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
namespace reluva::stafu {
struct RmPaletteCoverage {
 size_t pixels=0,lowerPixels=0,upperPixels=0,lowerDistinct=0,upperDistinct=0;
 std::array<size_t,256> counts{};
};
inline RmPaletteCoverage rmPaletteCoverage(const uint8_t* bytes,size_t length,size_t raster=640u*480u){
 RmPaletteCoverage r;
 if(!bytes||length<raster)return r;
 r.pixels=raster;
 for(size_t i=0;i<raster;++i){
  const unsigned index=bytes[i];
  if(r.counts[index]++==0){if(index<128)++r.lowerDistinct;else ++r.upperDistinct;}
  if(index<128)++r.lowerPixels;else ++r.upperPixels;
 }
 return r;
}
struct RmPaletteOverlap {
 size_t equal=0,different=0;
};
inline RmPaletteOverlap rmPaletteOverlap(const uint8_t* vga6,size_t length,
                                         const uint8_t* gif,size_t gifLength){
 RmPaletteOverlap r;
 if(!vga6||length!=384||!gif||gifLength<384)return r;
 for(size_t i=0;i<128;++i){
  bool same=true;
  for(size_t c=0;c<3;++c){
   const unsigned expanded=(unsigned(vga6[3*i+c])*255u+31u)/63u;
   if(vga6[3*i+c]>63||expanded!=gif[3*i+c])same=false;
  }
  if(same)++r.equal;else ++r.different;
 }
 return r;
}
}
