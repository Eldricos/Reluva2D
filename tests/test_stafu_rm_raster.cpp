#include "engines/stafu/rm_raster_candidate.h"
#include <cassert>
#include <cstdint>
int main(){
 using reluva::stafu::inspectRmRaster;
 const uint8_t b[]={9,1,1,2,1,1,2,8};
 const auto a=inspectRmRaster(b,8,1,3,2);
 assert(a.fits&&a.offset==1&&a.distinct==2);
 assert(a.horizontalPairs==4&&a.horizontalMatches==2);
 assert(a.verticalPairs==3&&a.verticalMatches==3);
 assert(a.boundaryPairs==1&&a.boundaryMatches==0);
 assert(a.outsideBytes==2&&a.outsideDistinct==2);
 assert(!inspectRmRaster(b,8,9,3,2).fits);
 assert(!inspectRmRaster(b,8,0,0,2).fits);
 assert(!inspectRmRaster(nullptr,8,0,3,2).fits);
 return 0;
}
