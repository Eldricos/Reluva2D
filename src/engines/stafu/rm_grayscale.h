// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
// Diagnostic mapping only: byte value -> grayscale RGBA, not an original palette.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace reluva::stafu {
struct RmGrayscale {
 bool valid=false;
 unsigned width=640,height=480;
 std::vector<uint8_t> rgba;
 std::string error;
};
inline RmGrayscale decodeRmGrayscale(const uint8_t* bytes,size_t length,
                                    size_t offset=0,unsigned width=640,unsigned height=480) {
 RmGrayscale out;out.width=width;out.height=height;
 if(!bytes||!width||!height||offset>length||
    size_t(width)>(length-offset)/size_t(height)) {
  out.error="RM resource is too short for the requested raster";return out;
 }
 const size_t count=size_t(width)*height;
 out.rgba.resize(count*4);
 for(size_t i=0;i<count;++i) {
  const uint8_t value=bytes[offset+i];
  out.rgba[4*i]=value;out.rgba[4*i+1]=value;
  out.rgba[4*i+2]=value;out.rgba[4*i+3]=255;
 }
 out.valid=true;return out;
}
}
