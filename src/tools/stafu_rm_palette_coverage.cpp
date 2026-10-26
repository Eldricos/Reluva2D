// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/rm_palette_coverage.h"
#include "engines/stafu/rm_candidate_palette.h"
#include <algorithm>
#include <filesystem>
#include <cctype>
#include <fstream>
#include <iterator>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-rm-palette-coverage <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager resources;std::string error;
 if(!resources.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}

 // Milestone 95: inventory palette-related resource names, without assuming ownership.
 // Main archive inventory is complete when STTNG.ZIP opens; loose-file scan is top-level only.
 auto paletteName=[](std::string name){
  std::transform(name.begin(),name.end(),name.begin(),[](unsigned char ch){
   return static_cast<char>(std::tolower(ch));
  });
  const std::string extensions[]={".pal",".act",".clut",".rgb",".dac",".cmap"};
  for(const auto& ext:extensions)
   if(name.size()>=ext.size()&&name.compare(name.size()-ext.size(),ext.size(),ext)==0)
    return true;
  return name.find("palett")!=std::string::npos||
         name.find("color")!=std::string::npos||
         name.find("colour")!=std::string::npos;
 };
 std::vector<std::string> inventory;
 reluva::stafu::ZipArchive archive;
 std::string archiveError;
 const auto archivePath=std::filesystem::path(argv[2])/"STTNG.ZIP";
 if(archive.open(archivePath,archiveError)){
  size_t candidateCount=0;
  for(const auto& entry:archive.entries())
   if(paletteName(entry.name)){++candidateCount;inventory.push_back(entry.name);}
  std::sort(inventory.begin(),inventory.end());
  std::cout<<"RM_PALETTE_INVENTORY source=mainArchive entries="<<archive.size()
           <<" candidates="<<candidateCount<<"\n";
  for(size_t i=0;i<std::min<size_t>(60,inventory.size());++i)
   std::cout<<"RM_PALETTE_RESOURCE source=mainArchive name="<<inventory[i]<<"\n";
  if(inventory.size()>60)
   std::cout<<"RM_PALETTE_RESOURCE_TRUNCATED remaining="<<(inventory.size()-60)<<"\n";
 }else std::cout<<"RM_PALETTE_INVENTORY source=mainArchive unavailable="<<archiveError<<"\n";
 inventory.clear();
 std::error_code scanError;
 const std::filesystem::path looseRoot(argv[1]);
 if(std::filesystem::is_directory(looseRoot,scanError)){
  for(std::filesystem::directory_iterator it(looseRoot,scanError),end;
      !scanError&&it!=end;it.increment(scanError)){
   if(!it->is_regular_file(scanError))continue;
   const auto name=it->path().filename().string();
   if(paletteName(name))inventory.push_back(name);
  }
  std::sort(inventory.begin(),inventory.end());
  std::cout<<"RM_PALETTE_INVENTORY source=installedTopLevel candidates="
           <<inventory.size()<<" scanError="<<scanError.message()<<"\n";
  for(size_t i=0;i<std::min<size_t>(60,inventory.size());++i)
   std::cout<<"RM_PALETTE_RESOURCE source=installedTopLevel name="<<inventory[i]<<"\n";
 }else std::cout<<"RM_PALETTE_INVENTORY source=installedTopLevel unavailable=notDirectory\n";


 // Milestone 99: inventory actual .RM resources before choosing a third example.
 // Archive sizes are central-directory metadata; no extraction or modification.
 auto isRm=[](std::string name){
  std::transform(name.begin(),name.end(),name.begin(),[](unsigned char ch){
   return static_cast<char>(std::tolower(ch));
  });
  return name.size()>=3&&name.compare(name.size()-3,3,".rm")==0;
 };
 std::vector<std::pair<std::string,uint64_t>> rmArchive;
 std::string rmArchiveError;
 reluva::stafu::ZipArchive rmZip;
 if(rmZip.open(std::filesystem::path(argv[2])/"STTNG.ZIP",rmArchiveError)){
  for(const auto& entry:rmZip.entries())
   if(isRm(entry.name))rmArchive.emplace_back(entry.name,entry.uncompressedSize);
  std::sort(rmArchive.begin(),rmArchive.end());
  std::cout<<"RM_FILE_INVENTORY source=mainArchive totalEntries="<<rmZip.size()
           <<" rmCount="<<rmArchive.size()<<"\n";
  for(const auto& item:rmArchive)
   std::cout<<"RM_FILE_ENTRY source=mainArchive name="<<item.first
            <<" bytes="<<item.second
            <<" expected307584="<<(item.second==307584)<<"\n";
 }else std::cout<<"RM_FILE_INVENTORY source=mainArchive unavailable="
                 <<rmArchiveError<<"\n";
 std::vector<std::pair<std::string,uint64_t>> rmLoose;
 std::error_code rmScanError;
 if(std::filesystem::is_directory(looseRoot,rmScanError)){
  for(std::filesystem::directory_iterator it(looseRoot,rmScanError),end;
      !rmScanError&&it!=end;it.increment(rmScanError)){
   std::error_code fileError;
   if(!it->is_regular_file(fileError)||fileError)continue;
   const auto name=it->path().filename().string();
   if(!isRm(name))continue;
   const auto size=it->file_size(fileError);
   if(!fileError)rmLoose.emplace_back(name,size);
  }
  std::sort(rmLoose.begin(),rmLoose.end());
  std::cout<<"RM_FILE_INVENTORY source=installedTopLevel rmCount="
           <<rmLoose.size()<<" scanError="<<rmScanError.message()<<"\n";
  for(const auto& item:rmLoose)
   std::cout<<"RM_FILE_ENTRY source=installedTopLevel name="<<item.first
            <<" bytes="<<item.second
            <<" expected307584="<<(item.second==307584)<<"\n";
 }else std::cout<<"RM_FILE_INVENTORY source=installedTopLevel unavailable=notDirectory\n";

 // Milestone 96: bounded read-only ASCII reference survey of installed executable/overlays.
 // Substring hits are clues, not evidence of calls, loading order or palette ownership.
 const std::vector<std::string> referenceTerms={
  "BRIDGE.RM","VIEWSCR.RM","STANDARD.PAL","COLOR.IMG",
  "BRIDGE.IMG","DBRIDGE.IMG"
 };
 std::error_code referenceError;
 std::vector<std::filesystem::path> binaries;
 if(std::filesystem::is_directory(looseRoot,referenceError)){
  for(std::filesystem::directory_iterator it(looseRoot,referenceError),end;
      !referenceError&&it!=end;it.increment(referenceError)){
   if(!it->is_regular_file(referenceError))continue;
   std::string ext=it->path().extension().string();
   std::transform(ext.begin(),ext.end(),ext.begin(),[](unsigned char ch){
    return static_cast<char>(std::toupper(ch));
   });
   if(ext==".EXE"||ext==".OVL")binaries.push_back(it->path());
  }
 }
 std::sort(binaries.begin(),binaries.end());
 std::cout<<"RM_REFERENCE_FILES candidates="<<binaries.size()
          <<" scanError="<<referenceError.message()<<"\n";
 for(const auto& file:binaries){
  std::error_code sizeError;
  const auto fileSize=std::filesystem::file_size(file,sizeError);
  if(sizeError||fileSize>8ull*1024*1024){
   std::cout<<"RM_REFERENCE_FILE name="<<file.filename().string()
            <<" status=skippedSizeOrError\n";
   continue;
  }
  std::ifstream input(file,std::ios::binary);
  if(!input){
   std::cout<<"RM_REFERENCE_FILE name="<<file.filename().string()
            <<" status=unreadable\n";
   continue;
  }
  std::vector<unsigned char> raw((std::istreambuf_iterator<char>(input)),
                                  std::istreambuf_iterator<char>());
  std::string upper;
  upper.reserve(raw.size());
  for(unsigned char ch:raw)upper.push_back(static_cast<char>(std::toupper(ch)));
  std::vector<std::pair<size_t,size_t>> hits;
  for(size_t term=0;term<referenceTerms.size();++term){
   size_t pos=0;
   while((pos=upper.find(referenceTerms[term],pos))!=std::string::npos){
    hits.emplace_back(pos,term);
    ++pos;
   }
  }
  std::sort(hits.begin(),hits.end());
  std::cout<<"RM_REFERENCE_FILE name="<<file.filename().string()
           <<" bytes="<<raw.size()<<" hits="<<hits.size()<<"\n";
  for(size_t i=0;i<std::min<size_t>(100,hits.size());++i){
   const auto [offset,term]=hits[i];
   const size_t left=offset>64?offset-64:0;
   const size_t right=std::min(raw.size(),offset+referenceTerms[term].size()+64);
   std::string context;
   for(size_t j=left;j<right;++j){
    const unsigned char ch=raw[j];
    context.push_back(ch>=32&&ch<=126?static_cast<char>(ch):'.');
   }
   std::cout<<"RM_REFERENCE_HIT file="<<file.filename().string()
            <<" offset="<<offset<<" term="<<referenceTerms[term]
            <<" context="<<context<<"\n";
  }
  if(hits.size()>100)
   std::cout<<"RM_REFERENCE_TRUNCATED file="<<file.filename().string()
            <<" remaining="<<(hits.size()-100)<<"\n";
  for(size_t i=1;i<hits.size();++i){
   const size_t distance=hits[i].first-hits[i-1].first;
   if(distance<=256)
    std::cout<<"RM_REFERENCE_NEAR file="<<file.filename().string()
             <<" first="<<referenceTerms[hits[i-1].second]
             <<" second="<<referenceTerms[hits[i].second]
             <<" distance="<<distance<<"\n";
  }
 }

 // Milestone 97: printable-string neighborhoods around known RM references.
 // These are bounded byte-range observations, not decoded code or call relationships.
 const auto overlayPath=looseRoot/"STTNG.OVL";
 std::ifstream overlay(overlayPath,std::ios::binary);
 if(overlay){
  std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(overlay)),
                                    std::istreambuf_iterator<char>());
  const std::vector<std::pair<const char*,size_t>> anchors={
   {"BRIDGE.RM",1407391},{"BRIDGE.RM",1408226},{"VIEWSCR.RM",1408760}
  };
  for(const auto& anchor:anchors){
   if(anchor.second>=bytes.size()){
    std::cout<<"RM_CONTEXT_ANCHOR term="<<anchor.first<<" status=outsideFile\n";
    continue;
   }
   const size_t begin=anchor.second>1024?anchor.second-1024:0;
   const size_t end=std::min(bytes.size(),anchor.second+1024);
   std::cout<<"RM_CONTEXT_ANCHOR term="<<anchor.first
            <<" offset="<<anchor.second<<" range="<<begin<<"-"<<end
            <<" interpretation=printableStringsOnly\n";
   size_t printed=0;
   for(size_t i=begin;i<end;){
    if(bytes[i]<32||bytes[i]>126){++i;continue;}
    const size_t start=i;
    while(i<end&&bytes[i]>=32&&bytes[i]<=126)++i;
    if(i-start<5)continue;
    std::string value;
    for(size_t j=start;j<i;++j)value.push_back(static_cast<char>(bytes[j]));
    if(printed<90){
     std::cout<<"RM_CONTEXT_STRING anchor="<<anchor.second
              <<" offset="<<start<<" length="<<(i-start)
              <<" text="<<value<<"\n";
    }
    ++printed;
   }
   std::cout<<"RM_CONTEXT_SUMMARY anchor="<<anchor.second
            <<" strings="<<printed<<" displayed="<<std::min<size_t>(printed,90)<<"\n";
  }
 }else std::cout<<"RM_CONTEXT_ANCHOR status=overlayUnavailable\n";
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

 // Milestone 98: independent third-RM comparison, without changing existing
 // two-file differential diagnostics or assuming the third resource is identical.
 {
  std::vector<uint8_t> holodeck;
  std::string readError;
  if(!resources.read("HOLODECK.RM",holodeck,readError,2*1024*1024)){
   std::cout<<"RM_THIRD_RESOURCE name=HOLODECK.RM available=0 reason="<<readError<<"\n";
  }else{
   const auto coverage=reluva::stafu::rmPaletteCoverage(holodeck.data(),holodeck.size());
   const auto first=reluva::stafu::rmLeadingPalette(
     holodeck.data(),holodeck.size(),standard.data(),standard.size());
   const auto tail=reluva::stafu::rmTrailingPalette(
     holodeck.data(),holodeck.size(),standard.data(),standard.size());
   const auto bits=reluva::stafu::rmHighBitAnalysis(holodeck.data(),holodeck.size());
   const auto at384=reluva::stafu::rmBoundary(holodeck.data(),holodeck.size(),384);
   std::cout<<"RM_THIRD_RESOURCE name=HOLODECK.RM available=1 bytes="
            <<holodeck.size()<<" expected307584="<<(holodeck.size()==307584)
            <<" rasterPixels="<<coverage.pixels<<"\n";
   std::cout<<"RM_THIRD_HEADER present="<<first.present
            <<" plausibleVga6="<<first.plausibleVga6
            <<" channelsWithinVga6="<<first.channelsWithinVga6
            <<" uniqueColors="<<first.uniqueColors
            <<" standardEntryMatches="<<first.standardEntryMatches<<"\n";
   std::cout<<"RM_THIRD_RASTER offset384Valid="<<at384.valid
            <<" lowPixels="<<bits.lowPixels<<" highPixels="<<bits.highPixels
            <<" rowsMixed="<<bits.rowsMixed
            <<" rowsLowOnly="<<bits.rowsLowOnly
            <<" rowsHighOnly="<<bits.rowsHighOnly<<"\n";
   std::cout<<"RM_THIRD_TRAILER bytes="<<tail.tailBytes
            <<" candidate384="<<tail.present<<"\n";
   for(const char* other:{"BRIDGE.RM","VIEWSCR.RM"}){
    std::vector<uint8_t> data;
    std::string otherError;
    if(!resources.read(other,data,otherError,2*1024*1024)){
     std::cout<<"RM_THIRD_COMPARE other="<<other<<" unavailable="<<otherError<<"\n";
     continue;
    }
    const size_t entries=std::min<size_t>(128,std::min(holodeck.size(),data.size())/3);
    size_t matching=0;
    for(size_t i=0;i<entries;++i)
     if(std::equal(holodeck.begin()+3*i,holodeck.begin()+3*i+3,data.begin()+3*i))
      ++matching;
    std::cout<<"RM_THIRD_COMPARE other="<<other
             <<" sameIndexHeaderRGB="<<matching<<" compared="<<entries<<"\n";
   }
  }
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
  const struct {const char* label;size_t first,last;} regions[]={
   {"upper",0,363},{"transition",360,370},{"lower",364,479}
  };
  for(const auto& region:regions){
   const auto s=reluva::stafu::rmSharedRaster(a.data(),a.size(),b.data(),b.size(),region.first,region.last);
   if(!s.valid){std::cerr<<"invalid RM comparison region\n";return 1;}
   std::cout<<"RM_SHARED_REGION "<<region.label<<" firstY="<<region.first
            <<" lastY="<<region.last<<" pixels="<<s.pixels
            <<" exact="<<s.exact<<" maskedEqual="<<s.maskedEqual
            <<" identicalRows="<<s.identicalRows
            <<" longestRun="<<s.longestRun<<" x="<<s.longestX<<" y="<<s.longestY<<"\n";
   std::vector<size_t> ranked;
   for(size_t y=region.first;y<=region.last;++y)ranked.push_back(y);
   std::sort(ranked.begin(),ranked.end(),[&](size_t x,size_t y){
    return s.rowMatches[x]!=s.rowMatches[y]?s.rowMatches[x]>s.rowMatches[y]:x<y;
   });
   for(size_t k=0;k<std::min<size_t>(5,ranked.size());++k)
    std::cout<<"RM_SHARED_ROW "<<region.label<<" y="<<ranked[k]
             <<" exact="<<s.rowMatches[ranked[k]]<<" of=640\n";
  }
  for(const auto& region:regions){
   const auto d=reluva::stafu::rmDifferenceRegion(a.data(),a.size(),b.data(),b.size(),region.first,region.last);
   if(!d.valid){std::cerr<<"invalid RM difference region\n";return 1;}
   std::cout<<"RM_DIFF_REGION "<<region.label<<" exact="<<d.exact
            <<" flagOnly="<<d.flagOnly<<" other="<<d.other
            <<" longestFlagRun="<<d.longestFlagRun
            <<" x="<<d.longestFlagX<<" y="<<d.longestFlagY<<"\n";
   for(size_t k=0;k<4;++k)
    std::cout<<"RM_DIFF_BAND "<<region.label<<" band="<<k
             <<" exact="<<d.bandExact[k]<<" flagOnly="<<d.bandFlagOnly[k]
             <<" other="<<d.bandOther[k]<<"\n";
  }
  for(const auto& region:regions){
   const auto pairs=reluva::stafu::rmFlagPairs(a.data(),a.size(),b.data(),b.size(),region.first,region.last);
   if(!pairs.valid){std::cerr<<"invalid RM flag pair region\n";return 1;}
   std::cout<<"RM_FLAG_PAIRS "<<region.label
            <<" bridgeLow_viewHigh="<<pairs.lowToHigh
            <<" bridgeHigh_viewLow="<<pairs.highToLow<<"\n";
   std::vector<size_t> indices;
   for(size_t k=0;k<128;++k)
    if(pairs.lowToHighByBase[k]||pairs.highToLowByBase[k])indices.push_back(k);
   std::sort(indices.begin(),indices.end(),[&](size_t x,size_t y){
    const size_t ax=pairs.lowToHighByBase[x]+pairs.highToLowByBase[x];
    const size_t ay=pairs.lowToHighByBase[y]+pairs.highToLowByBase[y];
    return ax!=ay?ax>ay:x<y;
   });
   for(size_t k=0;k<std::min<size_t>(16,indices.size());++k){
    const size_t base=indices[k];
    std::cout<<"RM_FLAG_PAIR "<<region.label<<" base="<<base
             <<" low="<<base<<" high="<<(base+128)
             <<" bridgeLow_viewHigh="<<pairs.lowToHighByBase[base]
             <<" bridgeHigh_viewLow="<<pairs.highToLowByBase[base]<<"\n";
   }
  }
  const auto geom=reluva::stafu::rmZero128Geometry(a.data(),a.size(),b.data(),b.size(),364,479);
  if(!geom.valid){std::cerr<<"invalid RM 0/128 geometry\n";return 1;}
  std::cout<<"RM_ZERO128_GEOMETRY total="<<geom.total<<" rows="<<geom.rows
           <<" bbox="<<geom.minX<<","<<geom.minY<<"-"<<geom.maxX<<","<<geom.maxY
           <<" longestRun="<<geom.longestRun<<" x="<<geom.longestX
           <<" y="<<geom.longestY<<"\n";
  std::vector<size_t> rows;
  for(size_t y=364;y<=479;++y)if(geom.rowCounts[y])rows.push_back(y);
  std::sort(rows.begin(),rows.end(),[&](size_t x,size_t y){
   return geom.rowCounts[x]!=geom.rowCounts[y]?geom.rowCounts[x]>geom.rowCounts[y]:x<y;
  });
  for(size_t k=0;k<std::min<size_t>(12,rows.size());++k){
   const size_t y=rows[k];
   std::cout<<"RM_ZERO128_ROW y="<<y<<" count="<<geom.rowCounts[y]
            <<" firstX="<<geom.rowFirstX[y]<<" lastX="<<geom.rowLastX[y]<<"\n";
  }
  for(const auto& entry:rmFiles){
   const auto& bytes=entry.second;
   if(bytes.size()>=384)
    std::cout<<"RM_ZERO128_PALETTE "<<entry.first
             <<" leadingIndex0Vga6="<<unsigned(bytes[0])<<","
             <<unsigned(bytes[1])<<","<<unsigned(bytes[2])
             <<" leadingIndex128=outside128EntryHeader"
             <<" index128Rgb=unresolved\n";
  }
  const size_t selectedRows[]={385,394,395,396,397,398,399,400,401,402,406,407,408};
  for(const size_t y:selectedRows){
   const auto spans=reluva::stafu::rmZero128Spans(a.data(),a.size(),b.data(),b.size(),y);
   if(!spans.valid){std::cerr<<"invalid RM span row\n";return 1;}
   std::cout<<"RM_ZERO128_SPANS y="<<y<<" pixels="<<spans.pixels
            <<" spans="<<spans.count;
   for(size_t k=0;k<spans.count;++k)
    std::cout<<" "<<spans.starts[k]<<"-"<<spans.ends[k];
   std::cout<<"\n";
  }
  for(size_t i=1;i<sizeof(selectedRows)/sizeof(selectedRows[0]);++i){
   const auto before=reluva::stafu::rmZero128Spans(a.data(),a.size(),b.data(),b.size(),selectedRows[i-1]);
   const auto after=reluva::stafu::rmZero128Spans(a.data(),a.size(),b.data(),b.size(),selectedRows[i]);
   std::cout<<"RM_ZERO128_REPEAT y0="<<selectedRows[i-1]<<" y1="<<selectedRows[i]
            <<" sharedSpans="<<reluva::stafu::rmZero128SharedSpans(before,after)
            <<" count0="<<before.count<<" count1="<<after.count<<"\n";
  }
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

 // Milestone 93: compare candidate palette ownership hypotheses numerically.
 // RGB888 candidate tables and VGA6 RM headers are not assumed to share an index map.
 for(const auto& entry:rmFiles){
  const auto& bytes=entry.second;
  if(bytes.size()<384)continue;
  for(const auto& candidate:candidates){
   if(!candidate.palette.valid)continue;
   const auto& pal=candidate.palette;
   const auto direct=reluva::stafu::rmPaletteOffsets(bytes.data(),384,
     pal.rgb.data(),pal.entries*3);
   std::cout<<"RM_PALETTE_CANDIDATE rm="<<entry.first
            <<" source="<<candidate.name
            <<" entries="<<pal.entries
            <<" direct="<<direct.directMatches
            <<" bestOffset="<<direct.bestOffset
            <<" bestMatches="<<direct.bestMatches
            <<" compared="<<direct.bestCompared
            <<" membership="<<direct.rgbMembership<<"\n";
   for(const size_t index:{size_t(0),size_t(128)}){
    if(index>=pal.entries)continue;
    std::cout<<"RM_PALETTE_INDEX rm="<<entry.first
             <<" source="<<candidate.name<<" index="<<index
             <<" rgb888="<<unsigned(pal.rgb[3*index])<<","
             <<unsigned(pal.rgb[3*index+1])<<","
             <<unsigned(pal.rgb[3*index+2])
             <<" interpretation=candidateOnly\n";
   }
  }
  const auto standardMatch=reluva::stafu::rmPaletteOffsets(bytes.data(),384,
    standard.data(),standard.size());
  std::cout<<"RM_PALETTE_STANDARD rm="<<entry.first
           <<" direct="<<standardMatch.directMatches
           <<" bestOffset="<<standardMatch.bestOffset
           <<" bestMatches="<<standardMatch.bestMatches
           <<" compared="<<standardMatch.bestCompared
           <<" membership="<<standardMatch.rgbMembership
           <<" standardEntries=128\n";
 }
 // Milestone 94: fixed-size sections are observational, not assumed format boundaries.
 for(const auto& entry:rmFiles){
  const auto& bytes=entry.second;
  for(size_t first=0;first<128;first+=16){
   const auto s=reluva::stafu::rmHeaderSection(bytes.data(),bytes.size(),first,16);
   std::cout<<"RM_HEADER_SECTION rm="<<entry.first<<" first="<<first
            <<" last="<<(first+15)<<" unique="<<s.unique
            <<" black="<<s.black<<" adjacentEqual="<<s.adjacentEqual
            <<" minChannel="<<s.minChannel<<" maxChannel="<<s.maxChannel<<"\n";
  }
 }
 if(rmFiles.size()==2){
  for(size_t first=0;first<128;first+=16)
   std::cout<<"RM_HEADER_SECTION_COMPARE first="<<first
            <<" last="<<(first+15)
            <<" equalEntries="<<reluva::stafu::rmHeaderSectionMatches(
              rmFiles[0].second.data(),rmFiles[0].second.size(),
              rmFiles[1].second.data(),rmFiles[1].second.size(),first,16)
            <<" of=16\n";
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
