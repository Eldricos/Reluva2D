// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Region statistics for testing competing RM byte interpretations, not a decoder.
#include <array>
#include <cstddef>
#include <cstdint>
namespace reluva::stafu {
struct RmRegionStats {
 size_t bytes=0,distinct=0,highIndices=0,adjacentEqual=0,triplets=0,vga6Triplets=0;
 std::array<size_t,256> counts{};
};
inline RmRegionStats inspectRmRegion(const uint8_t* data,size_t size,size_t begin,size_t length){
 RmRegionStats s;
 if(!data||begin>size)return s;
 const size_t n=length<size-begin?length:size-begin;
 s.bytes=n;
 for(size_t i=0;i<n;++i){
  const unsigned v=data[begin+i];
  if(!s.counts[v]++)++s.distinct;
  if(v>=128)++s.highIndices;
  if(i&&v==data[begin+i-1])++s.adjacentEqual;
 }
 for(size_t i=0;i+2<n;i+=3){
  ++s.triplets;
  if(data[begin+i]<=63&&data[begin+i+1]<=63&&data[begin+i+2]<=63)
   ++s.vga6Triplets;
 }
 return s;
}
struct RmRgbHypothesis {
 size_t triplets=0,vga6Triplets=0,adjacentRgbEqual=0;
};
inline RmRgbHypothesis inspectRmRgbHypothesis(const uint8_t* data,size_t size,size_t offset){
 RmRgbHypothesis out;
 if(!data||offset>size)return out;
 for(size_t i=offset;i+2<size;i+=3){
  ++out.triplets;
  if(data[i]<=63&&data[i+1]<=63&&data[i+2]<=63)++out.vga6Triplets;
  if(i>=offset+3&&data[i]==data[i-3]&&data[i+1]==data[i-2]&&data[i+2]==data[i-1])
   ++out.adjacentRgbEqual;
 }
 return out;
}
}
