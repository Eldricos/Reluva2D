// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Diagnostic horizontal wrap only. Does not change or interpret game assets.
#include <cstddef>
#include <cstdint>
#include <vector>
namespace reluva::stafu {
inline bool shiftRmRgbaRows(const uint8_t* source,size_t length,
                            unsigned width,unsigned height,int shift,
                            std::vector<uint8_t>& destination) {
 if(!source||!width||!height||size_t(width)>SIZE_MAX/4||
    size_t(height)>SIZE_MAX/(size_t(width)*4))return false;
 const size_t needed=size_t(width)*height*4;
 if(length<needed)return false;
 const int normalized=int((int64_t(shift)%int64_t(width)+int64_t(width))%int64_t(width));
 destination.resize(needed);
 for(size_t y=0;y<height;++y)
  for(size_t x=0;x<width;++x){
   const size_t sourceX=(x+width-size_t(normalized))%width;
   const size_t src=(y*width+sourceX)*4;
   const size_t dst=(y*width+x)*4;
   for(size_t channel=0;channel<4;++channel)destination[dst+channel]=source[src+channel];
  }
 return true;
}
}
