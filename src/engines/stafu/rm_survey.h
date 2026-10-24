// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Read-only byte-level RM observations. Numeric interpretations are candidates, not format claims.
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
namespace reluva::stafu {
struct RmSurvey {
 size_t bytes=0,zeroBytes=0,distinctBytes=0;
 std::array<uint8_t,32> prefix{};
 size_t prefixSize=0;
 size_t aligned32InBounds=0,aligned32Monotonic=0;
 size_t aligned32Count=0;
 std::vector<size_t> gifOffsets;
 std::vector<size_t> trpsOffsets;
 std::vector<size_t> tsilOffsets;
};
inline RmSurvey surveyRm(const uint8_t* data,size_t size){
 RmSurvey r;
 if(!data||!size)return r;
 r.bytes=size;
 std::array<bool,256> seen{};
 for(size_t i=0;i<size;++i){seen[data[i]]=true;r.zeroBytes+=data[i]==0;}
 for(bool b:seen)r.distinctBytes+=b;
 r.prefixSize=std::min(size,r.prefix.size());
 std::copy_n(data,r.prefixSize,r.prefix.begin());
 const auto signature=[&](size_t i,const char* s,size_t n){
  if(n>size-i)return false;
  for(size_t j=0;j<n;++j)if(data[i+j]!=static_cast<uint8_t>(s[j]))return false;
  return true;
 };
 for(size_t i=0;i<size;++i){
  if(signature(i,"GIF87a",6)||signature(i,"GIF89a",6))r.gifOffsets.push_back(i);
  if(signature(i,"TRPS",4))r.trpsOffsets.push_back(i);
  if(signature(i,"TSIL",4))r.tsilOffsets.push_back(i);
 }
 uint32_t previous=0;bool havePrevious=false;
 for(size_t i=0;i+4<=size&&r.aligned32Count<64;i+=4){
  const uint32_t v=uint32_t(data[i])|(uint32_t(data[i+1])<<8)|
    (uint32_t(data[i+2])<<16)|(uint32_t(data[i+3])<<24);
  ++r.aligned32Count;
  r.aligned32InBounds+=v<size;
  if(havePrevious)r.aligned32Monotonic+=v>=previous;
  previous=v;havePrevious=true;
 }
 return r;
}
}
