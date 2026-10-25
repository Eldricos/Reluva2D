#include "engines/stafu/rm_standard_palette.h"
#include <cassert>
#include <cstdint>
#include <vector>
int main(){
 using namespace reluva::stafu;
 const uint8_t indices[]={0,127,128,255};
 auto stats=inspectRmIndices(indices,4,4);
 assert(stats.pixels==4&&stats.distinct==4&&stats.minimum==0&&stats.maximum==255&&stats.unsupported==2);
 assert(inspectRmIndices(indices,3,4).pixels==0);
 uint8_t palette[384]{};
 palette[0]=63;palette[1]=31;palette[2]=0;
 palette[381]=0;palette[382]=63;palette[383]=63;
 std::vector<uint8_t> rgba;
 assert(mapStandardPal(indices,4,palette,384,rgba));
 assert(rgba.size()==16&&rgba[0]==255&&rgba[1]==expandVga6(31)&&rgba[2]==0&&rgba[3]==255);
 assert(rgba[4]==0&&rgba[5]==255&&rgba[6]==255);
 assert(rgba[8]==255&&rgba[9]==0&&rgba[10]==255);
 assert(rgba[12]==255&&rgba[13]==0&&rgba[14]==255);
 assert(!mapStandardPal(indices,4,palette,383,rgba));
 palette[0]=64;
 assert(!mapStandardPal(indices,4,palette,384,rgba));
 return 0;
}
