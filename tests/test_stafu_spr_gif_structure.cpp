#include "engines/stafu/spr_gif_structure.h"
#include <cassert>
#include <cstdint>
#include <vector>
int main(){
 using reluva::stafu::inspectSprGifStructure;
 std::vector<uint8_t> gif={'G','I','F','8','7','a',1,0,1,0,0,0,0,
  0x21,0xfe,1,'x',0,
  0x2c,0,0,0,0,1,0,1,0,0,2,2,0x44,0x01,0,0x3b,0,0};
 const auto a=inspectSprGifStructure(gif.data(),gif.size());
 assert(a.valid&&a.trailer&&a.images==1&&a.extensions==1);
 assert(a.imageSubBlocks==1&&a.imageDataBytes==2);
 assert(a.extensionSubBlocks==1&&a.extensionDataBytes==1);
 assert(a.firstLzwMinimum==2&&a.trailingBytes==2&&a.trailingNonzero==0);
 auto missing=gif;missing.resize(missing.size()-3);
 assert(!inspectSprGifStructure(missing.data(),missing.size()).valid);
 auto bad=gif;bad[28]=9;
 assert(!inspectSprGifStructure(bad.data(),bad.size()).valid);
 auto truncated=gif;truncated[29]=250;
 assert(!inspectSprGifStructure(truncated.data(),truncated.size()).valid);
 auto invalid=gif;invalid[13]=0x55;
 assert(!inspectSprGifStructure(invalid.data(),invalid.size()).valid);
 assert(!inspectSprGifStructure(nullptr,0).valid);
 return 0;
}
