#include "engines/stafu/rm_section_align.h"
#include <cassert>
#include <cstdint>
#include <vector>
int main(){
 // 4x3 grayscale RGBA: pixel value encodes row*10+column.
 uint8_t source[4*3*4]{};
 for(int y=0;y<3;++y)for(int x=0;x<4;++x){
  const size_t p=size_t(y*4+x)*4;
  source[p]=uint8_t(y*10+x);source[p+3]=255;
 }
 std::vector<uint8_t> result;
 using reluva::stafu::alignRmRgba;
 assert(alignRmRgba(source,sizeof(source),4,3,2,1,result));
 // Shift +2: first two columns are original x=2,3; move those down 1.
 assert(result[0]==0&&result[3]==255);
 assert(result[4]==0);
 assert(result[8]==0&&result[12]==1); // right section unchanged
 assert(result[16]==2&&result[20]==3);
 assert(result[24]==10&&result[28]==11);
 assert(result[32]==12&&result[36]==13);
 assert(alignRmRgba(source,sizeof(source),4,3,2,-1,result));
 assert(result[0]==12&&result[4]==13);
 assert(result[32]==0&&result[35]==255);
 assert(alignRmRgba(source,sizeof(source),4,3,0,2,result));
 assert(result[0]==0&&result[16]==10);
 assert(!alignRmRgba(nullptr,sizeof(source),4,3,2,1,result));
 assert(!alignRmRgba(source,sizeof(source)-1,4,3,2,1,result));
 return 0;
}
