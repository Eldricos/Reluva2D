#include "engines/stafu/rm_encoding_probe.h"
#include <cassert>
#include <cstdint>
int main(){
 using namespace reluva::stafu;
 const uint8_t data[]={1,2,3,1,2,3,128,128,128};
 const auto all=inspectRmRegion(data,9,0,9);
 assert(all.bytes==9&&all.distinct==4&&all.highIndices==3);
 assert(all.adjacentEqual==2&&all.triplets==3&&all.vga6Triplets==2);
 const auto slice=inspectRmRegion(data,9,6,20);
 assert(slice.bytes==3&&slice.highIndices==3);
 assert(inspectRmRegion(data,9,10,3).bytes==0);
 assert(inspectRmRegion(nullptr,9,0,9).bytes==0);
 const auto rgb=inspectRmRgbHypothesis(data,9,0);
 assert(rgb.triplets==3&&rgb.vga6Triplets==2&&rgb.adjacentRgbEqual==1);
 assert(inspectRmRgbHypothesis(data,9,10).triplets==0);
 return 0;
}
