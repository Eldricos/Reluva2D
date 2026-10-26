// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
namespace reluva::stafu {
struct RmPaletteCoverage {
 size_t pixels=0,lowerPixels=0,upperPixels=0,lowerDistinct=0,upperDistinct=0;
 std::array<size_t,256> counts{};
};
inline RmPaletteCoverage rmPaletteCoverage(const uint8_t* bytes,size_t length,size_t raster=640u*480u){
 RmPaletteCoverage r;
 if(!bytes||length<raster)return r;
 r.pixels=raster;
 for(size_t i=0;i<raster;++i){
  const unsigned index=bytes[i];
  if(r.counts[index]++==0){if(index<128)++r.lowerDistinct;else ++r.upperDistinct;}
  if(index<128)++r.lowerPixels;else ++r.upperPixels;
 }
 return r;
}
struct RmPaletteOverlap {
 size_t equal=0,different=0;
};
inline RmPaletteOverlap rmPaletteOverlap(const uint8_t* vga6,size_t length,
                                         const uint8_t* gif,size_t gifLength){
 RmPaletteOverlap r;
 if(!vga6||length!=384||!gif||gifLength<384)return r;
 for(size_t i=0;i<128;++i){
  bool same=true;
  for(size_t c=0;c<3;++c){
   const unsigned expanded=(unsigned(vga6[3*i+c])*255u+31u)/63u;
   if(vga6[3*i+c]>63||expanded!=gif[3*i+c])same=false;
  }
  if(same)++r.equal;else ++r.different;
 }
 return r;
}

struct RmPaletteOffsetResult {
 size_t directMatches=0,bestMatches=0,bestCompared=0,rgbMembership=0;
 int bestOffset=0;
 bool valid=false;
};
// Compare 128 VGA6 reference colors with an RGB888 candidate palette.
// Positive offsets mean candidate[index + offset].
inline RmPaletteOffsetResult rmPaletteOffsets(const uint8_t* vga6,size_t vgaBytes,
                                              const uint8_t* rgb,size_t rgbBytes){
 RmPaletteOffsetResult out;
 if(!vga6||vgaBytes!=384||!rgb||rgbBytes<3||rgbBytes%3||rgbBytes>768)return out;
 for(size_t i=0;i<384;++i)if(vga6[i]>63)return out;
 out.valid=true;
 const int entries=int(rgbBytes/3);
 const auto equal=[&](int a,int b){
  for(int c=0;c<3;++c)
   if((unsigned(vga6[3*a+c])*255u+31u)/63u!=rgb[3*b+c])return false;
  return true;
 };
 for(int i=0;i<128;++i){
  if(i<entries&&equal(i,i))++out.directMatches;
  for(int j=0;j<entries;++j)if(equal(i,j)){++out.rgbMembership;break;}
 }
 bool haveBest=false;
 for(int offset=-(128-1);offset<entries;++offset){
  size_t matches=0,compared=0;
  for(int i=0;i<128;++i){
   const int j=i+offset;
   if(j<0||j>=entries)continue;
   ++compared;
   if(equal(i,j))++matches;
  }
  if(!compared)continue;
  // Prefer the greatest absolute evidence; resolve ties by coverage,
  // then the smallest absolute offset.
  const int absOffset=offset<0?-offset:offset;
  const int absBest=out.bestOffset<0?-out.bestOffset:out.bestOffset;
  if(!haveBest||matches>out.bestMatches||
     (matches==out.bestMatches&&(compared>out.bestCompared||
      (compared==out.bestCompared&&absOffset<absBest)))){
   out.bestOffset=offset;out.bestMatches=matches;out.bestCompared=compared;
   haveBest=true;
  }
 }
 return out;
}


struct RmTrailingPalette {
 bool present=false,plausibleVga6=false;
 size_t totalBytes=0,tailBytes=0,channelsWithinVga6=0;
 size_t sameBytesAsStandard=0,sameEntriesAsStandard=0;
 std::array<uint8_t,384> tail{};
};
inline RmTrailingPalette rmTrailingPalette(const uint8_t* bytes,size_t length,
                                           const uint8_t* standard,size_t standardLength,
                                           size_t raster=640u*480u){
 RmTrailingPalette r;
 r.totalBytes=length;
 if(!bytes||length<raster)return r;
 r.tailBytes=length-raster;
 if(r.tailBytes!=384)return r;
 r.present=true;
 for(size_t i=0;i<384;++i){
  const uint8_t value=bytes[raster+i];
  r.tail[i]=value;
  if(value<=63)++r.channelsWithinVga6;
 }
 r.plausibleVga6=r.channelsWithinVga6==384;
 if(standard&&standardLength==384){
  for(size_t i=0;i<384;++i)
   if(r.tail[i]==standard[i])++r.sameBytesAsStandard;
  for(size_t i=0;i<128;++i)
   if(r.tail[3*i]==standard[3*i]&&
      r.tail[3*i+1]==standard[3*i+1]&&
      r.tail[3*i+2]==standard[3*i+2])++r.sameEntriesAsStandard;
 }
 return r;
}


struct RmBoundary {
 bool valid=false;
 size_t offset=0;
 RmPaletteCoverage coverage{};
 size_t adjacentEqual=0,verticalEqual=0;
};
inline RmBoundary rmBoundary(const uint8_t* data,size_t length,size_t offset,
                             size_t width=640,size_t height=480){
 RmBoundary out;
 out.offset=offset;
 if(!data||!width||!height||width>SIZE_MAX/height)return out;
 const size_t raster=width*height;
 if(offset>length||raster>length-offset)return out;
 out.valid=true;
 out.coverage=rmPaletteCoverage(data+offset,length-offset,raster);
 for(size_t y=0;y<height;++y)
  for(size_t x=0;x<width;++x){
   const size_t i=offset+y*width+x;
   if(x&&data[i]==data[i-1])++out.adjacentEqual;
   if(y&&data[i]==data[i-width])++out.verticalEqual;
  }
 return out;
}
inline size_t rmEqualRegion(const uint8_t* a,size_t aLength,const uint8_t* b,
                            size_t bLength,size_t start,size_t count){
 if(!a||!b||start>aLength||start>bLength||
    count>aLength-start||count>bLength-start)return 0;
 size_t matches=0;
 for(size_t i=0;i<count;++i)if(a[start+i]==b[start+i])++matches;
 return matches;
}


struct RmLeadingPalette {
 bool present=false,plausibleVga6=false;
 size_t channelsWithinVga6=0,uniqueColors=0,standardByteMatches=0,standardEntryMatches=0;
 std::array<uint8_t,384> colors{};
};
inline RmLeadingPalette rmLeadingPalette(const uint8_t* bytes,size_t length,
                                          const uint8_t* standard,size_t standardLength){
 RmLeadingPalette r;
 if(!bytes||length<384)return r;
 r.present=true;
 for(size_t i=0;i<384;++i){
  r.colors[i]=bytes[i];
  if(bytes[i]<=63)++r.channelsWithinVga6;
  if(standard&&standardLength==384&&bytes[i]==standard[i])++r.standardByteMatches;
 }
 r.plausibleVga6=r.channelsWithinVga6==384;
 for(size_t i=0;i<128;++i){
  bool seen=false;
  for(size_t j=0;j<i;++j)
   if(bytes[3*i]==bytes[3*j]&&bytes[3*i+1]==bytes[3*j+1]&&
      bytes[3*i+2]==bytes[3*j+2]){seen=true;break;}
  if(!seen)++r.uniqueColors;
  if(standard&&standardLength==384&&
     bytes[3*i]==standard[3*i]&&bytes[3*i+1]==standard[3*i+1]&&
     bytes[3*i+2]==standard[3*i+2])++r.standardEntryMatches;
 }
 return r;
}


struct RmHighBitAnalysis {
 bool valid=false;
 size_t lowPixels=0,highPixels=0;
 size_t horizontalPairs=0,horizontalBitTransitions=0,horizontalMaskedEqual=0;
 size_t horizontalFlagOnlyTransitions=0;
 size_t verticalPairs=0,verticalBitTransitions=0,verticalMaskedEqual=0;
 size_t verticalFlagOnlyTransitions=0;
 size_t rowsLowOnly=0,rowsHighOnly=0,rowsMixed=0;
 size_t pairedIndices=0,lowOnlyIndices=0,highOnlyIndices=0;
 std::array<size_t,128> lowCounts{},highCounts{};
};
inline RmHighBitAnalysis rmHighBitAnalysis(const uint8_t* bytes,size_t length,
                                           size_t offset=384,size_t width=640,size_t height=480){
 RmHighBitAnalysis r;
 if(!bytes||!width||!height||width>SIZE_MAX/height)return r;
 const size_t raster=width*height;
 if(offset>length||raster>length-offset)return r;
 r.valid=true;
 for(size_t y=0;y<height;++y){
  size_t rowLow=0,rowHigh=0;
  for(size_t x=0;x<width;++x){
   const size_t i=offset+y*width+x;
   const uint8_t v=bytes[i];
   const unsigned index=v&127u;
   if(v&128u){++r.highPixels;++r.highCounts[index];++rowHigh;}
   else {++r.lowPixels;++r.lowCounts[index];++rowLow;}
   if(x){
    const uint8_t prev=bytes[i-1];
    ++r.horizontalPairs;
    if(((prev^v)&128u)!=0)++r.horizontalBitTransitions;
    if((prev&127u)==index){
     ++r.horizontalMaskedEqual;
     if(((prev^v)&128u)!=0)++r.horizontalFlagOnlyTransitions;
    }
   }
   if(y){
    const uint8_t prev=bytes[i-width];
    ++r.verticalPairs;
    if(((prev^v)&128u)!=0)++r.verticalBitTransitions;
    if((prev&127u)==index){
     ++r.verticalMaskedEqual;
     if(((prev^v)&128u)!=0)++r.verticalFlagOnlyTransitions;
    }
   }
  }
  if(rowLow&&rowHigh)++r.rowsMixed;
  else if(rowHigh)++r.rowsHighOnly;
  else ++r.rowsLowOnly;
 }
 for(size_t i=0;i<128;++i){
  if(r.lowCounts[i]&&r.highCounts[i])++r.pairedIndices;
  else if(r.lowCounts[i])++r.lowOnlyIndices;
  else if(r.highCounts[i])++r.highOnlyIndices;
 }
 return r;
}


struct RmRegion {
 size_t x0=0,y0=0,width=0,height=0,low=0,high=0,distinctLow=0,distinctHigh=0;
 unsigned topIndex=0;
 size_t topCount=0;
};
struct RmRegionGrid {
 bool valid=false;
 std::array<RmRegion,16> cells{};
};
inline RmRegionGrid rmRegionGrid(const uint8_t* bytes,size_t length,
                                  size_t offset=384,size_t width=640,size_t height=480){
 RmRegionGrid out;
 if(!bytes||width<4||height<4||width>SIZE_MAX/height)return out;
 const size_t raster=width*height;
 if(offset>length||raster>length-offset)return out;
 out.valid=true;
 for(size_t gy=0;gy<4;++gy)for(size_t gx=0;gx<4;++gx){
  auto& cell=out.cells[gy*4+gx];
  cell.x0=gx*width/4;
  cell.y0=gy*height/4;
  const size_t xEnd=(gx+1)*width/4,yEnd=(gy+1)*height/4;
  cell.width=xEnd-cell.x0;
  cell.height=yEnd-cell.y0;
  std::array<size_t,256> counts{};
  for(size_t y=cell.y0;y<yEnd;++y)
   for(size_t x=cell.x0;x<xEnd;++x){
    const unsigned v=bytes[offset+y*width+x];
    if(v&128u)++cell.high;else ++cell.low;
    if(++counts[v]>cell.topCount){cell.topCount=counts[v];cell.topIndex=v;}
   }
  for(size_t i=0;i<256;++i)if(counts[i]){
   if(i<128)++cell.distinctLow;else ++cell.distinctHigh;
  }
 }
 return out;
}


struct RmScanline {
 size_t low=0,high=0,transitions=0,longestConstantRun=0;
 unsigned dominantIndex=0;
 size_t dominantCount=0;
};
struct RmScanlineSurvey {
 bool valid=false;
 size_t height=0;
 std::array<RmScanline,480> rows{};
};
inline RmScanlineSurvey rmScanlines(const uint8_t* bytes,size_t length,
                                    size_t offset=384,size_t width=640,size_t height=480){
 RmScanlineSurvey out;
 if(!bytes||!width||!height||height>out.rows.size()||width>SIZE_MAX/height)return out;
 const size_t raster=width*height;
 if(offset>length||raster>length-offset)return out;
 out.valid=true;out.height=height;
 for(size_t y=0;y<height;++y){
  auto& row=out.rows[y];
  std::array<size_t,256> counts{};
  size_t run=0;
  for(size_t x=0;x<width;++x){
   const uint8_t v=bytes[offset+y*width+x];
   if(v&128u)++row.high;else ++row.low;
   if(++counts[v]>row.dominantCount){row.dominantCount=counts[v];row.dominantIndex=v;}
   if(x==0||v!=bytes[offset+y*width+x-1])run=1;
   else ++run;
   if(run>row.longestConstantRun)row.longestConstantRun=run;
   if(x&&((v^bytes[offset+y*width+x-1])&128u))++row.transitions;
  }
 }
 return out;
}


struct RmBoundaryRun {
 size_t x=0,length=0;
 unsigned index=0;
};
struct RmBoundaryRow {
 bool valid=false;
 size_t y=0,low=0,high=0,runCount=0;
 std::array<RmBoundaryRun,3> longestRuns{};
 std::array<size_t,256> counts{};
};
inline RmBoundaryRow rmBoundaryRow(const uint8_t* bytes,size_t length,size_t y,
                                   size_t offset=384,size_t width=640,size_t height=480){
 RmBoundaryRow out;
 if(!bytes||!width||!height||y>=height||width>SIZE_MAX/height)return out;
 const size_t raster=width*height;
 if(offset>length||raster>length-offset)return out;
 out.valid=true;out.y=y;
 size_t start=0;
 for(size_t x=0;x<width;++x){
  const unsigned v=bytes[offset+y*width+x];
  ++out.counts[v];
  if(v&128u)++out.high;else ++out.low;
  if(x+1<width&&bytes[offset+y*width+x+1]==v)continue;
  const RmBoundaryRun run{start,x+1-start,v};
  ++out.runCount;
  for(size_t k=0;k<out.longestRuns.size();++k)
   if(run.length>out.longestRuns[k].length){
    for(size_t j=out.longestRuns.size()-1;j>k;--j)
     out.longestRuns[j]=out.longestRuns[j-1];
    out.longestRuns[k]=run;
    break;
   }
  start=x+1;
 }
 return out;
}


struct RmSharedRaster {
 bool valid=false;
 size_t pixels=0,exact=0,maskedEqual=0,longestRun=0,longestX=0,longestY=0;
 size_t identicalRows=0;
 std::array<size_t,480> rowMatches{};
};
inline RmSharedRaster rmSharedRaster(const uint8_t* a,size_t aLength,
 const uint8_t* b,size_t bLength,size_t firstY,size_t lastY,
 size_t offset=384,size_t width=640,size_t height=480){
 RmSharedRaster out;
 if(!a||!b||!width||!height||height>out.rowMatches.size()||
    firstY>lastY||lastY>=height||width>SIZE_MAX/height)return out;
 const size_t raster=width*height;
 if(offset>aLength||raster>aLength-offset||offset>bLength||raster>bLength-offset)return out;
 out.valid=true;
 for(size_t y=firstY;y<=lastY;++y){
  size_t run=0;
  for(size_t x=0;x<width;++x){
   const size_t i=offset+y*width+x;
   ++out.pixels;
   if((a[i]&127u)==(b[i]&127u))++out.maskedEqual;
   if(a[i]==b[i]){
    ++out.exact;++out.rowMatches[y];++run;
    if(run>out.longestRun){out.longestRun=run;out.longestX=x+1-run;out.longestY=y;}
   }else run=0;
  }
  if(out.rowMatches[y]==width)++out.identicalRows;
 }
 return out;
}


struct RmDifferenceRegion {
 bool valid=false;
 size_t exact=0,flagOnly=0,other=0;
 size_t longestFlagRun=0,longestFlagX=0,longestFlagY=0;
 std::array<size_t,4> bandFlagOnly{};
 std::array<size_t,4> bandExact{};
 std::array<size_t,4> bandOther{};
};
inline RmDifferenceRegion rmDifferenceRegion(const uint8_t* a,size_t aLength,
 const uint8_t* b,size_t bLength,size_t firstY,size_t lastY,
 size_t offset=384,size_t width=640,size_t height=480){
 RmDifferenceRegion out;
 if(!a||!b||!width||!height||firstY>lastY||lastY>=height||
    width>SIZE_MAX/height)return out;
 const size_t raster=width*height;
 if(offset>aLength||raster>aLength-offset||offset>bLength||raster>bLength-offset)return out;
 out.valid=true;
 for(size_t y=firstY;y<=lastY;++y){
  size_t run=0;
  for(size_t x=0;x<width;++x){
   const size_t i=offset+y*width+x;
   const size_t band=x*4/width;
   if(a[i]==b[i]){++out.exact;++out.bandExact[band];run=0;}
   else if((a[i]&127u)==(b[i]&127u)){
    ++out.flagOnly;++out.bandFlagOnly[band];++run;
    if(run>out.longestFlagRun){
     out.longestFlagRun=run;out.longestFlagX=x+1-run;out.longestFlagY=y;
    }
   }else{++out.other;++out.bandOther[band];run=0;}
  }
 }
 return out;
}


struct RmFlagPairs {
 bool valid=false;
 size_t lowToHigh=0,highToLow=0;
 std::array<size_t,128> lowToHighByBase{};
 std::array<size_t,128> highToLowByBase{};
};
inline RmFlagPairs rmFlagPairs(const uint8_t* a,size_t aLength,
 const uint8_t* b,size_t bLength,size_t firstY,size_t lastY,
 size_t offset=384,size_t width=640,size_t height=480){
 RmFlagPairs out;
 if(!a||!b||!width||!height||firstY>lastY||lastY>=height||
    width>SIZE_MAX/height)return out;
 const size_t raster=width*height;
 if(offset>aLength||raster>aLength-offset||offset>bLength||raster>bLength-offset)return out;
 out.valid=true;
 for(size_t y=firstY;y<=lastY;++y)
  for(size_t x=0;x<width;++x){
   const size_t i=offset+y*width+x;
   if((a[i]^b[i])!=128u)continue;
   const size_t base=a[i]&127u;
   if(a[i]&128u){++out.highToLow;++out.highToLowByBase[base];}
   else{++out.lowToHigh;++out.lowToHighByBase[base];}
  }
 return out;
}


struct RmZero128Geometry {
 bool valid=false;
 size_t total=0,rows=0,minX=0,minY=0,maxX=0,maxY=0;
 size_t longestRun=0,longestX=0,longestY=0;
 std::array<size_t,480> rowCounts{};
 std::array<size_t,480> rowFirstX{};
 std::array<size_t,480> rowLastX{};
};
inline RmZero128Geometry rmZero128Geometry(const uint8_t* bridge,size_t bridgeLength,
 const uint8_t* view,size_t viewLength,size_t firstY,size_t lastY,
 size_t offset=384,size_t width=640,size_t height=480){
 RmZero128Geometry out;
 if(!bridge||!view||!width||!height||height>out.rowCounts.size()||
    firstY>lastY||lastY>=height||width>SIZE_MAX/height)return out;
 const size_t raster=width*height;
 if(offset>bridgeLength||raster>bridgeLength-offset||
    offset>viewLength||raster>viewLength-offset)return out;
 out.valid=true;
 for(size_t y=firstY;y<=lastY;++y){
  size_t run=0;
  for(size_t x=0;x<width;++x){
   const size_t i=offset+y*width+x;
   if(bridge[i]!=128u||view[i]!=0u){run=0;continue;}
   if(!out.total){out.minX=out.maxX=x;out.minY=out.maxY=y;}
   if(x<out.minX)out.minX=x;
   if(x>out.maxX)out.maxX=x;
   out.maxY=y;
   if(!out.rowCounts[y]){++out.rows;out.rowFirstX[y]=x;}
   out.rowLastX[y]=x;
   ++out.rowCounts[y];++out.total;++run;
   if(run>out.longestRun){out.longestRun=run;out.longestX=x+1-run;out.longestY=y;}
  }
 }
 return out;
}

}
