#include "engines/stafu/rm_horizontal_shift.h"
#include <cassert>
#include <cstdint>
#include <vector>
int main(){
 const uint8_t pixels[]={
  1,1,1,255,2,2,2,255,3,3,3,255,
  4,4,4,255,5,5,5,255,6,6,6,255
 };
 std::vector<uint8_t> out;
 using reluva::stafu::shiftRmRgbaRows;
 assert(shiftRmRgbaRows(pixels,sizeof(pixels),3,2,1,out));
 assert(out[0]==3&&out[4]==1&&out[8]==2);
 assert(out[12]==6&&out[16]==4&&out[20]==5);
 assert(shiftRmRgbaRows(pixels,sizeof(pixels),3,2,-1,out));
 assert(out[0]==2&&out[4]==3&&out[8]==1);
 assert(shiftRmRgbaRows(pixels,sizeof(pixels),3,2,3,out));
 assert(out[0]==1&&out[12]==4);
 assert(!shiftRmRgbaRows(nullptr,sizeof(pixels),3,2,0,out));
 assert(!shiftRmRgbaRows(pixels,sizeof(pixels)-1,3,2,0,out));
 assert(!shiftRmRgbaRows(pixels,sizeof(pixels),0,2,0,out));
 return 0;
}
