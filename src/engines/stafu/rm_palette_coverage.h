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

}
