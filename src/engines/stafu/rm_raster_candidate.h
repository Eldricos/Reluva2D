// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Milestone 59: compare two candidate placements of a 640x480 byte raster.
// Statistics are observations, not evidence of a palette or decoded image.
#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
namespace reluva::stafu {
struct RmRasterCandidate {
 bool fits=false;
 size_t offset=0,width=640,height=480,distinct=0;
 size_t horizontalMatches=0,horizontalPairs=0;
 size_t verticalMatches=0,verticalPairs=0;
 size_t boundaryMatches=0,boundaryPairs=0;
 size_t interiorMatches=0,interiorPairs=0;
 size_t outsideBytes=0,outsideZero=0,outsideDistinct=0;
};
inline RmRasterCandidate inspectRmRaster(const uint8_t* data,size_t size,size_t offset,
                                         size_t width=640,size_t height=480) {
 RmRasterCandidate r;r.offset=offset;r.width=width;r.height=height;
 if(!data||!width||!height||offset>size||width>size-offset||height>(size-offset)/width)return r;
 r.fits=true;
 const size_t end=offset+width*height;
 std::array<bool,256> used{},outside{};
 for(size_t i=offset;i<end;++i)used[data[i]]=true;
 for(bool v:used)r.distinct+=v;
 for(size_t i=0;i<size;++i)if(i<offset||i>=end){
  ++r.outsideBytes;r.outsideZero+=data[i]==0;outside[data[i]]=true;
 }
 for(bool v:outside)r.outsideDistinct+=v;
 for(size_t y=0;y<height;++y){
  const size_t base=offset+y*width;
  for(size_t x=1;x<width;++x){
   ++r.horizontalPairs;r.horizontalMatches+=data[base+x]==data[base+x-1];
  }
  if(y){
   for(size_t x=0;x<width;++x){
    ++r.verticalPairs;r.verticalMatches+=data[base+x]==data[base-width+x];
   }
   // Compare the artificial row break against ordinary within-row adjacency.
   ++r.boundaryPairs;r.boundaryMatches+=data[base]==data[base-1];
  }
  if(width>1){
   ++r.interiorPairs;r.interiorMatches+=data[base+1]==data[base];
  }
 }
 return r;
}
}
