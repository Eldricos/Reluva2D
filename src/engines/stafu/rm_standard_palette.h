// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Experimental compatibility checks; no assertion that standard.pal belongs to RM.
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <utility>
namespace reluva::stafu {
struct RmIndexStats {
 size_t pixels=0,unsupported=0;
 unsigned minimum=255,maximum=0,distinct=0;
 std::array<size_t,256> counts{};
};
inline RmIndexStats inspectRmIndices(const uint8_t* bytes,size_t length,size_t count=640u*480u){
 RmIndexStats result;
 if(!bytes||length<count)return result;
 result.pixels=count;
 for(size_t i=0;i<count;++i){
  const auto v=bytes[i];
  if(!result.counts[v]++)++result.distinct;
  if(v<result.minimum)result.minimum=v;
  if(v>result.maximum)result.maximum=v;
  if(v>=128)++result.unsupported;
 }
 return result;
}
inline uint8_t expandVga6(uint8_t value){return uint8_t((unsigned(value)*255u+31u)/63u);}
inline bool mapStandardPal(const uint8_t* indices,size_t length,
                           const uint8_t* palette,size_t paletteBytes,
                           std::vector<uint8_t>& rgba){
 if(!indices||!palette||paletteBytes!=384||length>SIZE_MAX/4)return false;
 std::vector<uint8_t> result(length*4);
 for(size_t i=0;i<length;++i){
  const size_t p=i*4;
  if(indices[i]>=128){
   // Bright magenta explicitly marks unsupported palette indices.
   result[p]=255;result[p+1]=0;result[p+2]=255;
  }else{
   const size_t c=size_t(indices[i])*3;
   if(palette[c]>63||palette[c+1]>63||palette[c+2]>63)return false;
   result[p]=expandVga6(palette[c]);
   result[p+1]=expandVga6(palette[c+1]);
   result[p+2]=expandVga6(palette[c+2]);
  }
  result[p+3]=255;
 }
 rgba=std::move(result);
 return true;
}
}
