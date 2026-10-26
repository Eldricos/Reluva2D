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

 uint8_t shifted[768]={};
 // Unique VGA6 colors: 128 reference entries and a shifted candidate table.
 for(unsigned i=0;i<128;++i){
  vga[3*i]=uint8_t(i%64);
  vga[3*i+1]=uint8_t(i/64);
  vga[3*i+2]=uint8_t((i*3)%64);
  const unsigned j=i+64;
  for(unsigned c=0;c<3;++c)
   shifted[3*j+c]=uint8_t((unsigned(vga[3*i+c])*255u+31u)/63u);
 }
 const auto shiftedResult=rmPaletteOffsets(vga,384,shifted,768);
 assert(shiftedResult.valid&&shiftedResult.bestOffset==64);
 assert(shiftedResult.bestMatches==128&&shiftedResult.bestCompared==128);
 assert(shiftedResult.rgbMembership==128);
 assert(!rmPaletteOffsets(vga,383,shifted,768).valid);
 vga[0]=64;
 assert(!rmPaletteOffsets(vga,384,shifted,768).valid);
 uint8_t rm[6+384]={};
 uint8_t standard[384]={};
 for(size_t i=0;i<384;++i){rm[6+i]=uint8_t(i%64);standard[i]=uint8_t(i%64);}
 auto tail=rmTrailingPalette(rm,sizeof(rm),standard,sizeof(standard),6);
 assert(tail.present&&tail.plausibleVga6&&tail.channelsWithinVga6==384);
 assert(tail.sameBytesAsStandard==384&&tail.sameEntriesAsStandard==128);
 rm[6+9]=64;
 tail=rmTrailingPalette(rm,sizeof(rm),standard,sizeof(standard),6);
 assert(tail.present&&!tail.plausibleVga6&&tail.channelsWithinVga6==383);
 assert(tail.sameBytesAsStandard==383&&tail.sameEntriesAsStandard==127);
 assert(!rmTrailingPalette(rm,sizeof(rm)-1,standard,sizeof(standard),6).present);
 assert(!rmTrailingPalette(nullptr,sizeof(rm),standard,sizeof(standard),6).present);
 uint8_t boundaryData[2+12]={0,0,1,1,1,2,2,2,3,3,3,3,4,4};
 const auto boundary=rmBoundary(boundaryData,sizeof(boundaryData),2,4,3);
 assert(boundary.valid&&boundary.coverage.pixels==12);
 assert(boundary.adjacentEqual==6);
 assert(!rmBoundary(boundaryData,sizeof(boundaryData),3,4,3).valid);
 assert(!rmBoundary(nullptr,sizeof(boundaryData),0,4,3).valid);
 assert(!rmBoundary(boundaryData,sizeof(boundaryData),0,0,3).valid);
 assert(rmEqualRegion(boundaryData,sizeof(boundaryData),boundaryData,sizeof(boundaryData),0,14)==14);
 assert(rmEqualRegion(boundaryData,sizeof(boundaryData),boundaryData,sizeof(boundaryData),10,5)==0);
 return 0;
}
