#include "engines/stafu/spr_gif_rgba.h"
#include <cassert>
#include <cstdint>
#include <vector>
int main(){
 using reluva::stafu::decodeSprGifRgba;
 std::vector<uint8_t> gif={'G','I','F','8','7','a',1,0,1,0,0x80,0,0,
  10,20,30,40,50,60,
  0x2c,0,0,0,0,1,0,1,0,0,
  2,2,0x44,0x01,0,0x3b};
 auto ok=decodeSprGifRgba(gif.data(),gif.size());
 assert(ok.valid&&ok.width==1&&ok.height==1&&ok.paletteEntries==2);
 assert(ok.pixels==std::vector<uint8_t>({10,20,30,255}));
 auto second=gif;second[31]=0x4c;
 auto secondResult=decodeSprGifRgba(second.data(),second.size());
 assert(secondResult.valid&&secondResult.pixels==std::vector<uint8_t>({40,50,60,255}));
 auto badIndex=gif;badIndex[31]=0x54;
 assert(!decodeSprGifRgba(badIndex.data(),badIndex.size()).valid);
 auto noPalette=gif;noPalette[10]=0;
 assert(!decodeSprGifRgba(noPalette.data(),noPalette.size()).valid);
 auto interlace=gif;interlace[28]=0x40;
 assert(!decodeSprGifRgba(interlace.data(),interlace.size()).valid);
 auto missingEnd=gif;missingEnd.pop_back();
 assert(!decodeSprGifRgba(missingEnd.data(),missingEnd.size()).valid);
 assert(!decodeSprGifRgba(nullptr,0).valid);
 return 0;
}
