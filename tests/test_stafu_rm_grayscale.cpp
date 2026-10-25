#include "engines/stafu/rm_grayscale.h"
#include <cassert>
#include <cstdint>
int main(){
 using reluva::stafu::decodeRmGrayscale;
 const uint8_t bytes[]={99,0,127,255,1};
 const auto a=decodeRmGrayscale(bytes,sizeof(bytes),1,3,1);
 assert(a.valid&&a.rgba.size()==12);
 assert(a.rgba[0]==0&&a.rgba[3]==255);
 assert(a.rgba[4]==127&&a.rgba[5]==127&&a.rgba[7]==255);
 assert(a.rgba[8]==255&&a.rgba[11]==255);
 assert(!decodeRmGrayscale(bytes,sizeof(bytes),2,4,1).valid);
 assert(!decodeRmGrayscale(bytes,sizeof(bytes),6,1,1).valid);
 assert(!decodeRmGrayscale(nullptr,0).valid);
 assert(!decodeRmGrayscale(bytes,sizeof(bytes),0,0,1).valid);
 return 0;
}
