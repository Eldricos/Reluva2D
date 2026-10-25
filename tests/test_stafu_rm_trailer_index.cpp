#include "engines/stafu/rm_trailer_index.h"
#include <cassert>
#include <cstdint>
int main(){
 using namespace reluva::stafu;
 const uint8_t a[]={1,1,2,2,2,3,3,1};
 const uint8_t b[]={1,2,2,2,2,3,4,1};
 const auto runs=rmByteRuns(a,8,1,6);
 assert(runs.size()==3);
 assert(runs[0].offset==0&&runs[0].length==1&&runs[0].value==1);
 assert(runs[1].offset==1&&runs[1].length==3&&runs[1].value==2);
 assert(runs[2].offset==4&&runs[2].length==2&&runs[2].value==3);
 const auto region=rmIndexRegion(a,8,2,4);
 assert(region.bytes==4&&region.distinct==2&&region.counts[2]==3&&region.counts[3]==1);
 assert(rmEqualBytes(a,8,b,8,0,0,8)==5);
 assert(rmByteRuns(a,8,9,3).empty());
 assert(rmIndexRegion(nullptr,8,0,8).bytes==0);
 assert(rmEqualBytes(a,8,b,8,9,0,4)==0);
 return 0;
}
