#include "engines/stafu/standard_pal_probe.h"
#include <cassert>
#include <cstdint>
int main(){
 using namespace reluva::stafu;
 const uint8_t data[]={0,0,0, 63,63,63, 63,63,63, 255,0,1};
 auto s=inspectPalTriplets(data,sizeof(data));
 assert(s.entries==4&&s.distinctColors==3);
 assert(s.distinctChannelValues==4&&s.minChannel==0&&s.maxChannel==255);
 assert(!s.allVga6&&s.grayscaleEntries==3&&s.adjacentRepeated==1&&s.zeroTriplets==1);
 auto shifted=inspectPalTriplets(data,sizeof(data),3);
 assert(shifted.entries==3&&shifted.distinctColors==2);
 assert(inspectPalTriplets(nullptr,sizeof(data)).entries==0);
 assert(inspectPalTriplets(data,sizeof(data),sizeof(data)+1).entries==0);
 return 0;
}
