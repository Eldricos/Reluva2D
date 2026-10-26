#include "engines/stafu/rm_palette_coverage.h"
#include <cassert>
#include <cstdint>
int main(){
 using namespace reluva::stafu;
 const uint8_t indices[]={0,0,127,128,128,255};
 const auto r=rmPaletteCoverage(indices,6,6);
 assert(r.pixels==6&&r.lowerPixels==3&&r.upperPixels==3);
 assert(r.lowerDistinct==2&&r.upperDistinct==2);
 assert(r.counts[0]==2&&r.counts[128]==2&&r.counts[255]==1);
 assert(rmPaletteCoverage(indices,5,6).pixels==0);
 uint8_t vga[384]={},gif[384]={};
 auto a=rmPaletteOverlap(vga,384,gif,384);
 assert(a.equal==128&&a.different==0);
 vga[0]=63;gif[0]=255;
 a=rmPaletteOverlap(vga,384,gif,384);
 assert(a.equal==128&&a.different==0);
 gif[0]=254;
 a=rmPaletteOverlap(vga,384,gif,384);
 assert(a.equal==127&&a.different==1);
 assert(rmPaletteOverlap(vga,383,gif,384).equal==0);
 return 0;
}
