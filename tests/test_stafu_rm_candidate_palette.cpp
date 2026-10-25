#include "engines/stafu/rm_candidate_palette.h"
#include <cassert>
#include <cstdint>
#include <vector>
int main(){
 using namespace reluva::stafu;
 std::vector<uint8_t> gif={'G','I','F','8','7','a',1,0,1,0,0x80,0,0,
  10,20,30,40,50,60,
  0x2c,0,0,0,0,1,0,1,0,0,
  2,2,0x44,0x01,0,0x3b};
 const auto palette=extractRmGifPalette(gif.data(),gif.size());
 assert(palette.valid&&palette.entries==2);
 const uint8_t indices[]={1,0,1};
 std::vector<uint8_t> rgba;
 assert(mapRmIndicesToRgba(indices,3,palette,rgba));
 assert(rgba==std::vector<uint8_t>({40,50,60,255,10,20,30,255,40,50,60,255}));
 const uint8_t invalid[]={2};
 assert(!mapRmIndicesToRgba(invalid,1,palette,rgba));
 assert(rgba.size()==12);
 assert(!extractRmGifPalette(nullptr,0).valid);
 auto truncated=gif;truncated.resize(14);
 assert(!extractRmGifPalette(truncated.data(),truncated.size()).valid);
 return 0;
}
