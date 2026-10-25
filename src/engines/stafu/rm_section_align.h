// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Experimental per-section vertical translation after horizontal row rotation.
// Left section = first 'shift' output columns when shift is positive.
// Out-of-range pixels are black; no row wraps vertically.
#include "engines/stafu/rm_horizontal_shift.h"
#include <cstddef>
#include <cstdint>
#include <vector>
namespace reluva::stafu {
inline bool alignRmRgba(const uint8_t* source,size_t length,
                        unsigned width,unsigned height,int horizontalShift,
                        int leftVerticalShift,std::vector<uint8_t>& destination) {
 std::vector<uint8_t> rotated;
 if(!shiftRmRgbaRows(source,length,width,height,horizontalShift,rotated))return false;
 const int normalized=int((int64_t(horizontalShift)%int64_t(width)+int64_t(width))%int64_t(width));
 destination=rotated;
 if(normalized==0||leftVerticalShift==0)return true;
 for(size_t y=0;y<height;++y){
  const int64_t sourceY=int64_t(y)-int64_t(leftVerticalShift);
  for(int x=0;x<normalized;++x){
   const size_t dst=(y*width+size_t(x))*4;
   if(sourceY<0||sourceY>=int64_t(height)){
    destination[dst]=0;destination[dst+1]=0;destination[dst+2]=0;
    destination[dst+3]=255;
   }else{
    const size_t src=(size_t(sourceY)*width+size_t(x))*4;
    for(size_t c=0;c<4;++c)destination[dst+c]=rotated[src+c];
   }
  }
 }
 return true;
}
}
