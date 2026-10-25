// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstddef>
#include <cstdint>
namespace reluva::stafu {
// Tests original storage seam at x=384 and candidate alignment at shifted x=0.
// An equal index is a weak signal; count exact equality without inferring palette.
struct RmSeamEvidence {size_t rows=0,rawEqual=0,alignedEqual=0,alignedPreviousRowEqual=0,alignedNextRowEqual=0;};
inline RmSeamEvidence rmSeamEvidence(const uint8_t* p,size_t size,size_t width=640,
                                    size_t height=480,size_t split=384){
 RmSeamEvidence r;
 if(!p||width<2||split==0||split>=width||height==0||size/width<height)return r;
 r.rows=height;
 // After right rotation by width-split, raw split is the output row's x=0.
 // At that edge the two neighboring output pixels are raw split-1 and split.
 for(size_t y=0;y<height;++y){
  const size_t base=y*width;
  const uint8_t left=p[base+split-1],right=p[base+split];
  if(left==right)++r.rawEqual;
  // The corrected left section is moved down by one row. At its right edge
  // compare raw last-column of prior row with current raw first-column.
  if(y>0){
   if(p[(y-1)*width+split-1]==right)++r.alignedPreviousRowEqual;
  }
  if(y+1<height){
   if(p[(y+1)*width+split-1]==right)++r.alignedNextRowEqual;
  }
  // Row-local comparison, equivalent to raw seam before vertical correction.
  if(left==right)++r.alignedEqual;
 }
 return r;
}
struct RmEdgeEvidence {size_t pairs=0,sameRow=0,priorRow=0,nextRow=0;};
inline RmEdgeEvidence rmEdgeEvidence(const uint8_t* p,size_t size,size_t width,size_t height,size_t split){
 RmEdgeEvidence r;
 if(!p||width<2||split==0||split>=width||size/width<height)return r;
 for(size_t y=1;y+1<height;++y){
  const uint8_t right=p[y*width+split];
  ++r.pairs;
  if(p[y*width+split-1]==right)++r.sameRow;
  if(p[(y-1)*width+split-1]==right)++r.priorRow;
  if(p[(y+1)*width+split-1]==right)++r.nextRow;
 }
 return r;
}
}
