// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/rm_palette_coverage.h"
#include "engines/stafu/rm_candidate_palette.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-rm-palette-coverage <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager resources;std::string error;
 if(!resources.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 std::vector<uint8_t> standard;
 if(!resources.read("standard.pal",standard,error,4096)||standard.size()!=384){
  std::cerr<<"standard.pal unavailable or unexpected size: "<<error<<"\n";return 1;
 }
 struct Candidate {const char* name;reluva::stafu::RmCandidatePalette palette;};
 std::vector<Candidate> candidates;
 for(const char* name:{"BRIDGE.IMG","DBRIDGE.IMG","color.img"}){
  std::vector<uint8_t> bytes;std::string readError;
  Candidate c{name,{}};
  if(resources.read(name,bytes,readError,1024*1024))
   c.palette=reluva::stafu::extractRmGifPalette(bytes.data(),bytes.size());
  else c.palette.error=readError;
  candidates.push_back(c);
 }
 for(const auto& c:candidates){
  if(!c.palette.valid){std::cout<<"PALETTE "<<c.name<<" unavailable="<<c.palette.error<<"\n";continue;}
  const auto overlap=reluva::stafu::rmPaletteOverlap(standard.data(),standard.size(),c.palette.rgb.data(),c.palette.entries*3);
  std::cout<<"PALETTE "<<c.name<<" entries="<<c.palette.entries
           <<" standard128Equal="<<overlap.equal<<" different="<<overlap.different<<"\n";
  const auto offsets=reluva::stafu::rmPaletteOffsets(
   standard.data(),standard.size(),c.palette.rgb.data(),c.palette.entries*3);
  if(!offsets.valid){std::cerr<<"Invalid palette data for "<<c.name<<"\n";return 1;}
  std::cout<<"PALETTE_OFFSET "<<c.name<<"\n"
           <<"  DIRECT matches="<<offsets.directMatches<<"\n"
           <<"  BEST_OFFSET offset="<<offsets.bestOffset
           <<" matches="<<offsets.bestMatches<<" compared="<<offsets.bestCompared
           <<" percent="<<(100.0*offsets.bestMatches/offsets.bestCompared)<<"\n"
           <<"  RGB_MEMBERSHIP matches="<<offsets.rgbMembership<<" of=128\n";

 }
 std::vector<std::pair<std::string,reluva::stafu::RmTrailingPalette>> tails;
 std::vector<std::pair<std::string,std::vector<uint8_t>>> rmFiles;
 std::vector<std::pair<std::string,reluva::stafu::RmLeadingPalette>> leading;
 auto hex64=[](const std::vector<uint8_t>& bytes){
  std::ostringstream out;
  const size_t count=std::min<size_t>(64,bytes.size());
  for(size_t i=0;i<count;++i){
   if(i)out<<" ";
   out<<std::hex<<std::setfill('0')<<std::setw(2)<<unsigned(bytes[i]);
  }
  return out.str();
 };
 for(const char* name:{"BRIDGE.RM","VIEWSCR.RM"}){
  std::vector<uint8_t> bytes;
  if(!resources.read(name,bytes,error,2*1024*1024)){std::cerr<<name<<": "<<error<<"\n";return 1;}
  const auto r=reluva::stafu::rmPaletteCoverage(bytes.data(),bytes.size());
  if(!r.pixels){std::cerr<<name<<": insufficient raster bytes\n";return 1;}
  const auto tail=reluva::stafu::rmTrailingPalette(bytes.data(),bytes.size(),standard.data(),standard.size());
  std::cout<<"RM_TRAILER "<<name<<" total="<<tail.totalBytes
           <<" remainder="<<tail.tailBytes<<" candidate384="<<tail.present
           <<" vga6Channels="<<tail.channelsWithinVga6
           <<" plausibleVga6="<<tail.plausibleVga6
           <<" standardByteMatches="<<tail.sameBytesAsStandard
           <<" standardEntryMatches="<<tail.sameEntriesAsStandard<<"\n";
  tails.emplace_back(name,tail);
  const auto first=reluva::stafu::rmLeadingPalette(bytes.data(),bytes.size(),standard.data(),standard.size());
  std::cout<<"RM_LEADING_PALETTE "<<name<<" present="<<first.present
           <<" vga6Channels="<<first.channelsWithinVga6<<" of=384"
           <<" plausibleVga6="<<first.plausibleVga6
           <<" uniqueRGBTriplets="<<first.uniqueColors<<" of=128"
           <<" standardByteMatches="<<first.standardByteMatches
           <<" standardEntryMatches="<<first.standardEntryMatches<<"\n";
  leading.emplace_back(name,first);
  std::cout<<"RM_FIRST64 "<<name<<" "<<hex64(bytes)<<"\n";
  for(size_t offset:{size_t(0),size_t(384)}){
   const auto boundary=reluva::stafu::rmBoundary(bytes.data(),bytes.size(),offset);
   std::cout<<"RM_BOUNDARY "<<name<<" offset="<<offset<<" valid="<<boundary.valid;
   if(boundary.valid)
    std::cout<<" lower="<<boundary.coverage.lowerPixels
             <<" upper="<<boundary.coverage.upperPixels
             <<" horizontalEqual="<<boundary.adjacentEqual
             <<" verticalEqual="<<boundary.verticalEqual;
   std::cout<<"\n";
  }
  for(size_t pos:{size_t(0),size_t(2),size_t(4),size_t(6),size_t(8),size_t(10),size_t(12),size_t(14)}){
   if(pos+1>=bytes.size())break;
   const unsigned le=unsigned(bytes[pos])|(unsigned(bytes[pos+1])<<8);
   const unsigned be=(unsigned(bytes[pos])<<8)|unsigned(bytes[pos+1]);
   if(le==640||le==480||be==640||be==480)
    std::cout<<"RM_DIMENSION_HINT "<<name<<" offset="<<pos<<" le16="<<le<<" be16="<<be<<"\n";
  }
  const auto bits=reluva::stafu::rmHighBitAnalysis(bytes.data(),bytes.size());
  std::cout<<"RM_HIGHBIT "<<name<<" valid="<<bits.valid
           <<" lowPixels="<<bits.lowPixels<<" highPixels="<<bits.highPixels
           <<" pairedIndices="<<bits.pairedIndices
           <<" lowOnlyIndices="<<bits.lowOnlyIndices
           <<" highOnlyIndices="<<bits.highOnlyIndices
           <<" rowsMixed="<<bits.rowsMixed
           <<" rowsLowOnly="<<bits.rowsLowOnly
           <<" rowsHighOnly="<<bits.rowsHighOnly<<"\n";
  std::cout<<"RM_HIGHBIT_NEIGHBORS "<<name
           <<" horizontalPairs="<<bits.horizontalPairs
           <<" bitTransitions="<<bits.horizontalBitTransitions
           <<" maskedEqual="<<bits.horizontalMaskedEqual
           <<" flagOnlyTransitions="<<bits.horizontalFlagOnlyTransitions
           <<" verticalPairs="<<bits.verticalPairs
           <<" verticalBitTransitions="<<bits.verticalBitTransitions
           <<" verticalMaskedEqual="<<bits.verticalMaskedEqual
           <<" verticalFlagOnlyTransitions="<<bits.verticalFlagOnlyTransitions<<"\n";
  const auto grid=reluva::stafu::rmRegionGrid(bytes.data(),bytes.size());
  if(!grid.valid){std::cerr<<name<<": invalid RM region grid\n";return 1;}
  for(size_t gy=0;gy<4;++gy)for(size_t gx=0;gx<4;++gx){
   const auto& c=grid.cells[gy*4+gx];
   std::cout<<"RM_REGION "<<name<<" col="<<gx<<" row="<<gy
            <<" x="<<c.x0<<" y="<<c.y0<<" width="<<c.width<<" height="<<c.height
            <<" low="<<c.low<<" high="<<c.high
            <<" highPercent="<<(100.0*double(c.high)/double(c.low+c.high))
            <<" distinctLow="<<c.distinctLow<<" distinctHigh="<<c.distinctHigh
            <<" topIndex="<<c.topIndex<<" topCount="<<c.topCount<<"\n";
  }
  const auto scan=reluva::stafu::rmScanlines(bytes.data(),bytes.size());
  if(!scan.valid){std::cerr<<name<<": invalid scanlines\n";return 1;}
  std::vector<size_t> jumps;
  for(size_t y=1;y<scan.height;++y)jumps.push_back(y);
  std::sort(jumps.begin(),jumps.end(),[&](size_t a,size_t b){
   const auto difference=[&](size_t y){
    const size_t now=scan.rows[y].high,prev=scan.rows[y-1].high;
    return now>prev?now-prev:prev-now;
   };
   return difference(a)!=difference(b)?difference(a)>difference(b):a<b;
  });
  for(size_t i=0;i<std::min<size_t>(12,jumps.size());++i){
   const size_t y=jumps[i];
   const auto& before=scan.rows[y-1];
   const auto& after=scan.rows[y];
   std::cout<<"RM_SCANLINE_JUMP "<<name<<" y="<<y
            <<" previousHigh="<<before.high<<" currentHigh="<<after.high
            <<" delta="<<(static_cast<long long>(after.high)-static_cast<long long>(before.high))
            <<" transitions="<<after.transitions
            <<" topIndex="<<after.dominantIndex
            <<" topCount="<<after.dominantCount
            <<" longestConstantRun="<<after.longestConstantRun<<"\n";
  }
  for(size_t y=0;y<scan.height;){
   const bool allLow=scan.rows[y].high==0;
   const bool allHigh=scan.rows[y].low==0;
   if(!allLow&&!allHigh){++y;continue;}
   size_t end=y+1;
   while(end<scan.height&&
         (allLow?scan.rows[end].high==0:scan.rows[end].low==0))++end;
   std::cout<<"RM_SCANLINE_SOLID_BIT "<<name<<" firstY="<<y
            <<" lastY="<<(end-1)<<" rows="<<(end-y)
            <<" state="<<(allLow?"low":"high")<<"\n";
   y=end;
  }
  for(size_t y=0;y<scan.height;++y){
   const auto& row=scan.rows[y];
   if(row.longestConstantRun>=320)
    std::cout<<"RM_SCANLINE_UNIFORM "<<name<<" y="<<y
             <<" longestConstantRun="<<row.longestConstantRun
             <<" topIndex="<<row.dominantIndex<<" topCount="<<row.dominantCount
             <<" high="<<row.high<<"\n";
  }
  const std::vector<size_t> focus=std::string(name)=="BRIDGE.RM"
   ?std::vector<size_t>{363,364,365}
   :std::vector<size_t>{69,70,71,305,306,307,308,309,384,385,386,476,477,478,479};
  for(size_t y:focus){
   const auto row=reluva::stafu::rmBoundaryRow(bytes.data(),bytes.size(),y);
   if(!row.valid){std::cerr<<name<<": invalid boundary row\n";return 1;}
   std::vector<unsigned> ranked;
   for(unsigned v=0;v<256;++v)if(row.counts[v])ranked.push_back(v);
   std::sort(ranked.begin(),ranked.end(),[&](unsigned a,unsigned b){
    return row.counts[a]!=row.counts[b]?row.counts[a]>row.counts[b]:a<b;
   });
   std::cout<<"RM_BOUNDARY_ROW "<<name<<" y="<<y
            <<" low="<<row.low<<" high="<<row.high<<" runs="<<row.runCount;
   for(size_t k=0;k<std::min<size_t>(3,ranked.size());++k)
    std::cout<<" top"<<k<<"="<<ranked[k]<<":"<<row.counts[ranked[k]];
   for(size_t k=0;k<row.longestRuns.size();++k){
    const auto& run=row.longestRuns[k];
    if(run.length)std::cout<<" run"<<k<<"="<<run.x<<"-"<<(run.x+run.length-1)
                           <<":"<<run.index;
   }
   std::cout<<"\n";
  }
  rmFiles.emplace_back(name,std::move(bytes));
  std::cout<<"RESOURCE "<<name<<" pixels="<<r.pixels
           <<" index0to127="<<r.lowerPixels<<" distinct="<<r.lowerDistinct
           <<" index128to255="<<r.upperPixels<<" distinct="<<r.upperDistinct<<"\n";
  std::vector<unsigned> upper;
  for(unsigned i=128;i<256;++i)if(r.counts[i])upper.push_back(i);
  std::sort(upper.begin(),upper.end(),[&](unsigned a,unsigned b){
   return r.counts[a]!=r.counts[b]?r.counts[a]>r.counts[b]:a<b;
  });
  std::cout<<"UPPER_TOP";
  for(size_t i=0;i<std::min<size_t>(16,upper.size());++i)
   std::cout<<" "<<upper[i]<<":"<<r.counts[upper[i]];
  std::cout<<"\nUPPER_USED";
  for(unsigned i=128;i<256;++i)if(r.counts[i])std::cout<<" "<<i;
  std::cout<<"\n";
 }
 if(leading.size()==2&&leading[0].second.present&&leading[1].second.present){
  const auto& a=leading[0].second.colors;
  const auto& b=leading[1].second.colors;
  size_t sameBytes=0,sameEntries=0;
  for(size_t i=0;i<384;++i)if(a[i]==b[i])++sameBytes;
  for(size_t i=0;i<128;++i)
   if(a[3*i]==b[3*i]&&a[3*i+1]==b[3*i+1]&&a[3*i+2]==b[3*i+2])++sameEntries;
  std::cout<<"RM_LEADING_COMPARE equalBytes="<<sameBytes<<" of=384"
           <<" equalEntries="<<sameEntries<<" of=128\n";
 }
 if(rmFiles.size()==2){
  const auto& a=rmFiles[0].second;
  const auto& b=rmFiles[1].second;
  const size_t n=std::min<size_t>(384,std::min(a.size(),b.size()));
  std::cout<<"RM_COMPARE_FIRST384 equal="
           <<reluva::stafu::rmEqualRegion(a.data(),a.size(),b.data(),b.size(),0,n)
           <<" of="<<n<<"\n";
  if(a.size()>=384&&b.size()>=384){
   size_t same=0;
   for(size_t i=0;i<384;++i)
    if(a[a.size()-384+i]==b[b.size()-384+i])++same;
   std::cout<<"RM_COMPARE_LAST384 equal="<<same<<" of=384\n";
  }
 }
 if(tails.size()==2){
  const auto& a=tails[0].second;
  const auto& b=tails[1].second;
  if(a.present&&b.present){
   size_t same=0;
   for(size_t i=0;i<384;++i)if(a.tail[i]==b.tail[i])++same;
   std::cout<<"RM_TRAILER_COMPARE "<<tails[0].first<<" "<<tails[1].first
            <<" equalBytes="<<same<<" of=384 identical="<<(same==384)<<"\n";
  }else std::cout<<"RM_TRAILER_COMPARE unavailable=non384-remainder\n";
 }
 std::cout<<"NOTE: Trailer plausibility does not establish palette ownership.\n";
 std::cout<<"NOTE: Palette overlap is numerical only; it does not establish RM palette provenance.\n";
 return 0;
}
