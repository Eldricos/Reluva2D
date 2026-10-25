// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Read-only raster boundary diagnostics; geometry hypotheses are not format claims.
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
namespace reluva::stafu {
struct RmBoundaryRow {
 size_t offset=0,bytes=0,distinct=0,adjacentEqual=0,highBytes=0;
 uint8_t first=0,last=0;
};
inline RmBoundaryRow inspectRmBoundaryRow(const uint8_t* data,size_t size,size_t offset,size_t width){
 RmBoundaryRow out;out.offset=offset;
 if(!data||offset>=size||!width)return out;
 const size_t n=width<size-offset?width:size-offset;
 out.bytes=n;out.first=data[offset];out.last=data[offset+n-1];
 std::array<bool,256> seen{};
 for(size_t i=0;i<n;++i){
  const uint8_t v=data[offset+i];
  if(!seen[v]){seen[v]=true;++out.distinct;}
  if(v>=128)++out.highBytes;
  if(i&&v==data[offset+i-1])++out.adjacentEqual;
 }
 return out;
}
inline size_t rmAdjacentEqual(const uint8_t* data,size_t size,size_t begin,size_t length){
 if(!data||begin>=size)return 0;
 const size_t n=length<size-begin?length:size-begin;
 size_t count=0;
 for(size_t i=1;i<n;++i)if(data[begin+i]==data[begin+i-1])++count;
 return count;
}
struct RmWidthHypothesis {
 size_t width=0,fullRows=0,remainder=0,matchingRowBoundaries=0,checkedBoundaries=0;
};
inline RmWidthHypothesis inspectRmWidth(const uint8_t* data,size_t size,size_t width){
 RmWidthHypothesis out;out.width=width;
 if(!data||!width)return out;
 out.fullRows=size/width;out.remainder=size%width;
 // Compare end of each row to start of next row; equality is only a weak clue.
 for(size_t pos=width;pos<size;pos+=width){
  ++out.checkedBoundaries;
  if(data[pos-1]==data[pos])++out.matchingRowBoundaries;
 }
 return out;
}
inline size_t rmPrefixEqual(const uint8_t* data,size_t size,size_t a,size_t b,size_t length){
 if(!data||a>size||b>size)return 0;
 size_t n=length;
 if(n>size-a)n=size-a;
 if(n>size-b)n=size-b;
 size_t matches=0;
 for(size_t i=0;i<n;++i)if(data[a+i]==data[b+i])++matches;
 return matches;
}
}
