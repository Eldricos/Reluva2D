#include "engines/stafu/color_img_probe.h"
#include <cassert>
#include <cstdint>
int main(){
 using namespace reluva::stafu;
 const uint8_t data[]={0,63,64,255,0,63};
 const auto s=inspectColorImg(data,6);
 assert(s.bytes==6&&s.distinct==4&&s.zeros==2&&s.vga6Bytes==4);
 assert(s.minimum==0&&s.maximum==255&&s.frequency[63]==2);
 assert(inspectColorImg(nullptr,6).bytes==0);
 const uint8_t windows[]={0,1,2,3,4,5,6,7,8,9,64,0};
 assert(countVga6TripletWindows(windows,12,6)==1);
 assert(countVga6TripletWindows(windows,12,0)==0);
 assert(countVga6TripletWindows(windows,12,5)==0);
 assert(countVga6TripletWindows(nullptr,12,6)==0);
 return 0;
}
