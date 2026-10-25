// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Read-only RM trailer run analysis and per-region index distribution.
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
namespace reluva::stafu {
struct RmByteRun {size_t offset=0,length=0;uint8_t value=0;};
inline std::vector<RmByteRun> rmByteRuns(const uint8_t* data,size_t size,size_t start,size_t length){
 std::vector<RmByteRun> runs;
 if(!data||start>size)return runs;
 const size_t end=start+(length<size-start?length:size-start);
 for(size_t i=start;i<end;){
  size_t j=i+1;
  while(j<end&&data[j]==data[i])++j;
  runs.push_back({i-start,j-i,data[i]});
  i=j;
 }
 return runs;
}
struct RmIndexRegion {
 size_t bytes=0,distinct=0;
 std::array<size_t,256> counts{};
};
inline RmIndexRegion rmIndexRegion(const uint8_t* data,size_t size,size_t start,size_t length){
 RmIndexRegion s;
 if(!data||start>size)return s;
 const size_t end=start+(length<size-start?length:size-start);
 for(size_t i=start;i<end;++i){
  if(!s.counts[data[i]]++)++s.distinct;
  ++s.bytes;
 }
 return s;
}
inline size_t rmEqualBytes(const uint8_t* a,size_t aSize,const uint8_t* b,size_t bSize,
                          size_t aStart,size_t bStart,size_t length){
 if(!a||!b||aStart>aSize||bStart>bSize)return 0;
 const size_t n=(length<aSize-aStart?length:aSize-aStart)<(bSize-bStart)
  ?(length<aSize-aStart?length:aSize-aStart):(bSize-bStart);
 size_t equal=0;
 for(size_t i=0;i<n;++i)if(a[aStart+i]==b[bStart+i])++equal;
 return equal;
}
}
