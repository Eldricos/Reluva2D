// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstddef>
#include <cstdint>
namespace reluva::stafu {
struct RmOffsetScore {
 size_t offset=0,rows=0,validPairs=0,equalVertical=0,equalHorizontal=0;
};
inline RmOffsetScore rmOffsetScore(const uint8_t* bytes,size_t size,size_t offset,
                                   size_t width=640,size_t rows=480){
 RmOffsetScore out;out.offset=offset;
 if(!bytes||!width||!rows||offset>=size)return out;
 const size_t available=(size-offset)/width;
 out.rows=available<rows?available:rows;
 for(size_t y=1;y<out.rows;++y){
  for(size_t x=0;x<width;++x){
   const size_t p=offset+y*width+x;
   ++out.validPairs;
   if(bytes[p]==bytes[p-width])++out.equalVertical;
   if(x&&bytes[p]==bytes[p-1])++out.equalHorizontal;
  }
 }
 return out;
}
struct RmSeamScore {
 size_t seam=0,rows=0,equalAcross=0,equalOffsetOne=0;
};
inline RmSeamScore rmSeamScore(const uint8_t* bytes,size_t size,size_t offset,
                               size_t width,size_t seam,size_t rows=480){
 RmSeamScore out;out.seam=seam;
 if(!bytes||!width||seam==0||seam>=width||offset>=size)return out;
 const size_t available=(size-offset)/width;
 out.rows=available<rows?available:rows;
 for(size_t y=0;y<out.rows;++y){
  const size_t p=offset+y*width+seam;
  if(bytes[p-1]==bytes[p])++out.equalAcross;
  if(y+1<out.rows&&bytes[p-1]==bytes[p+width])++out.equalOffsetOne;
 }
 return out;
}
}
