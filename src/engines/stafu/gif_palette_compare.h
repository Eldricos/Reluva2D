// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "engines/stafu/rm_candidate_palette.h"
#include <cstddef>
#include <cstdint>
namespace reluva::stafu {
struct GifPaletteDifference {
 size_t comparedEntries=0,identicalEntries=0,differentChannels=0;
 uint64_t absoluteChannelDifference=0;
 unsigned maximumChannelDifference=0;
};
inline GifPaletteDifference compareGifPalettes(const RmCandidatePalette& a,const RmCandidatePalette& b){
 GifPaletteDifference out;
 if(!a.valid||!b.valid)return out;
 out.comparedEntries=a.entries<b.entries?a.entries:b.entries;
 for(size_t i=0;i<out.comparedEntries;++i){
  bool same=true;
  for(size_t c=0;c<3;++c){
   const unsigned x=a.rgb[i*3+c],y=b.rgb[i*3+c];
   const unsigned delta=x>y?x-y:y-x;
   if(delta){same=false;++out.differentChannels;out.absoluteChannelDifference+=delta;
    if(delta>out.maximumChannelDifference)out.maximumChannelDifference=delta;}
  }
  if(same)++out.identicalEntries;
 }
 return out;
}
}
