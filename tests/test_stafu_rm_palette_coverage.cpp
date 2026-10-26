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
 uint8_t first[384]={};
 uint8_t ref[384]={};
 for(size_t i=0;i<128;++i){
  first[3*i]=uint8_t(i%64);
  first[3*i+1]=uint8_t(i/64);
  first[3*i+2]=0;
 }
 for(size_t i=0;i<384;++i)ref[i]=first[i];
 auto leading=rmLeadingPalette(first,sizeof(first),ref,sizeof(ref));
 assert(leading.present&&leading.plausibleVga6);
 assert(leading.channelsWithinVga6==384&&leading.uniqueColors==128);
 assert(leading.standardByteMatches==384&&leading.standardEntryMatches==128);
 first[0]=64;
 leading=rmLeadingPalette(first,sizeof(first),ref,sizeof(ref));
 assert(!leading.plausibleVga6&&leading.channelsWithinVga6==383);
 assert(leading.standardByteMatches==383&&leading.standardEntryMatches==127);
 assert(!rmLeadingPalette(first,383,ref,sizeof(ref)).present);
 assert(!rmLeadingPalette(nullptr,384,ref,sizeof(ref)).present);
 const uint8_t flags[]={0,128,1,129,128,128,129,1};
 const auto bits=rmHighBitAnalysis(flags,sizeof(flags),0,4,2);
 assert(bits.valid&&bits.lowPixels==3&&bits.highPixels==5);
 assert(bits.rowsMixed==2&&bits.rowsLowOnly==0&&bits.rowsHighOnly==0);
 assert(bits.pairedIndices==2&&bits.lowOnlyIndices==0&&bits.highOnlyIndices==0);
 assert(bits.horizontalPairs==6&&bits.horizontalBitTransitions==4);
 assert(bits.horizontalMaskedEqual==4&&bits.horizontalFlagOnlyTransitions==3);
 assert(bits.verticalPairs==4&&bits.verticalBitTransitions==3);
 assert(bits.verticalMaskedEqual==4&&bits.verticalFlagOnlyTransitions==3);
 assert(!rmHighBitAnalysis(flags,sizeof(flags),1,4,2).valid);
 assert(!rmHighBitAnalysis(nullptr,sizeof(flags),0,4,2).valid);
 assert(!rmHighBitAnalysis(flags,sizeof(flags),0,0,2).valid);
 const uint8_t regions[]={
  0,0,128,128,
  0,0,128,128,
  1,1,129,129,
  1,1,129,129
 };
 const auto grid=rmRegionGrid(regions,sizeof(regions),0,4,4);
 assert(grid.valid);
 assert(grid.cells[0].x0==0&&grid.cells[0].y0==0);
 assert(grid.cells[0].width==1&&grid.cells[0].height==1);
 assert(grid.cells[0].low==1&&grid.cells[0].high==0);
 assert(grid.cells[0].topIndex==0&&grid.cells[0].topCount==1);
 assert(grid.cells[2].high==1&&grid.cells[2].distinctHigh==1);
 assert(grid.cells[8].low==1&&grid.cells[8].topIndex==1);
 assert(grid.cells[10].high==1&&grid.cells[10].topIndex==129);
 assert(!rmRegionGrid(regions,sizeof(regions)-1,0,4,4).valid);
 assert(!rmRegionGrid(nullptr,sizeof(regions),0,4,4).valid);
 assert(!rmRegionGrid(regions,sizeof(regions),0,3,4).valid);
 const uint8_t scanBytes[]={0,0,128,128, 7,7,7,7, 129,1,129,1};
 const auto scan=rmScanlines(scanBytes,sizeof(scanBytes),0,4,3);
 assert(scan.valid&&scan.height==3);
 assert(scan.rows[0].low==2&&scan.rows[0].high==2);
 assert(scan.rows[0].transitions==1&&scan.rows[0].longestConstantRun==2);
 assert(scan.rows[0].dominantIndex==0&&scan.rows[0].dominantCount==2);
 assert(scan.rows[1].low==4&&scan.rows[1].high==0);
 assert(scan.rows[1].transitions==0&&scan.rows[1].longestConstantRun==4);
 assert(scan.rows[2].low==2&&scan.rows[2].high==2);
 assert(scan.rows[2].transitions==3&&scan.rows[2].longestConstantRun==1);
 assert(!rmScanlines(scanBytes,sizeof(scanBytes)-1,0,4,3).valid);
 assert(!rmScanlines(nullptr,sizeof(scanBytes),0,4,3).valid);
 assert(!rmScanlines(scanBytes,sizeof(scanBytes),0,4,481).valid);
 const uint8_t boundaryRowBytes[]={9,9,128,128,128,1,1,1,1,1};
 const auto detail=rmBoundaryRow(boundaryRowBytes,sizeof(boundaryRowBytes),0,0,10,1);
 assert(detail.valid&&detail.low==7&&detail.high==3);
 assert(detail.runCount==3&&detail.counts[9]==2&&detail.counts[128]==3&&detail.counts[1]==5);
 assert(detail.longestRuns[0].x==5&&detail.longestRuns[0].length==5&&detail.longestRuns[0].index==1);
 assert(detail.longestRuns[1].x==2&&detail.longestRuns[1].length==3&&detail.longestRuns[1].index==128);
 assert(detail.longestRuns[2].x==0&&detail.longestRuns[2].length==2&&detail.longestRuns[2].index==9);
 assert(!rmBoundaryRow(boundaryRowBytes,sizeof(boundaryRowBytes)-1,0,0,10,1).valid);
 assert(!rmBoundaryRow(boundaryRowBytes,sizeof(boundaryRowBytes),1,0,10,1).valid);
 assert(!rmBoundaryRow(nullptr,sizeof(boundaryRowBytes),0,0,10,1).valid);
 const uint8_t sharedA[]={1,2,3,4, 5,6,7,8};
 const uint8_t sharedB[]={1,2,9,4, 5,6,7,8};
 const auto shared=rmSharedRaster(sharedA,sizeof(sharedA),sharedB,sizeof(sharedB),0,1,0,4,2);
 assert(shared.valid&&shared.pixels==8&&shared.exact==7&&shared.maskedEqual==7);
 assert(shared.identicalRows==1&&shared.rowMatches[0]==3&&shared.rowMatches[1]==4);
 assert(shared.longestRun==4&&shared.longestX==0&&shared.longestY==1);
 assert(!rmSharedRaster(sharedA,7,sharedB,8,0,1,0,4,2).valid);
 assert(!rmSharedRaster(sharedA,8,sharedB,8,1,0,0,4,2).valid);
 assert(!rmSharedRaster(nullptr,8,sharedB,8,0,1,0,4,2).valid);
 const uint8_t diffA[]={1,2,3,4, 5,6,7,8};
 const uint8_t diffB[]={1,130,131,9, 133,134,7,8};
 const auto diff=rmDifferenceRegion(diffA,8,diffB,8,0,1,0,4,2);
 assert(diff.valid&&diff.exact==3&&diff.flagOnly==4&&diff.other==1);
 assert(diff.longestFlagRun==2&&diff.longestFlagX==1&&diff.longestFlagY==0);
 assert(diff.bandExact[0]==1&&diff.bandFlagOnly[1]==2);
 assert(!rmDifferenceRegion(diffA,7,diffB,8,0,1,0,4,2).valid);
 assert(!rmDifferenceRegion(diffA,8,diffB,8,1,0,0,4,2).valid);
 assert(!rmDifferenceRegion(nullptr,8,diffB,8,0,1,0,4,2).valid);
 return 0;
}
