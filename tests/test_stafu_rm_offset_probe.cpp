#include "engines/stafu/rm_offset_probe.h"
#include <cassert>
#include <cstdint>
int main(){
 using namespace reluva::stafu;
 const uint8_t bytes[]={1,1,2,2,1,1,2,2,3,3,4,4};
 const auto a=rmOffsetScore(bytes,12,0,4,3);
 assert(a.rows==3&&a.validPairs==8&&a.equalVertical==4&&a.equalHorizontal==4);
 const auto b=rmOffsetScore(bytes,12,4,4,3);
 assert(b.rows==2&&b.validPairs==4&&b.equalVertical==0);
 const auto seam=rmSeamScore(bytes,12,0,4,2,3);
 assert(seam.rows==3&&seam.equalAcross==0&&seam.equalOffsetOne==0);
 assert(rmOffsetScore(nullptr,12,0).rows==0);
 assert(rmSeamScore(bytes,12,0,4,4).rows==0);
 return 0;
}
