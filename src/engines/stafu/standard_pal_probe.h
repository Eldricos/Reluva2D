// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Diagnostics only: statistical properties do not establish a palette format.
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
namespace reluva::stafu {
struct PalTripletStats {
 size_t entries=0;
 unsigned distinctColors=0;
 unsigned distinctChannelValues=0;
 unsigned minChannel=255,maxChannel=0;
 bool allVga6=true;
 unsigned grayscaleEntries=0;
 unsigned adjacentRepeated=0;
 unsigned zeroTriplets=0;
};
inline PalTripletStats inspectPalTriplets(const uint8_t* data,size_t length,size_t offset=0){
 PalTripletStats s;
 if(!data||offset>length)return s;
 s.entries=(length-offset)/3;
 std::array<bool,256> seenChannel{};
 for(size_t i=0;i<s.entries;++i){
  const size_t p=offset+i*3;
  const unsigned r=data[p],g=data[p+1],b=data[p+2];
  for(unsigned v:{r,g,b}){
   if(!seenChannel[v]){seenChannel[v]=true;++s.distinctChannelValues;}
   if(v<s.minChannel)s.minChannel=v;
   if(v>s.maxChannel)s.maxChannel=v;
   if(v>63)s.allVga6=false;
  }
  if(r==g&&g==b)++s.grayscaleEntries;
  if(r==0&&g==0&&b==0)++s.zeroTriplets;
  if(i&&data[p]==data[p-3]&&data[p+1]==data[p-2]&&data[p+2]==data[p-1])++s.adjacentRepeated;
  bool first=true;
  for(size_t j=0;j<i;++j){
   const size_t q=offset+j*3;
   if(data[q]==r&&data[q+1]==g&&data[q+2]==b){first=false;break;}
  }
  if(first)++s.distinctColors;
 }
 if(!s.entries)s.minChannel=0;
 return s;
}
}
