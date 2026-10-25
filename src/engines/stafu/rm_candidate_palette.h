// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Experimental palette extraction and indexed-color mapping; no claim of RM palette provenance.
#include "engines/stafu/spr_gif_structure.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace reluva::stafu {
struct RmCandidatePalette {
 bool valid=false;
 size_t entries=0;
 std::array<uint8_t,768> rgb{};
 std::string error;
};
inline RmCandidatePalette extractRmGifPalette(const uint8_t* bytes,size_t length){
 RmCandidatePalette result;
 const auto structure=inspectSprGifStructure(bytes,length);
 if(!structure.valid){result.error=structure.error;return result;}
 const auto header=probeSprImg(bytes,length);
 if(header.tableBytes==0||header.tableBytes>768||header.tableBytes%3!=0||
    length<13||header.tableBytes>length-13){
  result.error="GIF has no supported global color table";return result;
 }
 result.entries=header.tableBytes/3;
 for(size_t i=0;i<header.tableBytes;++i)result.rgb[i]=bytes[13+i];
 result.valid=true;return result;
}
inline bool mapRmIndicesToRgba(const uint8_t* indices,size_t length,
                              const RmCandidatePalette& palette,
                              std::vector<uint8_t>& rgba){
 if(!indices||!palette.valid||!palette.entries||length>SIZE_MAX/4)return false;
 std::vector<uint8_t> result(length*4);
 for(size_t i=0;i<length;++i){
  const size_t index=indices[i];
  if(index>=palette.entries)return false;
  const size_t p=index*3;
  result[4*i]=palette.rgb[p];result[4*i+1]=palette.rgb[p+1];
  result[4*i+2]=palette.rgb[p+2];result[4*i+3]=255;
 }
 rgba=std::move(result);return true;
}
}
