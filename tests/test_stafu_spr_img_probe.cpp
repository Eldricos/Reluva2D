#include "engines/stafu/spr_img_probe.h"
#include <cassert>
#include <cstdint>
int main(){
 using reluva::stafu::probeSprImg;
 const uint8_t good[]={'G','I','F','8','7','a',2,0,3,0,0,0,0,0x2c};
 const auto a=probeSprImg(good,sizeof(good));
 assert(a.gifSignature&&a.logicalScreen&&a.width==2&&a.height==3);
 assert(a.globalTableFits&&a.tableBytes==0&&a.firstBlockOffset==13);
 assert(a.firstBlockValid&&a.firstBlock==0x2c);
 const uint8_t shortGif[]={'G','I','F','8','9','a'};
 const auto b=probeSprImg(shortGif,sizeof(shortGif));
 assert(b.gifSignature&&!b.logicalScreen);
 const uint8_t table[]={'G','I','F','8','9','a',1,0,1,0,0x80,0,0};
 const auto c=probeSprImg(table,sizeof(table));
 assert(c.logicalScreen&&c.tableBytes==6&&!c.globalTableFits);
 const uint8_t bad[]={'B','A','D','8','7','a'};
 assert(!probeSprImg(bad,sizeof(bad)).gifSignature);
 assert(!probeSprImg(nullptr,0).gifSignature);
 return 0;
}
