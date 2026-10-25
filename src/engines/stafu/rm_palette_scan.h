// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Read-only, heuristic scan for raw 256xRGB palettes. Hits are NOT proof of a palette.
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
namespace reluva::stafu {
struct PaletteWindow {size_t offset=0;unsigned distinct=0;uint8_t maximum=0;};
inline std::vector<PaletteWindow> scanVgaPaletteWindows(const uint8_t* data,size_t size,
                                                        size_t limit=12){
 std::vector<PaletteWindow> found;
 if(!data||size<768)return found;
 // Inspect aligned candidates, including offsets inside small headers.
 for(size_t offset=0;offset<=size-768;offset+=3){
  std::array<bool,64> seen{};
  unsigned distinct=0;uint8_t maximum=0;
  bool inRange=true;
  for(size_t j=0;j<768;++j){
   const uint8_t value=data[offset+j];
   if(value>63){inRange=false;break;}
   if(!seen[value]){seen[value]=true;++distinct;}
   if(value>maximum)maximum=value;
  }
  if(inRange&&distinct>=24&&maximum>=32){
   found.push_back({offset,distinct,maximum});
   if(found.size()>=limit)break;
   // Suppress nearly identical sliding windows over the same 768-byte region.
   offset+=765;
  }
 }
 return found;
}
inline bool equalPaletteRgb(const uint8_t* a,const uint8_t* b,size_t bytes){
 if(!a||!b)return false;
 for(size_t i=0;i<bytes;++i)if(a[i]!=b[i])return false;
 return true;
}
}
