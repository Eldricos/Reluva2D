// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Format-neutral byte statistics. These heuristics cannot identify an IMG format.
#include <array>
#include <cstddef>
#include <cstdint>
namespace reluva::stafu {
struct ColorImgStats {
 size_t bytes=0,distinct=0,zeros=0,vga6Bytes=0;
 unsigned minimum=255,maximum=0;
 std::array<size_t,256> frequency{};
};
inline ColorImgStats inspectColorImg(const uint8_t* data,size_t size){
 ColorImgStats s;
 if(!data||!size)return s;
 s.bytes=size;
 for(size_t i=0;i<size;++i){
  const unsigned v=data[i];
  if(!s.frequency[v]++)++s.distinct;
  if(v==0)++s.zeros;
  if(v<=63)++s.vga6Bytes;
  if(v<s.minimum)s.minimum=v;
  if(v>s.maximum)s.maximum=v;
 }
 return s;
}
inline size_t countVga6TripletWindows(const uint8_t* data,size_t size,size_t window=768){
 if(!data||!window||window>size||window%3)return 0;
 // Count only nonoverlapping, aligned windows. This is deliberately conservative.
 size_t count=0;
 for(size_t p=0;p+window<=size;p+=window){
  bool valid=true;
  for(size_t i=0;i<window;++i)if(data[p+i]>63){valid=false;break;}
  if(valid)++count;
 }
 return count;
}
}
