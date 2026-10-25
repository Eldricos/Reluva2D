// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Experimental split-section assembly of a 640x480 indexed/RGBA raster.
// Storage columns [384,640) become display left [0,256);
// storage columns [0,384) become display right [256,640).
// Sections have independent signed vertical translations. No wrap at edges.
#include <cstddef>
#include <cstdint>
#include <vector>
namespace reluva::stafu {
inline bool assembleRmSectionsRgba(const uint8_t* source,size_t length,
                                  unsigned width,unsigned height,unsigned split,
                                  int leftDown,int rightDown,
                                  std::vector<uint8_t>& out) {
 if(!source||!width||!height||!split||split>=width||
    size_t(width)>SIZE_MAX/4||size_t(height)>SIZE_MAX/(size_t(width)*4))return false;
 const size_t needed=size_t(width)*height*4;
 if(length<needed)return false;
 out.assign(needed,0);
 for(size_t y=0;y<height;++y)for(size_t x=0;x<width;++x){
  const bool left=x<width-split;
  const int delta=left?leftDown:rightDown;
  const int64_t sy=int64_t(y)-int64_t(delta);
  const size_t dst=(y*width+x)*4;
  out[dst+3]=255;
  if(sy<0||sy>=int64_t(height))continue;
  const size_t sx=left?size_t(split)+x:x-(width-split);
  const size_t src=(size_t(sy)*width+sx)*4;
  for(size_t c=0;c<4;++c)out[dst+c]=source[src+c];
 }
 return true;
}
}
