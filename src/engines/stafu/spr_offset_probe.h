// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "engines/stafu/spr_sequence_index.h"
#include <cstddef>
#include <cstdint>
namespace reluva::stafu {
struct SprOffsetProbe { bool absoluteBoundary=false, recordBoundary=false, tsilBoundary=false; bool absoluteInside=false,recordInside=false,tsilInside=false; };
inline SprOffsetProbe probeSprOffset(uint32_t value, size_t fileSize, const SprSequenceIndex::Record& record) {
 SprOffsetProbe result;
 const size_t positions[3]={size_t(value),record.start+size_t(value),size_t(12)+size_t(value)};
 for(size_t mode=0;mode<3;++mode){
  if(mode==0 && positions[mode]>=fileSize)continue;
  if(mode==1 && (value>fileSize || record.start>fileSize-size_t(value)))continue;
  if(mode==2 && (value>fileSize || size_t(12)>fileSize-size_t(value)))continue;
  const size_t p=positions[mode];
  for(const auto& e:record.events){
   const bool boundary=p==e.chunkOffset || p==e.payloadOffset || (e.hasImageHeader && p==e.imageDataOffset);
   const bool inside=p>=e.chunkOffset && p<e.chunkOffset+e.chunkSize;
   if(mode==0){result.absoluteBoundary|=boundary;result.absoluteInside|=inside;}
   if(mode==1){result.recordBoundary|=boundary;result.recordInside|=inside;}
   if(mode==2){result.tsilBoundary|=boundary;result.tsilInside|=inside;}
  }
 }
 return result;
}
}
