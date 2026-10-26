// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Compare standard.pal against every contiguous 128-entry window of a 256-color RGB table.
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
namespace reluva::stafu {
struct RmPaletteWindow {size_t offset=0,exact=0;};
inline std::vector<RmPaletteWindow> rmPaletteWindows(const uint8_t* vga,size_t vgaLength,
                                                     const uint8_t* rgb,size_t rgbLength){
 std::vector<RmPaletteWindow> results;
 if(!vga||vgaLength!=384||!rgb||rgbLength<384||rgbLength%3)return results;
 for(size_t i=0;i<384;++i)if(vga[i]>63)return results;
 const size_t entries=rgbLength/3;
 for(size_t offset=0;offset+128<=entries;++offset){
  size_t exact=0;
  for(size_t i=0;i<128;++i){
   bool same=true;
   for(size_t c=0;c<3;++c){
    const unsigned expanded=(unsigned(vga[3*i+c])*255u+31u)/63u;
    if(expanded!=rgb[3*(offset+i)+c])same=false;
   }
   if(same)++exact;
  }
  results.push_back({offset,exact});
 }
 return results;
}
}
