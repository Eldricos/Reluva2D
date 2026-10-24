// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Raw RM byte layout probes. Scores describe byte repetition, not decoded pixels.
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
namespace reluva::stafu {
struct RmRowScore {
 size_t stride=0,offset=0,rows=0,adjacentEqual=0,adjacentCompared=0;
 size_t verticalEqual=0,verticalCompared=0;
};
inline RmRowScore scoreRmRows(const uint8_t* p,size_t n,size_t stride,size_t offset){
 RmRowScore r; r.stride=stride;r.offset=offset;
 if(!p||!stride||offset>=n)return r;
 r.rows=(n-offset)/stride;
 // Bounded sampling over the first 128 complete rows, no data interpretation.
 const size_t rows=std::min<size_t>(r.rows,128);
 for(size_t y=0;y<rows;++y){
  const size_t base=offset+y*stride;
  for(size_t x=1;x<stride;++x){
   ++r.adjacentCompared;r.adjacentEqual+=p[base+x]==p[base+x-1];
  }
  if(y){
   for(size_t x=0;x<stride;++x){
    ++r.verticalCompared;r.verticalEqual+=p[base+x]==p[base-stride+x];
   }
  }
 }
 return r;
}
struct RmRegionComparison {
 size_t compared=0,equal=0,bothZero=0,firstZero=0,secondZero=0;
};
inline RmRegionComparison compareRmRegions(const uint8_t* a,size_t an,
                                           const uint8_t* b,size_t bn,
                                           size_t offset,size_t length){
 RmRegionComparison r;
 if(!a||!b||offset>=an||offset>=bn)return r;
 r.compared=std::min({length,an-offset,bn-offset});
 for(size_t i=0;i<r.compared;++i){
  const auto x=a[offset+i],y=b[offset+i];
  r.equal+=x==y;r.bothZero+=x==0&&y==0;
  r.firstZero+=x==0;r.secondZero+=y==0;
 }
 return r;
}
struct RmTail {
 size_t bytes=0,zero=0,distinct=0;
};
inline RmTail surveyRmTail(const uint8_t* p,size_t n,size_t tail){
 RmTail r;
 if(!p)return r;
 r.bytes=std::min(n,tail);
 std::array<bool,256> seen{};
 for(size_t i=n-r.bytes;i<n;++i){r.zero+=p[i]==0;seen[p[i]]=true;}
 for(bool b:seen)r.distinct+=b;
 return r;
}
}
