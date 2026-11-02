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



 // Milestone 107: compare each half of color.img's GIF palette to standard.pal.
 // Two explicit VGA6->RGB888 mappings are checked; matches do not prove ownership.
 {
  const auto it=std::find_if(candidates.begin(),candidates.end(),
    [](const Candidate& candidate){return std::string(candidate.name)=="color.img";});
  if(it==candidates.end()||!it->palette.valid||it->palette.entries<256){
   std::cout<<"RM_PALETTE_BANK_STATUS available=0 reason=colorImgPaletteMissingOrShort\n";
  }else{
   const auto& rgb=it->palette.rgb;
   std::cout<<"RM_PALETTE_BANK_STATUS available=1 gifEntries="<<it->palette.entries
            <<" standardEntries="<<(standard.size()/3)<<"\n";
   for(const char* conversion:{"scale255","shift2"}){
    auto convert=[&](unsigned v)->unsigned{
     return std::string(conversion)=="scale255"?(v*255+31)/63:(v<<2);
    };
    for(unsigned bank=0;bank<2;++bank){
     size_t sameIndex=0,member=0,uniqueMatched=0;
     bool matchedStandard[128]={};
     size_t sameIndexIndices[128]={};
     size_t sameIndexCount=0;
     for(unsigned i=0;i<128;++i){
      const unsigned gifIndex=bank*128+i;
      bool exact=true;
      for(unsigned channel=0;channel<3;++channel)
       if(convert(standard[3*i+channel])!=rgb[3*gifIndex+channel])
        exact=false;
      if(exact){++sameIndex;sameIndexIndices[sameIndexCount++]=i;}
      bool found=false;
      for(unsigned j=0;j<128;++j){
       bool equal=true;
       for(unsigned channel=0;channel<3;++channel)
        if(convert(standard[3*j+channel])!=rgb[3*gifIndex+channel])
         equal=false;
       if(equal){found=true;matchedStandard[j]=true;}
      }
      if(found)++member;
     }
     for(bool found:matchedStandard)if(found)++uniqueMatched;
     std::cout<<"RM_PALETTE_BANK_COMPARE source=color.img conversion="<<conversion
              <<" bank="<<bank<<" gifFirst="<<(bank*128)
              <<" gifLast="<<(bank*128+127)
              <<" sameIndex="<<sameIndex
              <<" gifEntriesMatchingAnyStandard="<<member
              <<" uniqueStandardEntriesMatched="<<uniqueMatched<<"\n";
     std::cout<<"RM_PALETTE_BANK_DIRECT source=color.img conversion="<<conversion
              <<" bank="<<bank<<" indices=";
     if(!sameIndexCount)std::cout<<"none";
     for(size_t k=0;k<sameIndexCount;++k){
      if(k)std::cout<<",";
      std::cout<<sameIndexIndices[k];
     }
     std::cout<<"\n";
    }
   }
  }
 }

 // Milestone 100: survey all five verified RM names directly from the CD archive.
 // This intentionally bypasses installed-file precedence and case-sensitive
 // ResourceManager lookup; results describe archive bytes only.
 {
  reluva::stafu::ZipArchive surveyArchive;
  std::string surveyError;
  const char* names[]={"bridge.rm","viewscr.rm","transp.rm","holodeck.rm","endgame.rm"};
  if(!surveyArchive.open(std::filesystem::path(argv[2])/"STTNG.ZIP",surveyError)){
   std::cout<<"RM_ALL_STATUS archive=unavailable reason="<<surveyError<<"\n";
  }else{
   struct Survey {std::string name;std::vector<uint8_t> bytes;};
   std::vector<Survey> surveys;
   for(const char* name:names){
    std::vector<uint8_t> data;
    std::string readError;
    if(!surveyArchive.read(name,data,readError,2*1024*1024)){
     std::cout<<"RM_ALL_RESOURCE name="<<name<<" read=0 reason="<<readError<<"\n";
     continue;
    }
    const auto head=reluva::stafu::rmLeadingPalette(
      data.data(),data.size(),standard.data(),standard.size());
    const auto raster=reluva::stafu::rmPaletteCoverage(data.data(),data.size());
    const auto bits=reluva::stafu::rmHighBitAnalysis(data.data(),data.size());
    const auto boundary=reluva::stafu::rmBoundary(data.data(),data.size(),384);
    const bool layout=data.size()==307584&&head.present&&raster.pixels==307200;
    std::cout<<"RM_ALL_RESOURCE name="<<name<<" read=1 bytes="<<data.size()
             <<" expectedLayout="<<layout
             <<" headerPresent="<<head.present
             <<" headerVga6="<<head.plausibleVga6
             <<" headerChannelsWithinVga6="<<head.channelsWithinVga6
             <<" uniqueHeaderColors="<<head.uniqueColors
             <<" standardSameIndex="<<head.standardEntryMatches
             <<" rasterPixels="<<raster.pixels
             <<" offset384Valid="<<boundary.valid
             <<" lowPixels="<<bits.lowPixels
             <<" highPixels="<<bits.highPixels
             <<" rowsMixed="<<bits.rowsMixed
             <<" rowsLowOnly="<<bits.rowsLowOnly
             <<" rowsHighOnly="<<bits.rowsHighOnly<<"\n";
    surveys.push_back({name,std::move(data)});
   }
   for(size_t i=0;i<surveys.size();++i)
    for(size_t j=i+1;j<surveys.size();++j){
     const auto& a=surveys[i];const auto& b=surveys[j];
     const size_t entries=std::min<size_t>(128,std::min(a.bytes.size(),b.bytes.size())/3);
     size_t equal=0;
     for(size_t k=0;k<entries;++k)
      if(std::equal(a.bytes.begin()+3*k,a.bytes.begin()+3*k+3,b.bytes.begin()+3*k))
       ++equal;
     std::cout<<"RM_ALL_PAIR first="<<a.name<<" second="<<b.name
              <<" sameIndexHeaderRGB="<<equal<<" compared="<<entries<<"\n";
    }
   std::cout<<"RM_ALL_STATUS archive=available requested=5 read="
            <<surveys.size()<<"\n";
  }
 }



 // Milestone 108: compare each RM's 128-entry VGA6 header against both
 // color.img GIF palette halves under explicit RGB888 conversions.
 // These are color comparisons only, not evidence of palette ownership.
 {
  const auto gif=std::find_if(candidates.begin(),candidates.end(),
    [](const Candidate& v){return std::string(v.name)=="color.img";});
  reluva::stafu::ZipArchive zip108;
  std::string error108;
  if(gif==candidates.end()||!gif->palette.valid||gif->palette.entries<256){
   std::cout<<"RM_GIF_BANK_STATUS available=0 reason=colorImgPaletteMissingOrShort\n";
  }else if(!zip108.open(std::filesystem::path(argv[2])/"STTNG.ZIP",error108)){
   std::cout<<"RM_GIF_BANK_STATUS available=0 reason=archiveUnavailable\n";
  }else{
   std::cout<<"RM_GIF_BANK_STATUS available=1 gifEntries="<<gif->palette.entries<<"\n";
   for(const char* name108:{"bridge.rm","viewscr.rm","transp.rm","holodeck.rm","endgame.rm"}){
    std::vector<uint8_t> raw108;
    std::string readError108;
    if(!zip108.read(name108,raw108,readError108,2*1024*1024)||raw108.size()!=307584){
     std::cout<<"RM_GIF_BANK_RESOURCE name="<<name108<<" read=0\n";
     continue;
    }
    std::cout<<"RM_GIF_BANK_RESOURCE name="<<name108<<" read=1 headerEntries=128\n";
    for(const char* method108:{"scale255","shift2"}){
     const bool scale108=std::string(method108)=="scale255";
     auto convert108=[&](unsigned x)->unsigned{
      return scale108?(x*255+31)/63:(x<<2);
     };
     for(unsigned bank108=0;bank108<2;++bank108){
      size_t direct108=0,rmMatching108=0,gifMatching108=0;
      bool rmMatched108[128]={};
      bool gifMatched108[128]={};
      for(size_t i=0;i<128;++i){
       bool same108=true;
       for(size_t channel=0;channel<3;++channel)
        if(convert108(raw108[3*i+channel])!=gif->palette.rgb[3*(bank108*128+i)+channel])
         same108=false;
       if(same108)++direct108;
       for(size_t j=0;j<128;++j){
        bool equal108=true;
        for(size_t channel=0;channel<3;++channel)
         if(convert108(raw108[3*i+channel])!=gif->palette.rgb[3*(bank108*128+j)+channel])
          equal108=false;
        if(equal108){rmMatched108[i]=true;gifMatched108[j]=true;}
       }
      }
      for(bool yes:rmMatched108)if(yes)++rmMatching108;
      for(bool yes:gifMatched108)if(yes)++gifMatching108;
      std::cout<<"RM_GIF_BANK_COMPARE name="<<name108
               <<" conversion="<<method108<<" bank="<<bank108
               <<" gifFirst="<<(bank108*128)<<" gifLast="<<(bank108*128+127)
               <<" sameIndex="<<direct108
               <<" rmEntriesMatchingAnyGif="<<rmMatching108
               <<" gifEntriesMatchingAnyRm="<<gifMatching108
               <<" compared=128\n";
     }
    }
   }
  }
 }

 // Milestone 101: high-index frequency and spatial survey for every archive RM.
 // Interpret bit 7 numerically only; no palette-bank or transparency assumption.
 {
  reluva::stafu::ZipArchive zip;
  std::string zipError;
  if(!zip.open(std::filesystem::path(argv[2])/"STTNG.ZIP",zipError)){
   std::cout<<"RM_UPPER_STATUS archive=unavailable reason="<<zipError<<"\n";
  }else{
   for(const char* name:{"bridge.rm","viewscr.rm","transp.rm","holodeck.rm","endgame.rm"}){
    std::vector<uint8_t> data;
    std::string readError;
    if(!zip.read(name,data,readError,2*1024*1024)){
     std::cout<<"RM_UPPER_RESOURCE name="<<name<<" read=0 reason="<<readError<<"\n";
     continue;
    }
    if(data.size()!=384+640*480){
     std::cout<<"RM_UPPER_RESOURCE name="<<name<<" read=1 layout=unexpected bytes="
              <<data.size()<<"\n";
     continue;
    }
    size_t counts[256]={};
    size_t bands[4][2]={};
    size_t quarters[4][4]={};
    for(size_t y=0;y<480;++y){
     for(size_t x=0;x<640;++x){
      const unsigned value=data[384+y*640+x];
      ++counts[value];
      ++bands[y/120][value>=128?1:0];
      if(value>=128)++quarters[y/120][x/160];
     }
    }
    size_t distinctHigh=0,paired=0,highTotal=0;
    std::vector<unsigned> ranked;
    for(unsigned i=128;i<256;++i){
     if(!counts[i])continue;
     ++distinctHigh;
     highTotal+=counts[i];
     if(counts[i-128])++paired;
     ranked.push_back(i);
    }
    std::sort(ranked.begin(),ranked.end(),[&](unsigned a,unsigned b){
     return counts[a]!=counts[b]?counts[a]>counts[b]:a<b;
    });
    std::cout<<"RM_UPPER_RESOURCE name="<<name<<" read=1 distinctHigh="
             <<distinctHigh<<" highPixels="<<highTotal
             <<" pairedWithUsedLow="<<paired<<" distinctLow=";
    size_t distinctLow=0;
    for(unsigned i=0;i<128;++i)if(counts[i])++distinctLow;
    std::cout<<distinctLow<<"\n";
    for(size_t i=0;i<std::min<size_t>(16,ranked.size());++i){
     const unsigned v=ranked[i];
     std::cout<<"RM_UPPER_TOP name="<<name<<" index="<<v
              <<" pixels="<<counts[v]<<" lowPartner="<<(v-128)
              <<" lowPartnerPixels="<<counts[v-128]<<"\n";
    }
    for(size_t b=0;b<4;++b){
     std::cout<<"RM_UPPER_BAND name="<<name<<" y0="<<(b*120)
              <<" y1="<<(b*120+119)<<" low="<<bands[b][0]
              <<" high="<<bands[b][1]<<"\n";
     for(size_t q=0;q<4;++q)
      std::cout<<"RM_UPPER_GRID name="<<name<<" col="<<q<<" row="<<b
               <<" high="<<quarters[b][q]<<" pixels="<<(160*120)<<"\n";
    }
   }
  }
 }


 // Milestone 102: inspect the exact upper-index boundary in holodeck.rm.
 // These are raw index values, not inferred palette-bank or transparency semantics.
 {
  reluva::stafu::ZipArchive zip;
  std::string zipError;
  if(!zip.open(std::filesystem::path(argv[2])/"STTNG.ZIP",zipError)){
   std::cout<<"RM_HOLO_BOUNDARY_STATUS read=0 reason="<<zipError<<"\n";
  }else{
   std::vector<uint8_t> data;
   std::string readError;
   if(!zip.read("holodeck.rm",data,readError,2*1024*1024)){
    std::cout<<"RM_HOLO_BOUNDARY_STATUS read=0 reason="<<readError<<"\n";
   }else if(data.size()!=384+640*480){
    std::cout<<"RM_HOLO_BOUNDARY_STATUS read=1 layout=unexpected bytes="<<data.size()<<"\n";
   }else{
    int firstRow=-1,lastRow=-1,firstX=-1,lastX=-1;
    size_t highTotal=0;
    size_t indexCounts[128]={};
    for(int y=0;y<480;++y){
     size_t rowCounts[128]={};
     int left=-1,right=-1;
     size_t spans=0,longest=0;
     int spanStart=-1;
     for(int x=0;x<=640;++x){
      const bool high=x<640 && data[384+size_t(y)*640+x]>=128;
      if(high){
       const unsigned index=data[384+size_t(y)*640+x]-128;
       ++rowCounts[index];
       ++indexCounts[index];
       ++highTotal;
       if(left<0)left=x;
       right=x;
       if(spanStart<0)spanStart=x;
       if(firstRow<0){firstRow=y;firstX=x;}
       lastRow=y;lastX=x;
      }else if(spanStart>=0){
       ++spans;
       longest=std::max(longest,size_t(x-spanStart));
       spanStart=-1;
      }
     }
     if(left<0)continue;
     size_t count=0,distinct=0;
     std::ostringstream indices;
     for(unsigned i=0;i<128;++i)if(rowCounts[i]){
      count+=rowCounts[i];++distinct;
      if(indices.tellp()>0)indices<<",";
      indices<<(i+128)<<":"<<rowCounts[i];
     }
     std::cout<<"RM_HOLO_ROW y="<<y<<" high="<<count
              <<" firstX="<<left<<" lastX="<<right
              <<" spans="<<spans<<" longestSpan="<<longest
              <<" distinct="<<distinct<<" indices="<<indices.str()<<"\n";
    }
    std::cout<<"RM_HOLO_BOUNDARY_STATUS read=1 layout=expected"
             <<" firstRow="<<firstRow<<" lastRow="<<lastRow
             <<" firstX="<<firstX<<" lastX="<<lastX
             <<" highPixels="<<highTotal<<"\n";
    for(unsigned i=0;i<128;++i)if(indexCounts[i])
     std::cout<<"RM_HOLO_INDEX index="<<(i+128)
              <<" pixels="<<indexCounts[i]<<"\n";
   }
  }
 }


 // Milestone 103: raw index neighborhoods at the holodeck boundary.
 // The numeric +/-128 comparison is observational, not a palette-bank claim.
 {
  reluva::stafu::ZipArchive zip;
  std::string error103;
  if(!zip.open(std::filesystem::path(argv[2])/"STTNG.ZIP",error103)){
   std::cout<<"RM_HOLO_EDGE_STATUS read=0 reason="<<error103<<"\n";
  }else{
   std::vector<uint8_t> bytes;
   if(!zip.read("holodeck.rm",bytes,error103,2*1024*1024)){
    std::cout<<"RM_HOLO_EDGE_STATUS read=0 reason="<<error103<<"\n";
   }else if(bytes.size()!=384+640*480){
    std::cout<<"RM_HOLO_EDGE_STATUS read=1 layout=unexpected bytes="<<bytes.size()<<"\n";
   }else{
    auto pixel=[&](int x,int y)->unsigned{return bytes[384+size_t(y)*640+size_t(x)];};
    const int rows[]={363,364,365,393,394,395,396,397,479};
    const struct {int left,right;} windows[]={{0,15},{368,400},{624,639}};
    for(int y:rows){
     for(const auto& window:windows){
      std::ostringstream raw;
      size_t low=0,high=0;
      for(int x=window.left;x<=window.right;++x){
       if(x!=window.left)raw<<",";
       const unsigned v=pixel(x,y);
       raw<<v;
       if(v>=128)++high;else ++low;
      }
      std::cout<<"RM_HOLO_EDGE_ROW y="<<y<<" x0="<<window.left
               <<" x1="<<window.right<<" low="<<low<<" high="<<high
               <<" values="<<raw.str()<<"\n";
     }
    }
    // Compare adjacent pixels across the horizontal transition (y=394/395)
    // and vertical boundary (x=383/384) using equal or bit7-only differences.
    for(int x:{0,1,15,368,383,384,385,400,624,639}){
     const unsigned a=pixel(x,394),b=pixel(x,395);
     std::cout<<"RM_HOLO_EDGE_VERTICAL x="<<x<<" above="<<a<<" below="<<b
              <<" equal="<<(a==b)<<" sameLow7="<<((a&127)==(b&127))<<"\n";
    }
    for(int y:{393,394,395,396,397,427,479}){
     const unsigned a=pixel(383,y),b=pixel(384,y);
     std::cout<<"RM_HOLO_EDGE_HORIZONTAL y="<<y<<" left="<<a<<" right="<<b
              <<" equal="<<(a==b)<<" sameLow7="<<((a&127)==(b&127))<<"\n";
    }
    for(int y:{363,364,365,393,394,395,396,397,479}){
     const unsigned a=pixel(0,y),b=pixel(1,y);
     std::cout<<"RM_HOLO_EDGE_COLUMN y="<<y<<" x0="<<a<<" x1="<<b
              <<" equal="<<(a==b)<<" sameLow7="<<((a&127)==(b&127))<<"\n";
    }
    std::cout<<"RM_HOLO_EDGE_STATUS read=1 layout=expected\n";
   }
  }
 }


 // Milestone 104: compare adjacent pixels differing only by bit 7 in all five RMs.
 // Count horizontal and vertical transitions separately and preserve exact pairs.
 {
  reluva::stafu::ZipArchive zip;
  std::string err104;
  if(!zip.open(std::filesystem::path(argv[2])/"STTNG.ZIP",err104)){
   std::cout<<"RM_BIT7_EDGE_STATUS read=0 reason="<<err104<<"\n";
  }else{
   size_t readCount=0;
   for(const char* name:{"bridge.rm","viewscr.rm","transp.rm","holodeck.rm","endgame.rm"}){
    std::vector<uint8_t> bytes;
    std::string readError;
    if(!zip.read(name,bytes,readError,2*1024*1024)){
     std::cout<<"RM_BIT7_EDGE_RESOURCE name="<<name<<" read=0 reason="<<readError<<"\n";
     continue;
    }
    if(bytes.size()!=384+640*480){
     std::cout<<"RM_BIT7_EDGE_RESOURCE name="<<name<<" read=1 layout=unexpected bytes="
              <<bytes.size()<<"\n";
     continue;
    }
    ++readCount;
    auto pixel=[&](int x,int y)->unsigned{
     return bytes[384+size_t(y)*640+size_t(x)];
    };
    size_t horizontal=0,vertical=0;
    size_t hRow[480]={},vRow[480]={};
    size_t hPair[128]={},vPair[128]={};
    for(int y=0;y<480;++y){
     for(int x=0;x<640;++x){
      const unsigned a=pixel(x,y);
      if(x+1<640){
       const unsigned b=pixel(x+1,y);
       if((a^b)==128){
        ++horizontal;++hRow[y];++hPair[a&127];
       }
      }
      if(y+1<480){
       const unsigned b=pixel(x,y+1);
       if((a^b)==128){
        ++vertical;++vRow[y];++vPair[a&127];
       }
      }
     }
    }
    std::cout<<"RM_BIT7_EDGE_RESOURCE name="<<name<<" read=1 horizontal="
             <<horizontal<<" vertical="<<vertical
             <<" horizontalComparisons="<<(639*480)
             <<" verticalComparisons="<<(640*479)<<"\n";
    auto reportRows=[&](const char* axis,const size_t* rows){
     std::vector<int> ranked;
     for(int y=0;y<480;++y)if(rows[y])ranked.push_back(y);
     std::sort(ranked.begin(),ranked.end(),[&](int a,int b){
      return rows[a]!=rows[b]?rows[a]>rows[b]:a<b;
     });
     std::cout<<"RM_BIT7_EDGE_ROWS name="<<name<<" axis="<<axis
              <<" affected="<<ranked.size()<<"\n";
     for(size_t i=0;i<std::min<size_t>(12,ranked.size());++i)
      std::cout<<"RM_BIT7_EDGE_ROW name="<<name<<" axis="<<axis
               <<" y="<<ranked[i]<<" transitions="<<rows[ranked[i]]<<"\n";
    };
    reportRows("horizontal",hRow);
    reportRows("vertical",vRow);
    for(unsigned low=0;low<128;++low){
     if(hPair[low]||vPair[low])
      std::cout<<"RM_BIT7_EDGE_PAIR name="<<name<<" low="<<low
               <<" high="<<(low+128)<<" horizontal="<<hPair[low]
               <<" vertical="<<vPair[low]<<"\n";
    }
   }
   std::cout<<"RM_BIT7_EDGE_STATUS read="<<readCount<<" requested=5\n";
  }
 }


 // Milestone 105: bounded, read-only observations near STANDARD.PAL in STTNG.OVL.
 // Printable strings and VGA6-like byte runs do not establish executable semantics.
 {
  std::ifstream input(looseRoot/"STTNG.OVL",std::ios::binary);
  if(!input){
   std::cout<<"RM_OVL_PALETTE_STATUS read=0 reason=missingOverlay\n";
  }else{
   std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(input)),
                                     std::istreambuf_iterator<char>());
   const std::string needle105="STANDARD.PAL";
   std::vector<size_t> offsets;
   for(size_t i=0;i+needle105.size()<=bytes.size();++i){
    bool match=true;
    for(size_t j=0;j<needle105.size();++j)
     if(std::toupper(bytes[i+j])!=needle105[j]){match=false;break;}
    if(match)offsets.push_back(i);
   }
   std::cout<<"RM_OVL_PALETTE_STATUS read=1 bytes="<<bytes.size()
            <<" references="<<offsets.size()<<"\n";
   for(size_t offset:offsets){
    const size_t start=offset>512?offset-512:0;
    const size_t end=std::min(bytes.size(),offset+needle105.size()+512);
    std::cout<<"RM_OVL_PALETTE_ANCHOR offset="<<offset
             <<" rangeStart="<<start<<" rangeEnd="<<end<<"\n";
    // Report nearby printable runs, retaining their absolute file offsets.
    size_t reported=0;
    for(size_t i=start;i<end;){
     if(bytes[i]<32||bytes[i]>126){++i;continue;}
     const size_t runStart=i;
     while(i<end&&bytes[i]>=32&&bytes[i]<=126)++i;
     if(i-runStart<5)continue;
     if(reported++>=20)break;
     std::string run(bytes.begin()+runStart,bytes.begin()+std::min(i,runStart+96));
     std::cout<<"RM_OVL_PALETTE_STRING anchor="<<offset
              <<" offset="<<runStart<<" length="<<(i-runStart)
              <<" text="<<run<<"\n";
    }
    // Find 384-byte windows whose every byte is within VGA6 range [0,63].
    // This is only a format candidate and may occur in code, padding, or other data.
    size_t candidateCount=0;
    size_t firstCandidate=bytes.size();
    if(end>=start+384){
     size_t bad=0;
     for(size_t i=start;i<start+384;++i)if(bytes[i]>63)++bad;
     for(size_t i=start;i+384<=end;++i){
      if(bad==0){++candidateCount;if(firstCandidate==bytes.size())firstCandidate=i;}
      if(i+384<end){
       if(bytes[i]>63)--bad;
       if(bytes[i+384]>63)++bad;
      }
     }
    }
    std::cout<<"RM_OVL_PALETTE_VGA6_CANDIDATE anchor="<<offset
             <<" windows384="<<candidateCount
             <<" firstOffset="<<(firstCandidate==bytes.size()?-1LL:static_cast<long long>(firstCandidate))
             <<" scope=plusMinus512\n";
   }
  }
 }


 // Milestone 106: raw format inventory for the two known palette-related assets.
 // Byte layouts are observations; they do not establish RM palette ownership.
 for(const char* name106:{"standard.pal","color.img"}){
  std::vector<uint8_t> raw106;
  std::string err106;
  if(!resources.read(name106,raw106,err106,1024*1024)){
   std::cout<<"RM_PALETTE_FORMAT name="<<name106<<" read=0 reason="<<err106<<"\n";
   continue;
  }
  size_t under64=0,zero=0;
  bool used106[256]={};
  for(uint8_t b:raw106){
   if(b<64)++under64;
   if(b==0)++zero;
   used106[b]=true;
  }
  size_t unique106=0;
  for(bool present:used106)if(present)++unique106;
  std::cout<<"RM_PALETTE_FORMAT name="<<name106
           <<" read=1 bytes="<<raw106.size()
           <<" divisibleBy3="<<(raw106.size()%3==0)
           <<" rgbTriplets="<<(raw106.size()/3)
           <<" divisibleBy384="<<(raw106.size()%384==0)
           <<" full384Blocks="<<(raw106.size()/384)
           <<" bytesUnder64="<<under64<<" zeroBytes="<<zero
           <<" uniqueByteValues="<<unique106<<"\n";
  std::cout<<"RM_PALETTE_FORMAT_PREFIX name="<<name106<<" hex=";
  for(size_t i=0;i<std::min<size_t>(32,raw106.size());++i){
   if(i)std::cout<<",";
   std::cout<<std::hex<<std::setw(2)<<std::setfill('0')<<unsigned(raw106[i]);
  }
  std::cout<<std::dec<<std::setfill(' ')<<"\n";
  if(raw106.size()>=13&&raw106[0]=='G'&&raw106[1]=='I'&&raw106[2]=='F'
      &&raw106[3]=='8'&&(raw106[4]=='7'||raw106[4]=='9')&&raw106[5]=='a'){
   const unsigned width=unsigned(raw106[6])|(unsigned(raw106[7])<<8);
   const unsigned height=unsigned(raw106[8])|(unsigned(raw106[9])<<8);
   const unsigned packed=raw106[10];
   const bool globalTable=(packed&0x80)!=0;
   const size_t tableEntries=globalTable?(size_t(1)<<(1+(packed&7))):0;
   const size_t tableEnd=13+3*tableEntries;
   std::cout<<"RM_PALETTE_FORMAT_GIF name="<<name106
            <<" width="<<width<<" height="<<height
            <<" globalColorTable="<<globalTable
            <<" globalEntries="<<tableEntries
            <<" globalTableEnd="<<tableEnd
            <<" tableFits="<<(tableEnd<=raw106.size())
            <<" trailingBytes="<<(tableEnd<=raw106.size()?raw106.size()-tableEnd:0)
            <<"\n";
  }else{
   std::cout<<"RM_PALETTE_FORMAT_GIF name="<<name106<<" signature=absent\n";
  }
  if(raw106.size()%384==0&&raw106.size()<=4096){
   for(size_t offset=0;offset<raw106.size();offset+=384){
    size_t vga6=0;
    for(size_t i=offset;i<offset+384;++i)if(raw106[i]<=63)++vga6;
    std::cout<<"RM_PALETTE_FORMAT_BLOCK name="<<name106
             <<" offset="<<offset<<" bytes=384 vga6RangeBytes="<<vga6
             <<" of=384\n";
   }
  }
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

 // Milestone 109: bounded byte-pattern survey, not disassembly.
 // Patterns may occur in code, embedded data, immediates, or compressed material.
 // DOS port writes: MOV DX,03C8/03C9 (BA imm16), OUT DX,AL (EE),
 // OUT DX,AX (EF), and OUT DX,AL/AX after immediate load.
 // BIOS INT 10h bytes (CD 10) and common AX palette function loads
 // (B8 10 10 / B8 12 10) are independently counted; no control-flow claim.
 {
  const struct Pattern109 {const char* label;std::vector<uint8_t> bytes;} patterns109[]={
   {"mov_dx_03c8",{0xBA,0xC8,0x03}},
   {"mov_dx_03c9",{0xBA,0xC9,0x03}},
   {"out_dx_al",{0xEE}},
   {"out_dx_ax",{0xEF}},
   {"int_10",{0xCD,0x10}},
   {"mov_ax_1010",{0xB8,0x10,0x10}},
   {"mov_ax_1012",{0xB8,0x12,0x10}},
   {"mov_ah_10",{0xB4,0x10}},
   {"mov_al_10",{0xB0,0x10}},
   {"mov_al_12",{0xB0,0x12}}
  };
  for(const char* file109:{"STTNG.EXE","STTNG.OVL"}){
   const auto path109=std::filesystem::path(argv[1])/file109;
   std::ifstream input109(path109,std::ios::binary);
   if(!input109){
    std::cout<<"RM_VGA_SCAN_STATUS file="<<file109<<" read=0\n";
    continue;
   }
   const std::vector<uint8_t> data109(
    (std::istreambuf_iterator<char>(input109)),std::istreambuf_iterator<char>());
   std::cout<<"RM_VGA_SCAN_STATUS file="<<file109
            <<" read=1 bytes="<<data109.size()
            <<" scope=rawBytePatternOnly\n";
   for(const auto& pattern109:patterns109){
    size_t count109=0;
    std::vector<size_t> first109;
    if(data109.size()>=pattern109.bytes.size())
     for(size_t pos109=0;pos109<=data109.size()-pattern109.bytes.size();++pos109)
      if(std::equal(pattern109.bytes.begin(),pattern109.bytes.end(),
                    data109.begin()+pos109)){
       ++count109;
       if(first109.size()<12)first109.push_back(pos109);
      }
    std::cout<<"RM_VGA_SCAN_PATTERN file="<<file109
             <<" pattern="<<pattern109.label
             <<" matches="<<count109<<" firstOffsets=";
    if(first109.empty())std::cout<<"none";
    for(size_t k109=0;k109<first109.size();++k109){
     if(k109)std::cout<<",";
     std::cout<<first109[k109];
    }
    std::cout<<"\n";
   }
   // Report nearby pairs of MOV DX, DAC port and OUT DX instruction bytes.
   // This is a proximity heuristic, not instruction decoding or a call graph.
   size_t pairs109=0;
   for(size_t i109=0;i109+2<data109.size();++i109){
    if(data109[i109]!=0xBA||data109[i109+2]!=0x03||
       (data109[i109+1]!=0xC8&&data109[i109+1]!=0xC9))continue;
    const size_t end109=std::min(data109.size(),i109+35);
    for(size_t j109=i109+3;j109<end109;++j109)
     if(data109[j109]==0xEE||data109[j109]==0xEF){
      ++pairs109;
      if(pairs109<=20)
       std::cout<<"RM_VGA_SCAN_NEAR file="<<file109
                <<" movDxOffset="<<i109
                <<" port=0x3"<<(data109[i109+1]==0xC8?"c8":"c9")
                <<" outOffset="<<j109
                <<" distance="<<(j109-i109)
                <<"\n";
      break;
     }
   }
   std::cout<<"RM_VGA_SCAN_NEAR_SUMMARY file="<<file109
            <<" candidatePairs="<<pairs109<<" displayed="<<std::min<size_t>(pairs109,20)
            <<" maxDistance=34\n";
  }
 }


 // Milestone 110: bounded, offset-addressed hex windows around the strongest
 // Milestone 109 VGA DAC candidates. No instruction-boundary assumptions.
 {
  const auto overlay110=std::filesystem::path(argv[1])/"STTNG.OVL";
  std::ifstream stream110(overlay110,std::ios::binary);
  if(!stream110){
   std::cout<<"RM_VGA_WINDOW_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> bytes110(
    (std::istreambuf_iterator<char>(stream110)),std::istreambuf_iterator<char>());
   const size_t anchors110[]={637886,638028,1211956,1212507,1358117};
   std::cout<<"RM_VGA_WINDOW_STATUS read=1 bytes="<<bytes110.size()
            <<" anchors=5 radius=32\n";
   for(size_t anchor110:anchors110){
    if(anchor110>=bytes110.size()){
     std::cout<<"RM_VGA_WINDOW_ANCHOR offset="<<anchor110<<" available=0\n";
     continue;
    }
    const size_t start110=anchor110>32?anchor110-32:0;
    const size_t end110=std::min(bytes110.size(),anchor110+33);
    std::cout<<"RM_VGA_WINDOW_ANCHOR offset="<<anchor110
             <<" available=1 start="<<start110<<" endExclusive="<<end110<<"\n";
    for(size_t row110=start110;row110<end110;row110+=16){
     std::cout<<"RM_VGA_WINDOW_HEX anchor="<<anchor110
              <<" offset="<<row110<<" hex=";
     for(size_t i110=row110;i110<std::min(end110,row110+16);++i110){
      if(i110!=row110)std::cout<<" ";
      std::cout<<std::hex<<std::setw(2)<<std::setfill('0')<<unsigned(bytes110[i110]);
     }
     std::cout<<std::dec<<std::setfill(' ')<<"\n";
    }
    // Match only contiguous four-byte sequences, without claiming they are code.
    size_t exact110=0;
    for(size_t pos110=start110;pos110+3<end110;++pos110){
     if(bytes110[pos110]!=0xBA||bytes110[pos110+2]!=0x03||
        (bytes110[pos110+1]!=0xC8&&bytes110[pos110+1]!=0xC9)||
        (bytes110[pos110+3]!=0xEE&&bytes110[pos110+3]!=0xEF))
      continue;
     ++exact110;
     std::cout<<"RM_VGA_WINDOW_SEQUENCE anchor="<<anchor110
              <<" offset="<<pos110
              <<" port="<<(bytes110[pos110+1]==0xC8?"0x3C8":"0x3C9")
              <<" outByte="<<(bytes110[pos110+3]==0xEE?"EE":"EF")
              <<" interpretation=bytePatternOnly\n";
    }
    std::cout<<"RM_VGA_WINDOW_SUMMARY anchor="<<anchor110
             <<" contiguousMovDxOut="<<exact110<<"\n";
   }
  }
 }


 // Milestone 111: inspect a bounded region around the parameterized VGA DAC
 // writer and survey candidate near calls. This is NOT a control-flow analysis:
 // relative CALL targets use file offsets only, which may not correspond to
 // runtime addresses in an overlay.
 {
  const auto path111=std::filesystem::path(argv[1])/"STTNG.OVL";
  std::ifstream input111(path111,std::ios::binary);
  if(!input111){
   std::cout<<"RM_DAC_WRITER_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> data111(
    (std::istreambuf_iterator<char>(input111)),std::istreambuf_iterator<char>());
   const size_t anchor111=1358117;
   const size_t start111=anchor111>96?anchor111-96:0;
   const size_t end111=std::min(data111.size(),anchor111+129);
   std::cout<<"RM_DAC_WRITER_STATUS read=1 bytes="<<data111.size()
            <<" anchor="<<anchor111<<" start="<<start111
            <<" endExclusive="<<end111<<"\n";
   for(size_t pos111=start111;pos111<end111;pos111+=16){
    std::cout<<"RM_DAC_WRITER_HEX offset="<<pos111<<" hex=";
    for(size_t j111=pos111;j111<std::min(end111,pos111+16);++j111){
     if(j111!=pos111)std::cout<<" ";
     std::cout<<std::hex<<std::setw(2)<<std::setfill('0')<<unsigned(data111[j111]);
    }
    std::cout<<std::dec<<std::setfill(' ')<<"\n";
   }
   // Verify the exact observed function prologue and DAC writer body bytes.
   const uint8_t signature111[]={
    0x55,0x8B,0xEC,0x50,0x51,0x52,0x56,0x8B,0x4D,0x10,
    0x8B,0x45,0x0C,0x8B,0x75,0x08,0x66,0xBA,0xC8,0x03,
    0xEE,0x66,0x42,0xAC,0xEE,0xAC,0xEE,0xAC,0xEE,
    0x49,0x75,0xF7,0x5E,0x5A,0x59,0x58,0x5D,0xC3
   };
   const size_t entry111=1358101;
   const bool signatureFits111=entry111+sizeof(signature111)<=data111.size();
   const bool signatureMatches111=signatureFits111&&std::equal(
    std::begin(signature111),std::end(signature111),data111.begin()+entry111);
   std::cout<<"RM_DAC_WRITER_SIGNATURE entryOffset="<<entry111
            <<" matches="<<signatureMatches111
            <<" bytes="<<sizeof(signature111)
            <<" parameterOffsets=8,12,16"
            <<" note=staticBytePatternOnly\n";
   // An E8 displacement is relative to the next instruction in a flat
   // address space; overlay relocation and segment mapping are unknown.
   size_t rawCalls111=0,nearEntry111=0;
   for(size_t i111=0;i111+4<data111.size();++i111){
    if(data111[i111]!=0xE8)continue;
    ++rawCalls111;
    const uint32_t displacement111=
      uint32_t(data111[i111+1])|
      (uint32_t(data111[i111+2])<<8)|
      (uint32_t(data111[i111+3])<<16)|
      (uint32_t(data111[i111+4])<<24);
    const int64_t target111=int64_t(i111)+5+int64_t(int32_t(displacement111));
    if(target111>=int64_t(entry111)-32&&target111<=int64_t(entry111)+32){
     ++nearEntry111;
     if(nearEntry111<=20)
      std::cout<<"RM_DAC_WRITER_CALL_CANDIDATE offset="<<i111
               <<" flatTarget="<<target111
               <<" deltaFromEntry="<<(target111-int64_t(entry111))
               <<" scope=unvalidatedFlatFileOffset\n";
    }
   }
   std::cout<<"RM_DAC_WRITER_CALL_SUMMARY rawE8Patterns="<<rawCalls111
            <<" flatNearEntryCandidates="<<nearEntry111
            <<" displayed="<<std::min<size_t>(nearEntry111,20)
            <<" warning=overlayAddressMappingUnknown\n";
  }
 }


 // Milestone 112: corrected DAC-writer entry and bounded caller neighborhoods.
 // The E8 sites remain byte-pattern candidates, not validated runtime calls.
 {
  const auto path112=std::filesystem::path(argv[1])/"STTNG.OVL";
  std::ifstream input112(path112,std::ios::binary);
  if(!input112){
   std::cout<<"RM_DAC_CALLERS_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> data112(
    (std::istreambuf_iterator<char>(input112)),std::istreambuf_iterator<char>());
   const size_t entry112=1358100;
   const uint8_t signature112[]={
    0x55,0x8B,0xEC,0x50,0x51,0x52,0x56,0x8B,0x4D,0x10,
    0x8B,0x45,0x0C,0x8B,0x75,0x08,0x66,0xBA,0xC8,0x03,
    0xEE,0x66,0x42,0xAC,0xEE,0xAC,0xEE,0xAC,0xEE,
    0x49,0x75,0xF7,0x5E,0x5A,0x59,0x58,0x5D,0xC3
   };
   const bool matches112=entry112+sizeof(signature112)<=data112.size()&&
    std::equal(std::begin(signature112),std::end(signature112),data112.begin()+entry112);
   std::cout<<"RM_DAC_CALLERS_STATUS read=1 bytes="<<data112.size()
            <<" candidateCalls=4 radiusBefore=80 radiusAfter=24\n";
   std::cout<<"RM_DAC_CALLERS_SIGNATURE entryOffset="<<entry112
            <<" matches="<<matches112<<" bytes="<<sizeof(signature112)
            <<" scope=staticBytePatternOnly\n";
   const size_t sites112[]={1165092,1165591,1166889,1167061};
   for(size_t site112:sites112){
    if(site112+5>data112.size()){
     std::cout<<"RM_DAC_CALLERS_SITE offset="<<site112<<" available=0\n";
     continue;
    }
    const bool e8_112=data112[site112]==0xE8;
    const uint32_t displacement112=
      uint32_t(data112[site112+1])|
      (uint32_t(data112[site112+2])<<8)|
      (uint32_t(data112[site112+3])<<16)|
      (uint32_t(data112[site112+4])<<24);
    const int64_t target112=int64_t(site112)+5+int64_t(int32_t(displacement112));
    const size_t start112=site112>80?site112-80:0;
    const size_t end112=std::min(data112.size(),site112+29);
    std::cout<<"RM_DAC_CALLERS_SITE offset="<<site112
             <<" available=1 opcodeE8="<<e8_112
             <<" flatTarget="<<target112
             <<" matchesEntry="<<(e8_112&&target112==int64_t(entry112))
             <<" start="<<start112<<" endExclusive="<<end112
             <<" scope=unvalidatedFlatFileOffset\n";
    for(size_t row112=start112;row112<end112;row112+=16){
     std::cout<<"RM_DAC_CALLERS_HEX site="<<site112
              <<" offset="<<row112<<" hex=";
     for(size_t j112=row112;j112<std::min(end112,row112+16);++j112){
      if(j112!=row112)std::cout<<" ";
      std::cout<<std::hex<<std::setw(2)<<std::setfill('0')<<unsigned(data112[j112]);
     }
     std::cout<<std::dec<<std::setfill(' ')<<"\n";
    }
   }
  }
 }


 // Milestone 113: bounded palette-structure and RM-reference proximity survey.
 // Raw byte occurrences are not instruction decoding or runtime data-flow proof.
 {
  const auto path113=std::filesystem::path(argv[1])/"STTNG.OVL";
  std::ifstream input113(path113,std::ios::binary);
  if(!input113){
   std::cout<<"RM_PALETTE_TRACE_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> data113(
    (std::istreambuf_iterator<char>(input113)),std::istreambuf_iterator<char>());
   const size_t sites113[]={1166889,1167061};
   const uint8_t global113[]={0xF8,0x17,0x12,0x00};
   const uint8_t rm113[]={0x2E,0x52,0x4D};
   std::cout<<"RM_PALETTE_TRACE_STATUS read=1 bytes="<<data113.size()
            <<" callerSites=2 radius=192 scope=rawByteSurvey\n";
   for(size_t site113:sites113){
    const size_t start113=site113>192?site113-192:0;
    const size_t end113=std::min(data113.size(),site113+193);
    size_t globals113=0,offsets113=0;
    std::cout<<"RM_PALETTE_TRACE_CALLER site="<<site113
             <<" start="<<start113<<" endExclusive="<<end113<<"\n";
    for(size_t p113=start113;p113<end113;++p113){
     if(p113+sizeof(global113)<=end113&&
        std::equal(std::begin(global113),std::end(global113),data113.begin()+p113)){
      ++globals113;
      std::cout<<"RM_PALETTE_TRACE_GLOBAL site="<<site113
               <<" offset="<<p113<<" addressLE=001217F8\n";
     }
     if(p113+3<=end113&&data113[p113]==0x00&&
        (data113[p113+1]==0x7E||data113[p113+1]==0x80||data113[p113+1]==0x84)&&
        data113[p113+2]==0x00){
      ++offsets113;
      if(offsets113<=24)
       std::cout<<"RM_PALETTE_TRACE_FIELD_CANDIDATE site="<<site113
                <<" offset="<<p113<<" fieldHex="<<std::hex
                <<unsigned(data113[p113+1])<<std::dec
                <<" scope=rawBytePattern\n";
     }
    }
    std::cout<<"RM_PALETTE_TRACE_CALLER_SUMMARY site="<<site113
             <<" globalPatterns="<<globals113
             <<" fieldPatterns="<<offsets113<<"\n";
   }
   // Search the complete overlay for references to the exact global address
   // bytes; summarize distribution without claiming common ownership.
   size_t globalTotal113=0,nearRm113=0,shown113=0;
   for(size_t p113=0;p113+sizeof(global113)<=data113.size();++p113){
    if(!std::equal(std::begin(global113),std::end(global113),data113.begin()+p113))
     continue;
    ++globalTotal113;
    const size_t begin113=p113>512?p113-512:0;
    const size_t finish113=std::min(data113.size(),p113+516);
    bool rmNearby113=false;
    for(size_t q113=begin113;q113+sizeof(rm113)<=finish113;++q113){
     if(std::equal(std::begin(rm113),std::end(rm113),data113.begin()+q113)){
      rmNearby113=true;break;
     }
    }
    if(rmNearby113)++nearRm113;
    if(shown113<32){
     ++shown113;
     std::cout<<"RM_PALETTE_TRACE_GLOBAL_REF offset="<<p113
              <<" rmExtensionWithin512="<<rmNearby113<<"\n";
    }
   }
   std::cout<<"RM_PALETTE_TRACE_GLOBAL_SUMMARY matches="<<globalTotal113
            <<" rmExtensionNearby="<<nearRm113
            <<" displayed="<<shown113
            <<" caution=rawBytesNotDataFlow\n";
  }
 }


 // Milestone 114: bounded instruction-oriented inspection of the two palette
 // call paths. Opcode labels are limited to verified local byte templates;
 // this is not a complete disassembler or a runtime data-flow analysis.
 {
  const auto path114=std::filesystem::path(argv[1])/"STTNG.OVL";
  std::ifstream input114(path114,std::ios::binary);
  if(!input114){
   std::cout<<"RM_PALETTE_FLOW_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> data114(
    (std::istreambuf_iterator<char>(input114)),std::istreambuf_iterator<char>());
   const size_t sites114[]={1166889,1167061};
   std::cout<<"RM_PALETTE_FLOW_STATUS read=1 bytes="<<data114.size()
            <<" callerSites=2 scope=selectedInstructionTemplates\n";
   for(size_t site114:sites114){
    const size_t begin114=site114>96?site114-96:0;
    const size_t end114=std::min(data114.size(),site114+65);
    size_t global114=0,fields114=0,branches114=0;
    std::cout<<"RM_PALETTE_FLOW_CALLER site="<<site114
             <<" start="<<begin114<<" endExclusive="<<end114<<"\n";
    for(size_t p114=begin114;p114<end114;++p114){
     // A1 imm32: MOV EAX, moffs32, with exact global address.
     if(p114+5<=end114&&data114[p114]==0xA1&&
        data114[p114+1]==0xF8&&data114[p114+2]==0x17&&
        data114[p114+3]==0x12&&data114[p114+4]==0x00){
      ++global114;
      std::cout<<"RM_PALETTE_FLOW_GLOBAL_LOAD site="<<site114
               <<" offset="<<p114<<" instruction=mov_eax_moffs32"
               <<" address=001217F8\n";
     }
     // Match displacement bytes only in the observed 32-bit ModRM forms:
     // 0F BF /r disp8, 66 8B /r disp32, 8B /r disp32.
     if(p114+4<=end114&&data114[p114]==0x0F&&data114[p114+1]==0xBF&&
        (data114[p114+2]&0xC0)==0x40&&
        (data114[p114+3]==0x7E||data114[p114+3]==0x80||
         data114[p114+3]==0x84)){
      ++fields114;
      std::cout<<"RM_PALETTE_FLOW_FIELD site="<<site114
               <<" offset="<<p114<<" opcode=movsx_r32_rm16_disp8"
               <<" displacementSigned="<<int(int8_t(data114[p114+3]))<<"\n";
     }
     if(p114+7<=end114&&
        ((data114[p114]==0x66&&data114[p114+1]==0x8B&&
          (data114[p114+2]&0xC0)==0x80&&
          data114[p114+3]==0x80&&data114[p114+4]==0&&
          data114[p114+5]==0&&data114[p114+6]==0)||
         (data114[p114]==0x8B&&(data114[p114+1]&0xC0)==0x80&&
          (data114[p114+2]==0x80||data114[p114+2]==0x84)&&
          data114[p114+3]==0&&data114[p114+4]==0&&
          data114[p114+5]==0))){
      ++fields114;
      std::cout<<"RM_PALETTE_FLOW_FIELD site="<<site114
               <<" offset="<<p114<<" opcode=mov_register_memory_disp32"
               <<" scope=observedByteTemplate\n";
     }
     if(p114+6<=end114&&data114[p114]==0x0F&&
        (data114[p114+1]==0x84||data114[p114+1]==0x85)){
      ++branches114;
      const uint32_t raw114=uint32_t(data114[p114+2])|
       (uint32_t(data114[p114+3])<<8)|
       (uint32_t(data114[p114+4])<<16)|
       (uint32_t(data114[p114+5])<<24);
      std::cout<<"RM_PALETTE_FLOW_BRANCH site="<<site114
               <<" offset="<<p114
               <<" condition="<<(data114[p114+1]==0x84?"JZ":"JNZ")
               <<" flatTarget="<<(int64_t(p114)+6+int64_t(int32_t(raw114)))
               <<" scope=unvalidatedFlatFileOffset\n";
     }
    }
    std::cout<<"RM_PALETTE_FLOW_SUMMARY site="<<site114
             <<" globalLoads="<<global114<<" fieldCandidates="<<fields114
             <<" conditionalBranchCandidates="<<branches114
             <<" caution=noRuntimeControlFlowProof\n";
   }
  }
 }


 // Milestone 115: inspect branch destinations and deduplicate overlapping
 // structure-field byte templates. This is bounded static evidence, not a
 // full instruction decoder or proof of executable control flow.
 {
  const auto path115=std::filesystem::path(argv[1])/"STTNG.OVL";
  std::ifstream input115(path115,std::ios::binary);
  if(!input115){
   std::cout<<"RM_PALETTE_DEST_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> data115(
    (std::istreambuf_iterator<char>(input115)),std::istreambuf_iterator<char>());
   const size_t destinations115[]={1167145,1167213,1167351};
   std::cout<<"RM_PALETTE_DEST_STATUS read=1 bytes="<<data115.size()
            <<" destinations=3 radiusBefore=48 radiusAfter=96"
            <<" scope=staticByteWindows\n";
   for(size_t destination115:destinations115){
    if(destination115>=data115.size()){
     std::cout<<"RM_PALETTE_DEST_ANCHOR offset="<<destination115
              <<" available=0\n";
     continue;
    }
    const size_t begin115=destination115>48?destination115-48:0;
    const size_t end115=std::min(data115.size(),destination115+97);
    std::cout<<"RM_PALETTE_DEST_ANCHOR offset="<<destination115
             <<" available=1 start="<<begin115
             <<" endExclusive="<<end115<<"\n";
    for(size_t row115=begin115;row115<end115;row115+=16){
     std::cout<<"RM_PALETTE_DEST_HEX anchor="<<destination115
              <<" offset="<<row115<<" hex=";
     for(size_t j115=row115;j115<std::min(end115,row115+16);++j115){
      if(j115!=row115)std::cout<<" ";
      std::cout<<std::hex<<std::setw(2)<<std::setfill('0')
               <<unsigned(data115[j115]);
     }
     std::cout<<std::dec<<std::setfill(' ')<<"\n";
    }
    size_t returns115=0,nearCalls115=0;
    for(size_t p115=destination115;p115<end115;++p115){
     if(data115[p115]==0xC3||data115[p115]==0xC2){
      ++returns115;
      if(returns115<=12)
       std::cout<<"RM_PALETTE_DEST_RET_CANDIDATE anchor="<<destination115
                <<" offset="<<p115<<" opcode="
                <<(data115[p115]==0xC3?"C3":"C2")
                <<" scope=rawBytePattern\n";
     }
     if(data115[p115]==0xE8&&p115+5<=end115){
      ++nearCalls115;
      const uint32_t raw115=uint32_t(data115[p115+1])|
       (uint32_t(data115[p115+2])<<8)|
       (uint32_t(data115[p115+3])<<16)|
       (uint32_t(data115[p115+4])<<24);
      if(nearCalls115<=12)
       std::cout<<"RM_PALETTE_DEST_CALL_CANDIDATE anchor="<<destination115
                <<" offset="<<p115
                <<" flatTarget="<<(int64_t(p115)+5+int64_t(int32_t(raw115)))
                <<" scope=unvalidatedFlatFileOffset\n";
     }
    }
    std::cout<<"RM_PALETTE_DEST_SUMMARY anchor="<<destination115
             <<" rawRetBytes="<<returns115<<" rawCallBytes="<<nearCalls115<<"\n";
   }
   // Correct the prior survey's duplicate recognition of the same disp32
   // instruction at adjacent starting offsets. Match the exact observed
   // templates and advance past the full matched instruction.
   const size_t callers115[]={1166889,1167061};
   for(size_t site115:callers115){
    const size_t begin115=site115>96?site115-96:0;
    const size_t end115=std::min(data115.size(),site115+65);
    size_t count115=0;
    for(size_t p115=begin115;p115<end115;){
     size_t len115=0;
     const char* kind115=nullptr;
     int displacement115=0;
     if(p115+4<=end115&&data115[p115]==0x0F&&data115[p115+1]==0xBF&&
        (data115[p115+2]&0xC0)==0x40&&data115[p115+3]==0x7E){
      len115=4;kind115="movsx_disp8_7E";displacement115=126;
     }else if(p115+7<=end115&&data115[p115]==0x66&&
        data115[p115+1]==0x8B&&(data115[p115+2]&0xC0)==0x80&&
        data115[p115+3]==0x80&&data115[p115+4]==0&&
        data115[p115+5]==0&&data115[p115+6]==0){
      len115=7;kind115="mov16_disp32_80";displacement115=128;
     }else if(p115+6<=end115&&data115[p115]==0x8B&&
        (data115[p115+1]&0xC0)==0x80&&
        (data115[p115+2]==0x80||data115[p115+2]==0x84)&&
        data115[p115+3]==0&&data115[p115+4]==0&&
        data115[p115+5]==0){
      len115=6;kind115="mov32_disp32";
      displacement115=data115[p115+2];
     }
     if(len115){
      ++count115;
      std::cout<<"RM_PALETTE_DEST_FIELD site="<<site115
               <<" offset="<<p115<<" kind="<<kind115
               <<" displacement="<<displacement115
               <<" length="<<len115
               <<" scope=selectedByteTemplate\n";
      p115+=len115;
     }else ++p115;
    }
    std::cout<<"RM_PALETTE_DEST_FIELD_SUMMARY site="<<site115
             <<" nonOverlappingMatches="<<count115
             <<" caution=notFullInstructionDecode\n";
   }
  }
 }


 // Milestone 116: bounded inspection of the alternate-path call target at
 // flat file offset 1157325. The offset is a candidate, not a proven runtime
 // address; raw opcode scans do not establish instruction boundaries.
 {
  const auto path116=std::filesystem::path(argv[1])/"STTNG.OVL";
  std::ifstream input116(path116,std::ios::binary);
  if(!input116){
   std::cout<<"RM_PALETTE_BUFFER_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> data116(
    (std::istreambuf_iterator<char>(input116)),std::istreambuf_iterator<char>());
   const size_t target116=1157325;
   if(target116>=data116.size()){
    std::cout<<"RM_PALETTE_BUFFER_STATUS read=1 targetAvailable=0 bytes="
             <<data116.size()<<"\n";
   }else{
    const size_t start116=target116>64?target116-64:0;
    const size_t end116=std::min(data116.size(),target116+257);
    std::cout<<"RM_PALETTE_BUFFER_STATUS read=1 targetAvailable=1 bytes="
             <<data116.size()<<" target="<<target116
             <<" start="<<start116<<" endExclusive="<<end116
             <<" scope=staticByteWindow\n";
    for(size_t row116=start116;row116<end116;row116+=16){
     std::cout<<"RM_PALETTE_BUFFER_HEX offset="<<row116<<" hex=";
     for(size_t j116=row116;j116<std::min(end116,row116+16);++j116){
      if(j116!=row116)std::cout<<" ";
      std::cout<<std::hex<<std::setw(2)<<std::setfill('0')
               <<unsigned(data116[j116]);
     }
     std::cout<<std::dec<<std::setfill(' ')<<"\n";
    }
    size_t ret116=0,call116=0,string116=0,rep116=0;
    for(size_t p116=target116;p116<end116;++p116){
     if(data116[p116]==0xC3||data116[p116]==0xC2){
      ++ret116;
      if(ret116<=12)
       std::cout<<"RM_PALETTE_BUFFER_RET_CANDIDATE offset="<<p116
                <<" opcode="<<(data116[p116]==0xC3?"C3":"C2")
                <<" scope=rawBytePattern\n";
     }
     if(data116[p116]==0xE8&&p116+5<=end116){
      ++call116;
      const uint32_t raw116=uint32_t(data116[p116+1])|
       (uint32_t(data116[p116+2])<<8)|
       (uint32_t(data116[p116+3])<<16)|
       (uint32_t(data116[p116+4])<<24);
      if(call116<=12)
       std::cout<<"RM_PALETTE_BUFFER_CALL_CANDIDATE offset="<<p116
                <<" flatTarget="<<(int64_t(p116)+5+int64_t(int32_t(raw116)))
                <<" scope=unvalidatedFlatFileOffset\n";
     }
     if(data116[p116]==0xAC||data116[p116]==0xAA||
        data116[p116]==0xA4||data116[p116]==0xA5){
      ++string116;
      if(string116<=12)
       std::cout<<"RM_PALETTE_BUFFER_STRING_CANDIDATE offset="<<p116
                <<" opcode="<<std::hex<<unsigned(data116[p116])<<std::dec
                <<" scope=rawBytePattern\n";
     }
     if(data116[p116]==0xF3){
      ++rep116;
      if(rep116<=12)
       std::cout<<"RM_PALETTE_BUFFER_REP_CANDIDATE offset="<<p116
                <<" scope=rawBytePattern\n";
     }
    }
    std::cout<<"RM_PALETTE_BUFFER_SUMMARY rawReturnBytes="<<ret116
             <<" rawCallBytes="<<call116
             <<" rawStringOpBytes="<<string116
             <<" rawRepPrefixBytes="<<rep116
             <<" caution=noInstructionBoundaryOrDataFlowProof\n";
   }
  }
 }


 // Milestone 117: verify the complete memcpy call argument sequence.
 // Register labels are based on the exact 32-bit x86 byte template below.
 // This is static evidence only; pointer contents and runtime count are unknown.
 {
  const auto path117=std::filesystem::path(argv[1])/"STTNG.OVL";
  std::ifstream input117(path117,std::ios::binary);
  if(!input117){
   std::cout<<"RM_PALETTE_COPY_ARGS_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> data117(
    (std::istreambuf_iterator<char>(input117)),std::istreambuf_iterator<char>());
   const size_t start117=1167213;
   // xor ebx,ebx; mov bl,[eax+0f]; lea eax,[ebx*4];
   // sub eax,ebx; push eax; mov ebx,[esi+0c]; push ebx;
   // mov ecx,[edx+84]; push ecx; call rel32
   const std::vector<uint8_t> prefix117={
    0x31,0xDB,0x8A,0x58,0x0F,0x8D,0x04,0x9D,0,0,0,0,
    0x29,0xD8,0x50,0x8B,0x5E,0x0C,0x53,0x8B,0x8A,0x84,0,0,0,0x51,0xE8
   };
   bool matched117=start117+prefix117.size()+4<=data117.size();
   if(matched117)
    for(size_t i117=0;i117<prefix117.size();++i117)
     if(data117[start117+i117]!=prefix117[i117]){matched117=false;break;}
   std::cout<<"RM_PALETTE_COPY_ARGS_STATUS read=1 bytes="<<data117.size()
            <<" sequenceOffset="<<start117<<" exactTemplateMatch="
            <<(matched117?1:0)<<" scope=staticX86Template\n";
   if(matched117){
    const size_t call117=start117+prefix117.size()-1;
    const uint32_t raw117=uint32_t(data117[call117+1])|
      (uint32_t(data117[call117+2])<<8)|
      (uint32_t(data117[call117+3])<<16)|
      (uint32_t(data117[call117+4])<<24);
    const int64_t target117=int64_t(call117)+5+int64_t(int32_t(raw117));
    std::cout<<"RM_PALETTE_COPY_ARGS_COUNT expression=3*zeroExtendByte_at_EAX_plus_0F"
             <<" derivation=lea_4x_then_subtract_1x"
             <<" bytesPerEntry=3 countRuntimeUnknown=1\n";
    std::cout<<"RM_PALETTE_COPY_ARGS_SOURCE expression=dword_at_ESI_plus_0C"
             <<" pushedSecond=1\n";
    std::cout<<"RM_PALETTE_COPY_ARGS_DEST expression=dword_at_EDX_plus_84"
             <<" pushedThird=1\n";
    std::cout<<"RM_PALETTE_COPY_ARGS_CALL offset="<<call117
             <<" flatTarget="<<target117
             <<" expectedMemcpyTarget=1157325"
             <<" targetMatches="<<(target117==1157325?1:0)
             <<" argumentOrder=destination_source_byteCount\n";
    std::cout<<"RM_PALETTE_COPY_ARGS_RANGE countFieldBits=8"
             <<" theoreticalMaxBytes=765"
             <<" entries128Bytes=384 entries256Bytes=768"
             <<" caution=actualCountAndPaletteOwnershipUnknown\n";
    const size_t after117=call117+5;
    const std::vector<uint8_t> post117={
      0xA1,0xF8,0x17,0x12,0x00,0x83,0xC4,0x0C,
      0x66,0xC7,0x80,0x82,0,0,0,0x01,0x00
    };
    bool postMatch117=after117+post117.size()<=data117.size();
    if(postMatch117)
     for(size_t i117=0;i117<post117.size();++i117)
      if(data117[after117+i117]!=post117[i117]){postMatch117=false;break;}
    std::cout<<"RM_PALETTE_COPY_ARGS_POST offset="<<after117
             <<" exactTemplateMatch="<<(postMatch117?1:0)
             <<" operation=globalPointer_load_stackCleanup_word82_set1"
             <<" caution=stateMeaningUnknown\n";
   }
  }
 }


 // Milestone 118: inspect the provenance of ESI and the RGB count field
 // immediately before the verified memcpy path. The windows deliberately
 // include both DAC call sites and the intervening control flow. They do
 // not establish a complete reaching-definitions or runtime pointer trace.
 {
  const auto path118=std::filesystem::path(argv[1])/"STTNG.OVL";
  std::ifstream input118(path118,std::ios::binary);
  if(!input118){
   std::cout<<"RM_PALETTE_SOURCE_TRACE_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> data118(
    (std::istreambuf_iterator<char>(input118)),std::istreambuf_iterator<char>());
   const size_t anchors118[]={1166889,1167061,1167213};
   std::cout<<"RM_PALETTE_SOURCE_TRACE_STATUS read=1 bytes="<<data118.size()
            <<" anchors=3 scope=boundedStaticHexAndSelectedTemplates\n";
   for(size_t anchor118:anchors118){
    const size_t start118=anchor118>128?anchor118-128:0;
    const size_t end118=std::min(data118.size(),anchor118+49);
    std::cout<<"RM_PALETTE_SOURCE_TRACE_WINDOW anchor="<<anchor118
             <<" start="<<start118<<" endExclusive="<<end118<<"\n";
    for(size_t row118=start118;row118<end118;row118+=16){
     std::cout<<"RM_PALETTE_SOURCE_TRACE_HEX anchor="<<anchor118
              <<" offset="<<row118<<" hex=";
     for(size_t j118=row118;j118<std::min(end118,row118+16);++j118){
      if(j118!=row118)std::cout<<" ";
      std::cout<<std::hex<<std::setw(2)<<std::setfill('0')
               <<unsigned(data118[j118]);
     }
     std::cout<<std::dec<<std::setfill(' ')<<"\n";
    }
    size_t count118=0,source118=0,global118=0;
    for(size_t p118=start118;p118<end118;++p118){
     if(p118+5<=end118&&data118[p118]==0xA1&&
        data118[p118+1]==0xF8&&data118[p118+2]==0x17&&
        data118[p118+3]==0x12&&data118[p118+4]==0){
      ++global118;
      std::cout<<"RM_PALETTE_SOURCE_TRACE_GLOBAL anchor="<<anchor118
               <<" offset="<<p118<<" kind=mov_eax_global_pointer\n";
     }
     if(p118+3<=end118&&data118[p118]==0x8A&&
        data118[p118+1]==0x58&&data118[p118+2]==0x0F){
      ++count118;
      std::cout<<"RM_PALETTE_SOURCE_TRACE_COUNT anchor="<<anchor118
               <<" offset="<<p118
               <<" kind=mov_bl_byte_eax_plus_0F"
               <<" countRuntimeUnknown=1\n";
     }
     if(p118+3<=end118&&data118[p118]==0x8B&&
        data118[p118+1]==0x5E&&data118[p118+2]==0x0C){
      ++source118;
      std::cout<<"RM_PALETTE_SOURCE_TRACE_SOURCE anchor="<<anchor118
               <<" offset="<<p118
               <<" kind=mov_ebx_dword_esi_plus_0C"
               <<" esiProvenanceUnknown=1\n";
     }
    }
    std::cout<<"RM_PALETTE_SOURCE_TRACE_SUMMARY anchor="<<anchor118
             <<" globalLoads="<<global118
             <<" countLoads="<<count118
             <<" sourceLoads="<<source118
             <<" caution=overlappingWindowsAndNoControlFlowProof\n";
   }
   // Inspect a broader, contiguous context around the copy's predecessor.
   const size_t begin118=1166600,end118=std::min(data118.size(),size_t(1167265));
   for(size_t p118=begin118;p118+2<end118;++p118){
    // ESI register assignments: MOV ESI,r/m32; LEA ESI,m; POP ESI;
    // MOV ESI,imm32. These are candidates until decoded on instruction
    // boundaries, and may occur on mutually exclusive paths.
    const bool mov118=data118[p118]==0x8B&&
                      (data118[p118+1]&0x38)==0x30;
    const bool lea118=data118[p118]==0x8D&&
                      (data118[p118+1]&0x38)==0x30;
    const bool pop118=data118[p118]==0x5E;
    const bool imm118=data118[p118]==0xBE&&p118+5<=end118;
    if(mov118||lea118||pop118||imm118){
     std::cout<<"RM_PALETTE_SOURCE_TRACE_ESI_CANDIDATE offset="<<p118
              <<" opcode="<<std::hex<<unsigned(data118[p118])<<std::dec
              <<" kind="<<(mov118?"mov":lea118?"lea":pop118?"pop":"immediate")
              <<" scope=rawByteCandidateNotReachingDefinition\n";
    }
   }
   std::cout<<"RM_PALETTE_SOURCE_TRACE_LIMITATION"
            <<" runtimeRGBCount=unknown"
            <<" resourceIdentity=unknown"
            <<" esiReachingDefinition=unverified\n";
  }
 }


 // Milestone 119: anchored instruction sequences and control-flow evidence.
 // Unlike raw opcode surveys, each report requires a complete byte template
 // at a known offset. No general disassembly or reaching-definitions claim.
 {
  const auto path119=std::filesystem::path(argv[1])/"STTNG.OVL";
  std::ifstream in119(path119,std::ios::binary);
  if(!in119){
   std::cout<<"RM_PALETTE_ENTRY_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> d119(
    (std::istreambuf_iterator<char>(in119)),std::istreambuf_iterator<char>());
   auto match119=[&](size_t p119,const std::vector<uint8_t>& bytes119){
    if(p119>d119.size()||bytes119.size()>d119.size()-p119)return false;
    for(size_t i119=0;i119<bytes119.size();++i119)
     if(d119[p119+i119]!=bytes119[i119])return false;
    return true;
   };
   auto rel119=[&](size_t p119,size_t displacement119,size_t length119)->int64_t{
    if(p119>d119.size()||length119>d119.size()-p119||
       displacement119+4>length119)return -1;
    const size_t q119=p119+displacement119;
    const uint32_t v119=uint32_t(d119[q119])|
      (uint32_t(d119[q119+1])<<8)|
      (uint32_t(d119[q119+2])<<16)|
      (uint32_t(d119[q119+3])<<24);
    return int64_t(p119)+int64_t(length119)+int64_t(int32_t(v119));
   };
   std::cout<<"RM_PALETTE_ENTRY_STATUS read=1 bytes="<<d119.size()
            <<" scope=anchoredInstructionTemplatesNotFullDecoder\n";
   // The preceding ESI initialization must be sought at a function boundary,
   // not inferred from arbitrary 5E or 8B bytes in the middle of instructions.
   const size_t begin119=1166300,end119=std::min(d119.size(),size_t(1166800));
   size_t prologues119=0;
   for(size_t p119=begin119;p119+4<=end119;++p119){
    const bool frame119=match119(p119,{0x55,0x89,0xE5})||
                        match119(p119,{0x55,0x8B,0xEC});
    const bool save119=match119(p119,{0x53,0x56,0x57,0x55})||
                       match119(p119,{0x56,0x57,0x55});
    if(frame119||save119){
     ++prologues119;
     std::cout<<"RM_PALETTE_ENTRY_PROLOGUE_CANDIDATE offset="<<p119
              <<" kind="<<(frame119?"frame":"savedRegisters")
              <<" caution=notProvenFunctionBoundary\n";
    }
   }
   std::cout<<"RM_PALETTE_ENTRY_PROLOGUE_SUMMARY start="<<begin119
            <<" endExclusive="<<end119<<" candidates="<<prologues119<<"\n";
   // Known instruction-boundary candidates, validated as full templates.
   struct Branch119{size_t offset;size_t displacement;size_t length;
                    const char* kind;};
   const Branch119 branches119[]={
    {1166986,2,6,"jz"},{1166997,2,6,"jnz"},
    {1167026,2,6,"jnz"},{1167157,2,6,"jnz"}
   };
   for(const auto& b119:branches119){
    const bool valid119=b119.offset+6<=d119.size()&&
      d119[b119.offset]==0x0F&&
      d119[b119.offset+1]==(std::string(b119.kind)=="jz"?0x84:0x85);
    std::cout<<"RM_PALETTE_ENTRY_BRANCH offset="<<b119.offset
             <<" templateMatch="<<(valid119?1:0)
             <<" kind="<<b119.kind;
    if(valid119)std::cout<<" flatTarget="<<rel119(b119.offset,2,6);
    std::cout<<" caution=pathFeasibilityUnknown\n";
   }
   const size_t copy119=1167213;
   const bool count119=match119(copy119,{
     0x31,0xDB,0x8A,0x58,0x0F,0x8D,0x04,0x9D,0,0,0,0,0x29,0xD8,0x50});
   const bool source119=match119(1167228,{0x8B,0x5E,0x0C,0x53});
   const bool dest119=match119(1167232,{0x8B,0x8A,0x84,0,0,0,0x51});
   const bool call119=match119(1167239,{0xE8,0x41,0xD9,0xFF,0xFF});
   std::cout<<"RM_PALETTE_ENTRY_COPY_PATH offset="<<copy119
            <<" countTemplate="<<(count119?1:0)
            <<" sourceTemplate="<<(source119?1:0)
            <<" destinationTemplate="<<(dest119?1:0)
            <<" callTemplate="<<(call119?1:0)
            <<" countBaseRegister=EAX sourceBaseRegister=ESI"
            <<" destinationBaseRegister=EDX\n";
   // The call immediately preceding the copy path is on a distinct
   // branch; it cannot be assumed to provide EAX for the copy.
   // Byte-indexed evidence, independent of guessed instruction starts.
   // Show every byte and its absolute offset around the suspected CALL/JMP.
   const size_t inspectBegin119=1167196;
   const size_t inspectEnd119=std::min(d119.size(),size_t(1167216));
   for(size_t p119=inspectBegin119;p119<inspectEnd119;++p119){
    std::cout<<"RM_PALETTE_ENTRY_BYTE offset="<<p119
             <<" hex="<<std::hex<<std::setw(2)<<std::setfill('0')
             <<unsigned(d119[p119])<<std::dec<<std::setfill(' ')<<"\n";
   }
   // Find the complete CALL/JMP byte sequence in this bounded window.
   // This reports the observed offsets, not an assumed one.
   const std::vector<uint8_t> callJump119={
    0xE8,0x69,0xF2,0x02,0x00,0xEB,0x6E
   };
   size_t matches119=0;
   for(size_t p119=inspectBegin119;
       p119+callJump119.size()<=inspectEnd119;++p119){
    if(!match119(p119,callJump119))continue;
    ++matches119;
    const int64_t callTarget119=rel119(p119,1,5);
    const int8_t jumpDisp119=int8_t(d119[p119+6]);
    const int64_t jumpTarget119=int64_t(p119)+7+int64_t(jumpDisp119);
    std::cout<<"RM_PALETTE_ENTRY_CALL_JUMP_MATCH callOffset="<<p119
             <<" jumpOffset="<<(p119+5)
             <<" callFlatTarget="<<callTarget119
             <<" jumpFlatTarget="<<jumpTarget119
             <<" copyPathSkippedByJump="<<(jumpTarget119>1167213?1:0)
             <<" caution=staticByteSequenceNotPathFeasibilityProof\n";
   }
   std::cout<<"RM_PALETTE_ENTRY_CALL_JUMP_SUMMARY matches="<<matches119
            <<" searchStart="<<inspectBegin119
            <<" searchEndExclusive="<<inspectEnd119<<"\n";
   const bool prior119=match119(1167206,{0xE8,0x69,0xF2,0x02,0x00});
   const bool jump119=match119(1167211,{0xEB,0x6E});
   std::cout<<"RM_PALETTE_ENTRY_PRECEDING_CALL offset=1167206"
            <<" templateMatch="<<(prior119?1:0);
   if(prior119)std::cout<<" flatTarget="<<rel119(1167206,1,5);
   std::cout<<" followedByJump="<<(jump119?1:0)
            <<" copyPathFallthrough="<<(jump119?0:1)
            <<" caution=alternatePathNotCountProducerProof\n";
   // Full known byte sequence: mov eax,[global]; mov edx,[esp+1c];
   // movsx eax,byte [eax+2f]; mov [edx],eax. EAX is overwritten.
   const bool overwrite119=match119(1167184,{
     0xA1,0xF8,0x17,0x12,0x00,0x8B,0x54,0x24,0x1C,
     0x0F,0xBE,0x40,0x2F,0x89,0x02});
   std::cout<<"RM_PALETTE_ENTRY_EAX_OVERWRITE offset=1167184"
            <<" templateMatch="<<(overwrite119?1:0)
            <<" expression=signExtendByte_globalPlus2F"
            <<" caution=doesNotEstablishEAXAtCopy\n";
   std::cout<<"RM_PALETTE_ENTRY_LIMITATION"
            <<" esiEntryDefinition=unknown eaxAtCopy=unknown"
            <<" countRuntime=unknown rmResourceProvenance=unknown\n";
  }
 }


 // Milestone 120: investigate candidate entry at 1166740 and the ESI
 // register's provenance. Bounded byte-indexed evidence only: no raw
 // candidate is promoted to a reaching definition without control-flow proof.
 {
  std::ifstream in120(std::filesystem::path(argv[1])/"STTNG.OVL",std::ios::binary);
  if(!in120){
   std::cout<<"RM_PALETTE_ESI_ENTRY_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> d120(
    (std::istreambuf_iterator<char>(in120)),std::istreambuf_iterator<char>());
   auto match120=[&](size_t p,const std::vector<uint8_t>& seq){
    return p<=d120.size()&&seq.size()<=d120.size()-p&&
     std::equal(seq.begin(),seq.end(),d120.begin()+p);
   };
   std::cout<<"RM_PALETTE_ESI_ENTRY_STATUS read=1 bytes="<<d120.size()
            <<" scope=boundedByteIndexedAndAnchoredTemplates\n";
   const size_t entry120=1166740;
   const bool saves120=match120(entry120,{0x53,0x56,0x57,0x55})||
                       match120(entry120,{0x56,0x57,0x55});
   std::cout<<"RM_PALETTE_ESI_ENTRY_CANDIDATE offset="<<entry120
            <<" saveTemplateMatch="<<(saves120?1:0)
            <<" functionBoundaryProven=0\n";
   // Print small, independently indexed windows around the candidate
   // prologue and each of the two verified use sites of [ESI+0C].
   const size_t anchors120[]={1166740,1166843,1166886,1166990,1167055,1167199,1167228};
   for(size_t a120:anchors120){
    const size_t begin120=a120>=16?a120-16:0;
    const size_t end120=std::min(d120.size(),a120+33);
    std::cout<<"RM_PALETTE_ESI_WINDOW anchor="<<a120
             <<" start="<<begin120<<" endExclusive="<<end120<<"\n";
    for(size_t p120=begin120;p120<end120;++p120){
     std::cout<<"RM_PALETTE_ESI_BYTE anchor="<<a120
              <<" offset="<<p120<<" hex="<<std::hex
              <<std::setw(2)<<std::setfill('0')<<unsigned(d120[p120])
              <<std::dec<<std::setfill(' ')<<"\n";
    }
   }
   // Anchored ESI-relative operations known from prior verified dumps.
   const struct {size_t offset;std::vector<uint8_t> bytes;const char* meaning;} checks120[]={
    {1166843,{0x8B,0x7E,0x0C},"mov_edi_dword_esi_plus_0C"},
    {1166886,{0x8B,0x6E,0x0C},"mov_ebp_dword_esi_plus_0C"},
    {1167199,{0x66,0x83,0x7E,0x06,0x00},"cmp_word_esi_plus_06_zero"},
    {1167228,{0x8B,0x5E,0x0C},"mov_ebx_dword_esi_plus_0C"}
   };
   for(const auto& ch120:checks120){
    std::cout<<"RM_PALETTE_ESI_USE offset="<<ch120.offset
             <<" templateMatch="<<(match120(ch120.offset,ch120.bytes)?1:0)
             <<" operation="<<ch120.meaning<<"\n";
   }
   // Scan a narrow entry region for possible ESI writes; these are
   // byte patterns, not decoded instructions or proven path assignments.
   const size_t begin120=1166735;
   const size_t end120=std::min(d120.size(),size_t(1166810));
   size_t candidates120=0;
   for(size_t p120=begin120;p120<end120;++p120){
    const bool mov120=p120+2<=end120&&d120[p120]==0x8B&&
                      (d120[p120+1]&0x38)==0x30;
    const bool lea120=p120+2<=end120&&d120[p120]==0x8D&&
                      (d120[p120+1]&0x38)==0x30;
    const bool pop120=d120[p120]==0x5E;
    const bool immediate120=p120+5<=end120&&d120[p120]==0xBE;
    if(!(mov120||lea120||pop120||immediate120))continue;
    ++candidates120;
    std::cout<<"RM_PALETTE_ESI_WRITE_CANDIDATE offset="<<p120
             <<" kind="<<(mov120?"mov":lea120?"lea":pop120?"pop":"immediate")
             <<" scope=rawCandidateNotInstructionBoundary\n";
   }
   std::cout<<"RM_PALETTE_ESI_WRITE_SUMMARY start="<<begin120
            <<" endExclusive="<<end120<<" candidates="<<candidates120<<"\n";
   std::cout<<"RM_PALETTE_ESI_LIMITATION"
            <<" functionEntry=unverified"
            <<" esiReachingDefinition=unknown"
            <<" pointerOwnership=unknown"
            <<" rmPaletteLink=unverified\n";
  }
 }


 // Milestone 120 follow-up: independently index exact bytes around the
 // candidate ESI definition and the two previously mismatched use sites.
 // Only complete local byte sequences count as matches.
 {
  std::ifstream in120b(std::filesystem::path(argv[1])/"STTNG.OVL",std::ios::binary);
  if(!in120b){
   std::cout<<"RM_PALETTE_ESI_INDEX_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> d120b(
    (std::istreambuf_iterator<char>(in120b)),std::istreambuf_iterator<char>());
   auto matches120b=[&](size_t p,const std::vector<uint8_t>& seq){
    return p<=d120b.size()&&seq.size()<=d120b.size()-p&&
           std::equal(seq.begin(),seq.end(),d120b.begin()+p);
   };
   std::cout<<"RM_PALETTE_ESI_INDEX_STATUS read=1 bytes="<<d120b.size()
            <<" scope=byteIndexedWindowsNotDisassembly\n";
   const size_t anchors120b[]={1166740,1166747,1166843,1166886};
   for(size_t a120b:anchors120b){
    const size_t start120b=a120b>=12?a120b-12:0;
    const size_t end120b=std::min(d120b.size(),a120b+20);
    std::cout<<"RM_PALETTE_ESI_INDEX_WINDOW anchor="<<a120b
             <<" start="<<start120b<<" endExclusive="<<end120b<<"\n";
    for(size_t p120b=start120b;p120b<end120b;++p120b)
     std::cout<<"RM_PALETTE_ESI_INDEX_BYTE anchor="<<a120b
              <<" offset="<<p120b<<" hex="<<std::hex
              <<std::setw(2)<<std::setfill('0')<<unsigned(d120b[p120b])
              <<std::dec<<std::setfill(' ')<<"\n";
   }
   struct Pattern120b{const char* label;size_t begin;size_t end;
                      std::vector<uint8_t> seq;};
   const Pattern120b patterns120b[]={
    {"esi_mov_candidate",1166735,1166770,{0x8B,0x74,0x24}},
    {"esi_read_edi",1166828,1166860,{0x8B,0x7E,0x0C}},
    {"esi_read_ebp",1166871,1166905,{0x8B,0x6E,0x0C}}
   };
   for(const auto& pat120b:patterns120b){
    size_t count120b=0;
    const size_t end120b=std::min(pat120b.end,d120b.size());
    for(size_t p120b=pat120b.begin;
        p120b<=end120b&&pat120b.seq.size()<=end120b-p120b;++p120b){
     if(!matches120b(p120b,pat120b.seq))continue;
     ++count120b;
     std::cout<<"RM_PALETTE_ESI_INDEX_MATCH label="<<pat120b.label
              <<" offset="<<p120b
              <<" bytes="<<pat120b.seq.size()
              <<" caution=instructionBoundaryAndReachabilityUnverified\n";
    }
    std::cout<<"RM_PALETTE_ESI_INDEX_SUMMARY label="<<pat120b.label
             <<" matches="<<count120b
             <<" start="<<pat120b.begin
             <<" endExclusive="<<end120b<<"\n";
   }
   std::cout<<"RM_PALETTE_ESI_INDEX_LIMITATION"
            <<" esiReachingDefinition=unknown"
            <<" sourcePointerOwnership=unknown"
            <<" rmPaletteConnection=unverified\n";
  }
 }


 // Milestone 121: enumerate raw direct near CALL targets of the candidate
 // palette function. x86 rel32 targets are computed in flat file offsets.
 // Raw E8 patterns are NOT proof of instruction boundaries or reachability.
 {
  std::ifstream in121(std::filesystem::path(argv[1])/"STTNG.OVL",std::ios::binary);
  if(!in121){
   std::cout<<"RM_PALETTE_CALLER_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> d121(
    (std::istreambuf_iterator<char>(in121)),std::istreambuf_iterator<char>());
   const int64_t target121=1166740;
   size_t count121=0;
   std::cout<<"RM_PALETTE_CALLER_STATUS read=1 bytes="<<d121.size()
            <<" target="<<target121
            <<" scope=rawRel32CandidateSearchNotInstructionDecoder\n";
   for(size_t p121=0;p121+5<=d121.size();++p121){
    if(d121[p121]!=0xE8)continue;
    const uint32_t raw121=uint32_t(d121[p121+1])|
      (uint32_t(d121[p121+2])<<8)|
      (uint32_t(d121[p121+3])<<16)|
      (uint32_t(d121[p121+4])<<24);
    const int64_t destination121=int64_t(p121)+5+int64_t(int32_t(raw121));
    if(destination121!=target121)continue;
    ++count121;
    std::cout<<"RM_PALETTE_CALLER_CANDIDATE offset="<<p121
             <<" flatTarget="<<destination121
             <<" caution=rawOpcodeNotVerifiedCallBoundary\n";
    const size_t start121=p121>=48?p121-48:0;
    const size_t end121=std::min(d121.size(),p121+17);
    std::cout<<"RM_PALETTE_CALLER_WINDOW callOffset="<<p121
             <<" start="<<start121<<" endExclusive="<<end121<<"\n";
    for(size_t row121=start121;row121<end121;row121+=16){
     std::cout<<"RM_PALETTE_CALLER_HEX callOffset="<<p121
              <<" offset="<<row121<<" hex=";
     for(size_t j121=row121;j121<std::min(row121+16,end121);++j121){
      if(j121!=row121)std::cout<<" ";
      std::cout<<std::hex<<std::setw(2)<<std::setfill('0')
               <<unsigned(d121[j121]);
     }
     std::cout<<std::dec<<std::setfill(' ')<<"\n";
    }
   }
   std::cout<<"RM_PALETTE_CALLER_SUMMARY target="<<target121
            <<" candidates="<<count121<<"\n";
   std::cout<<"RM_PALETTE_CALLER_LIMITATION"
            <<" indirectCallsNotCovered=1"
            <<" callBoundariesUnverified=1"
            <<" callerArgumentProvenance=unknown"
            <<" rmResourceLink=unverified\n";
  }
 }


 // Milestone 122: locate literal references to the candidate palette
 // argument globals and possible pointer field. An absolute memory operand
 // is not equivalent to an effective-address reference via a base register.
 // This intentionally avoids claiming that a reference proves a write.
 {
  std::ifstream in122(std::filesystem::path(argv[1])/"STTNG.OVL",std::ios::binary);
  if(!in122){
   std::cout<<"RM_PALETTE_STRUCT_REF_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> d122(
    (std::istreambuf_iterator<char>(in122)),std::istreambuf_iterator<char>());
   struct Address122{uint32_t value;const char* role;};
   const Address122 addresses122[]={
    {0x00099758u,"firstArgumentStructure"},
    {0x00099764u,"candidateRgbPointerField"},
    {0x00099750u,"secondArgument"}
   };
   std::cout<<"RM_PALETTE_STRUCT_REF_STATUS read=1 bytes="<<d122.size()
            <<" scope=literalAddressSurveyNotInstructionDecoder\n";
   for(const auto& address122:addresses122){
    const uint32_t v122=address122.value;
    const uint8_t bytes122[]={
     uint8_t(v122&255),uint8_t((v122>>8)&255),
     uint8_t((v122>>16)&255),uint8_t((v122>>24)&255)};
    size_t matches122=0;
    for(size_t p122=0;p122+4<=d122.size();++p122){
     if(d122[p122]!=bytes122[0]||d122[p122+1]!=bytes122[1]||
        d122[p122+2]!=bytes122[2]||d122[p122+3]!=bytes122[3])continue;
     ++matches122;
     const size_t start122=p122>=12?p122-12:0;
     const size_t end122=std::min(d122.size(),p122+16);
     std::cout<<"RM_PALETTE_STRUCT_REF_MATCH role="<<address122.role
              <<" address=0x"<<std::hex<<std::setw(8)<<std::setfill('0')
              <<v122<<std::dec<<std::setfill(' ')
              <<" literalOffset="<<p122
              <<" precedingOpcode=";
     if(p122)std::cout<<std::hex<<std::setw(2)<<std::setfill('0')
                      <<unsigned(d122[p122-1])<<std::dec<<std::setfill(' ');
     else std::cout<<"none";
     std::cout<<" caution=rawBytesNotConfirmedOperand\n";
     std::cout<<"RM_PALETTE_STRUCT_REF_HEX role="<<address122.role
              <<" literalOffset="<<p122<<" start="<<start122<<" hex=";
     for(size_t j122=start122;j122<end122;++j122){
      if(j122!=start122)std::cout<<" ";
      std::cout<<std::hex<<std::setw(2)<<std::setfill('0')
               <<unsigned(d122[j122]);
     }
     std::cout<<std::dec<<std::setfill(' ')<<"\n";
    }
    std::cout<<"RM_PALETTE_STRUCT_REF_SUMMARY role="<<address122.role
             <<" address="<<v122<<" rawMatches="<<matches122<<"\n";
   }
   std::cout<<"RM_PALETTE_STRUCT_REF_LIMITATION"
            <<" absoluteOperandWrites=unverified"
            <<" computedAddressReferences=notEnumerated"
            <<" runtimePointerValue=unknown"
            <<" rmResourceProvenance=unverified\n";
  }
 }


 // Milestone 123: byte-indexed investigation of the structure initialization
 // and intervening direct CALL before the palette routine. Anchored x86
 // templates are checked separately from raw hex; no runtime provenance claim.
 {
  std::ifstream in123(std::filesystem::path(argv[1])/"STTNG.OVL",std::ios::binary);
  if(!in123){
   std::cout<<"RM_PALETTE_INIT_TRACE_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> d123(
    (std::istreambuf_iterator<char>(in123)),std::istreambuf_iterator<char>());
   auto match123=[&](size_t p,const std::vector<uint8_t>& bytes){
    return p<=d123.size()&&bytes.size()<=d123.size()-p&&
      std::equal(bytes.begin(),bytes.end(),d123.begin()+p);
   };
   auto callTarget123=[&](size_t p)->int64_t{
    if(p+5>d123.size()||d123[p]!=0xE8)return -1;
    const uint32_t raw=uint32_t(d123[p+1])|
      (uint32_t(d123[p+2])<<8)|
      (uint32_t(d123[p+3])<<16)|
      (uint32_t(d123[p+4])<<24);
    return int64_t(p)+5+int64_t(int32_t(raw));
   };
   std::cout<<"RM_PALETTE_INIT_TRACE_STATUS read=1 bytes="<<d123.size()
            <<" scope=boundedIndexedHexAndAnchoredTemplates\n";
   // Include code before the observed initialization to investigate EAX's
   // origin, as well as the CALL that runs between reset and palette setup.
   const size_t anchors123[]={522079,522091,522102,522123,522714,522730,522754};
   for(size_t a123:anchors123){
    const size_t begin123=a123>=48?a123-48:0;
    const size_t end123=std::min(d123.size(),a123+24);
    std::cout<<"RM_PALETTE_INIT_TRACE_WINDOW anchor="<<a123
             <<" start="<<begin123<<" endExclusive="<<end123<<"\n";
    for(size_t row123=begin123;row123<end123;row123+=16){
     std::cout<<"RM_PALETTE_INIT_TRACE_HEX anchor="<<a123
              <<" offset="<<row123<<" hex=";
     for(size_t j123=row123;j123<std::min(row123+16,end123);++j123){
      if(j123!=row123)std::cout<<" ";
      std::cout<<std::hex<<std::setw(2)<<std::setfill('0')
               <<unsigned(d123[j123]);
     }
     std::cout<<std::dec<<std::setfill(' ')<<"\n";
    }
   }
   // Verify the exact source-buffer setup sequence as observed in M122.
   const bool base123=match123(522083,{
    0x8B,0x58,0x06,0x8D,0x0C,0x18,0x89,0x0D,0x58,0x97,0x09,0x00});
   const bool clear123=match123(522095,{
    0x31,0xC9,0x8B,0x70,0x0A,0x89,0x0D,0x64,0x97,0x09,0x00});
   const bool assign123=match123(522120,{
    0x01,0xF0,0xA3,0x64,0x97,0x09,0x00});
   std::cout<<"RM_PALETTE_INIT_TRACE_TEMPLATES"
            <<" basePointerMatch="<<(base123?1:0)
            <<" clearPointerMatch="<<(clear123?1:0)
            <<" conditionalPointerAssignMatch="<<(assign123?1:0)
            <<" caution=offsetsRequireVerification\n";
   // Upstream call: the initializer passes [global 0x997A4]+0x1C4
   // to a helper, then uses the returned EAX as a candidate base pointer.
   // Verify the whole local sequence and compute the relative target.
   const bool upstream123=match123(522048,{
    0xA1,0xA4,0x97,0x09,0x00,
    0x05,0xC4,0x01,0x00,0x00,
    0x50,0xE8,0xD0,0xFC,0xFF,0xFF,
    0x83,0xC4,0x04,0x89,0xC2,0x85,0xC0});
   std::cout<<"RM_PALETTE_INIT_TRACE_UPSTREAM"
            <<" callOffset=522059"
            <<" templateMatch="<<(upstream123?1:0);
   if(upstream123)
    std::cout<<" flatTarget="<<callTarget123(522059);
   std::cout<<" argumentExpression=dword_at_000997A4_plus_0x1C4"
            <<" returnRegister=EAX"
            <<" caution=returnValueSemanticsUnknown\n";
   // Derive CALL location from a verified short sequence rather than
   // assuming its position; the target uses flat file-offset arithmetic.
   const size_t callStart123=522720,callEnd123=std::min(d123.size(),size_t(522740));
   size_t calls123=0;
   for(size_t p123=callStart123;p123+5<=callEnd123;++p123){
    if(d123[p123]!=0xE8)continue;
    ++calls123;
    std::cout<<"RM_PALETTE_INIT_TRACE_PRECALL offset="<<p123
             <<" flatTarget="<<callTarget123(p123)
             <<" caution=rawCallCandidateBoundaryUnverified\n";
   }
   std::cout<<"RM_PALETTE_INIT_TRACE_PRECALL_SUMMARY candidates="<<calls123
            <<" start="<<callStart123<<" endExclusive="<<callEnd123<<"\n";
   std::cout<<"RM_PALETTE_INIT_TRACE_LIMITATION"
            <<" eaxOrigin=unknown"
            <<" precallEffect=unknown"
            <<" esiSourceObject=unknown"
            <<" rmResourceProvenance=unverified\n";
  }
 }


 // Milestone 124: inspect the upstream helper at flat offset 521248.
 // Hex windows are byte-indexed and CALL candidates are raw rel32 patterns;
 // they do not establish instruction boundaries or runtime return semantics.
 {
  std::ifstream in124(std::filesystem::path(argv[1])/"STTNG.OVL",std::ios::binary);
  if(!in124){
   std::cout<<"RM_PALETTE_HELPER_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> d124(
    (std::istreambuf_iterator<char>(in124)),std::istreambuf_iterator<char>());
   auto matches124=[&](size_t p,const std::vector<uint8_t>& seq){
    return p<=d124.size()&&seq.size()<=d124.size()-p&&
           std::equal(seq.begin(),seq.end(),d124.begin()+p);
   };
   auto target124=[&](size_t p)->int64_t{
    if(p+5>d124.size()||d124[p]!=0xE8)return -1;
    const uint32_t u=uint32_t(d124[p+1])|
      (uint32_t(d124[p+2])<<8)|
      (uint32_t(d124[p+3])<<16)|
      (uint32_t(d124[p+4])<<24);
    return int64_t(p)+5+int64_t(int32_t(u));
   };
   const size_t helper124=521248;
   const size_t start124=helper124>=32?helper124-32:0;
   const size_t end124=std::min(d124.size(),helper124+256);
   std::cout<<"RM_PALETTE_HELPER_STATUS read=1 bytes="<<d124.size()
            <<" helperOffset="<<helper124
            <<" scope=boundedHexAndRawCallCandidates\n";
   for(size_t row124=start124;row124<end124;row124+=16){
    std::cout<<"RM_PALETTE_HELPER_HEX offset="<<row124<<" hex=";
    for(size_t j124=row124;j124<std::min(end124,row124+16);++j124){
     if(j124!=row124)std::cout<<" ";
     std::cout<<std::hex<<std::setw(2)<<std::setfill('0')
              <<unsigned(d124[j124]);
    }
    std::cout<<std::dec<<std::setfill(' ')<<"\n";
   }
   // Caller-to-helper relationship established in M123.
   const bool caller124=matches124(522048,{
    0xA1,0xA4,0x97,0x09,0x00,0x05,0xC4,0x01,0x00,0x00,
    0x50,0xE8,0xD0,0xFC,0xFF,0xFF,0x83,0xC4,0x04});
   std::cout<<"RM_PALETTE_HELPER_CALLER callOffset=522059"
            <<" templateMatch="<<(caller124?1:0)
            <<" flatTarget="<<target124(522059)
            <<" argument=global_000997A4_plus_0x1C4\n";
   size_t calls124=0;
   for(size_t p124=helper124;p124+5<=end124;++p124){
    if(d124[p124]!=0xE8)continue;
    ++calls124;
    std::cout<<"RM_PALETTE_HELPER_CALL_CANDIDATE offset="<<p124
             <<" flatTarget="<<target124(p124)
             <<" caution=rawOpcodeNotDecodedInstruction\n";
   }
   std::cout<<"RM_PALETTE_HELPER_CALL_SUMMARY candidates="<<calls124
            <<" start="<<helper124<<" endExclusive="<<end124<<"\n";
   std::cout<<"RM_PALETTE_HELPER_LIMITATION"
            <<" functionBoundary=unverified"
            <<" returnValueConstruction=unknown"
            <<" stackArgumentUse=unknown"
            <<" indirectCallsNotEnumerated=1"
            <<" rmResourceProvenance=unverified\n";
  }
 }


 // Milestone 125: bounded, byte-indexed helper branch and record-field survey.
 // This is a static inspection, not a control-flow graph or runtime proof.
 {
  std::ifstream in125(std::filesystem::path(argv[1])/"STTNG.OVL",std::ios::binary);
  if(!in125){
   std::cout<<"RM_PALETTE_HELPER_RECORD_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> d125(
    (std::istreambuf_iterator<char>(in125)),std::istreambuf_iterator<char>());
   const size_t start125=521248,end125=std::min(d125.size(),size_t(521488));
   auto match125=[&](size_t p,const std::vector<uint8_t>& v){
    return p<=d125.size()&&v.size()<=d125.size()-p&&
           std::equal(v.begin(),v.end(),d125.begin()+p);
   };
   auto u32125=[&](size_t p)->uint32_t{
    return uint32_t(d125[p])|(uint32_t(d125[p+1])<<8)|
           (uint32_t(d125[p+2])<<16)|(uint32_t(d125[p+3])<<24);
   };
   std::cout<<"RM_PALETTE_HELPER_RECORD_STATUS read=1 bytes="<<d125.size()
            <<" start="<<start125<<" endExclusive="<<end125
            <<" scope=anchoredTemplatesAndRawBranchSurvey\n";
   struct Pattern125{const char* name;size_t offset;std::vector<uint8_t> bytes;const char* meaning;};
   const std::vector<Pattern125> patterns125={
    {"entry_stack_arg",521248,{0x53,0x56,0x8B,0x5C,0x24,0x0C,0x8B,0x03},"argumentDereference"},
    {"state_8000",521258,{0x66,0x8B,0x50,0x10,0x81,0xFA,0x00,0x80,0x00,0x00},"stateComparison"},
    {"record_offset_0c",521270,{0x8B,0x50,0x0C,0x8B,0x5A,0x02,0x8D,0x0C,0x1A},"relativePointerCandidate"},
    {"record_update_0c",521279,{0x8B,0x58,0x08,0x89,0x48,0x0C},"recordMutation"},
    {"state_set_8001",521289,{0x66,0xC7,0x40,0x10,0x01,0x80},"stateMutation"},
    {"state_8001",521300,{0x81,0xFA,0x01,0x80,0x00,0x00},"stateComparison"},
    {"state_7ff1",521459,{0x81,0xFA,0xF1,0x7F,0x00,0x00},"stateComparison"}
   };
   for(const auto& p125:patterns125)
    std::cout<<"RM_PALETTE_HELPER_RECORD_TEMPLATE name="<<p125.name
             <<" offset="<<p125.offset<<" match="<<(match125(p125.offset,p125.bytes)?1:0)
             <<" interpretation="<<p125.meaning<<"\n";
   // Verify exact epilogues, not merely occurrences of byte 0xC3.
   // Return-register state is described only where directly established.
   struct Return125{const char* name;size_t offset;std::vector<uint8_t> bytes;const char* eax;};
   const std::vector<Return125> returnsVerified125={
    {"state_8000_pointer",521295,{0x89,0xD0,0x5E,0x5B,0xC3},"EDX"},
    {"state_8001_pointer",521369,{0x5E,0x5B,0xC3},"pathDependent"},
    {"alternate_pointer",521431,{0x89,0xD0,0x5E,0x5B,0xC3},"EDX"},
    {"zero_exit",521478,{0x31,0xC0,0x5E,0x5B,0x8B,0xC0,0xC3},"zero"}
   };
   for(const auto& ret125:returnsVerified125)
    std::cout<<"RM_PALETTE_HELPER_RECORD_RETURN_TEMPLATE name="<<ret125.name
             <<" offset="<<ret125.offset
             <<" match="<<(match125(ret125.offset,ret125.bytes)?1:0)
             <<" eax="<<ret125.eax
             <<" caution=localBytesOnlyNotPathReachability\n";
   size_t returns125=0,branches125=0;
   for(size_t p125=start125;p125<end125;++p125){
    if(d125[p125]==0xC3){
     ++returns125;
     std::cout<<"RM_PALETTE_HELPER_RECORD_RET offset="<<p125
              <<" caution=rawByteNotProvenInstruction\n";
    }
    if(p125+2<=end125 && (d125[p125]==0x74||d125[p125]==0x75||d125[p125]==0xEB)){
     ++branches125;
     const int8_t disp=static_cast<int8_t>(d125[p125+1]);
     std::cout<<"RM_PALETTE_HELPER_RECORD_BRANCH offset="<<p125
              <<" opcode="<<std::hex<<unsigned(d125[p125])<<std::dec
              <<" flatTarget="<<(int64_t(p125)+2+disp)
              <<" caution=rawByteNotProvenInstruction\n";
    }else if(p125+6<=end125 && d125[p125]==0x0F &&
             (d125[p125+1]==0x84||d125[p125+1]==0x85)){
     ++branches125;
     const int32_t disp=static_cast<int32_t>(u32125(p125+2));
     std::cout<<"RM_PALETTE_HELPER_RECORD_BRANCH offset="<<p125
              <<" opcode=0f"<<std::hex<<unsigned(d125[p125+1])<<std::dec
              <<" flatTarget="<<(int64_t(p125)+6+disp)
              <<" caution=rawByteNotProvenInstruction\n";
    }
   }
   std::cout<<"RM_PALETTE_HELPER_RECORD_SUMMARY rawReturns="<<returns125
            <<" rawBranches="<<branches125<<"\n";
   std::cout<<"RM_PALETTE_HELPER_RECORD_LIMITATION"
            <<" branchReachability=unknown"
            <<" recordLayout=unverified"
            <<" returnValuePaths=notProven"
            <<" rmResourceProvenance=unverified\n";
  }
 }


 // Milestone 126: survey direct literal references to the upstream owner
 // pointer (0x000997A4), plus bounded context and likely instruction shapes.
 // Literal references are not proof of ownership or decoded instruction flow.
 {
  std::ifstream in126(std::filesystem::path(argv[1])/"STTNG.OVL",std::ios::binary);
  if(!in126){
   std::cout<<"RM_PALETTE_OWNER_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> d126(
    (std::istreambuf_iterator<char>(in126)),std::istreambuf_iterator<char>());
   const std::vector<uint8_t> address126={0xA4,0x97,0x09,0x00};
   size_t matches126=0,printed126=0;
   std::cout<<"RM_PALETTE_OWNER_STATUS read=1 bytes="<<d126.size()
            <<" address=0x000997A4 scope=rawLiteralAddressSurvey\n";
   for(size_t p126=0;p126+address126.size()<=d126.size();++p126){
    if(!std::equal(address126.begin(),address126.end(),d126.begin()+p126))
     continue;
    ++matches126;
    if(printed126>=96)continue;
    ++printed126;
    const size_t begin126=p126>=12?p126-12:0;
    const size_t end126=std::min(d126.size(),p126+24);
    std::cout<<"RM_PALETTE_OWNER_MATCH literalOffset="<<p126
             <<" precedingByte="<<(p126?unsigned(d126[p126-1]):0)
             <<" caution=rawLiteralNotDecodedOperand\n";
    for(size_t row126=begin126;row126<end126;row126+=16){
     std::cout<<"RM_PALETTE_OWNER_HEX literalOffset="<<p126
              <<" offset="<<row126<<" hex=";
     for(size_t j126=row126;j126<std::min(end126,row126+16);++j126){
      if(j126!=row126)std::cout<<" ";
      std::cout<<std::hex<<std::setw(2)<<std::setfill('0')
               <<unsigned(d126[j126]);
     }
     std::cout<<std::dec<<std::setfill(' ')<<"\n";
    }
   }
   std::cout<<"RM_PALETTE_OWNER_SUMMARY rawMatches="<<matches126
            <<" printed="<<printed126<<" truncated="<<(matches126>printed126?1:0)<<"\n";
   // Known caller access and the offset applied to its loaded pointer.
   const size_t known126=522048;
   const std::vector<uint8_t> knownBytes126={
    0xA1,0xA4,0x97,0x09,0x00,0x05,0xC4,0x01,0x00,0x00,
    0x50,0xE8,0xD0,0xFC,0xFF,0xFF};
   const bool knownMatch126=known126+knownBytes126.size()<=d126.size()&&
    std::equal(knownBytes126.begin(),knownBytes126.end(),d126.begin()+known126);
   std::cout<<"RM_PALETTE_OWNER_KNOWN_ACCESS offset="<<known126
            <<" templateMatch="<<(knownMatch126?1:0)
            <<" operation=loadGlobalThenAdd0x1C4AndCall521248\n";
   // Survey literal displacement 0x1C4 in the same overlay. This alone
   // cannot associate a field access with the owner pointer.
   const std::vector<uint8_t> disp126={0xC4,0x01,0x00,0x00};
   size_t dispCount126=0,dispPrinted126=0;
   for(size_t p126=0;p126+disp126.size()<=d126.size();++p126){
    if(!std::equal(disp126.begin(),disp126.end(),d126.begin()+p126))continue;
    ++dispCount126;
    if(dispPrinted126>=64)continue;
    ++dispPrinted126;
    std::cout<<"RM_PALETTE_OWNER_FIELD_CANDIDATE literalOffset="<<p126
             <<" precedingByte="<<(p126?unsigned(d126[p126-1]):0)
             <<" caution=rawDisplacementNotOwnerAssociation\n";
   }
   std::cout<<"RM_PALETTE_OWNER_FIELD_SUMMARY rawMatches="<<dispCount126
            <<" printed="<<dispPrinted126
            <<" truncated="<<(dispCount126>dispPrinted126?1:0)<<"\n";
   std::cout<<"RM_PALETTE_OWNER_LIMITATION directWrites=notDecoded"
            <<" computedReferences=notEnumerated"
            <<" structureAllocation=unknown"
            <<" resourceLoaderLink=unverified"
            <<" rmPaletteProvenance=unverified\n";
  }
 }


 // Milestone 127: inspect the producer of EAX before the owner-pointer write,
 // and verify the nearby FVF signature comparison. Read-only raw evidence.
 {
  std::ifstream in127(std::filesystem::path(argv[1])/"STTNG.OVL",std::ios::binary);
  if(!in127){
   std::cout<<"RM_PALETTE_OWNER_PRODUCER_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> d127(
    (std::istreambuf_iterator<char>(in127)),std::istreambuf_iterator<char>());
   auto match127=[&](size_t at,const std::vector<uint8_t>& b){
    return at+b.size()<=d127.size()&&
     std::equal(b.begin(),b.end(),d127.begin()+at);
   };
   auto dump127=[&](const char* label,size_t start,size_t end){
    end=std::min(end,d127.size());
    for(size_t p=start;p<end;p+=16){
     std::cout<<"RM_PALETTE_OWNER_PRODUCER_HEX region="<<label
              <<" offset="<<p<<" hex=";
     for(size_t j=p;j<std::min(p+16,end);++j){
      if(j!=p)std::cout<<" ";
      std::cout<<std::hex<<std::setw(2)<<std::setfill('0')
               <<unsigned(d127[j]);
     }
     std::cout<<std::dec<<std::setfill(' ')<<"\n";
    }
   };
   std::cout<<"RM_PALETTE_OWNER_PRODUCER_STATUS read=1 bytes="<<d127.size()
            <<" scope=boundedRawBytesAndAnchoredTemplates\n";
   dump127("pre_assignment",522848,523024);
   const bool assignment127=match127(522991,{0xA3,0xA4,0x97,0x09,0x00});
   const bool nearby127=match127(522985,{0x68,0x98,0x02,0x00,0x00,0x50,0xA3,0xA4,0x97,0x09,0x00});
   std::cout<<"RM_PALETTE_OWNER_PRODUCER_ASSIGNMENT offset=522991 match="
            <<(assignment127?1:0)
            <<" operation=storeEAXToGlobal caution=EAXProducerNotYetEstablished\n";
   std::cout<<"RM_PALETTE_OWNER_PRODUCER_NEARBY offset=522985 match="
            <<(nearby127?1:0)
            <<" operation=push0x298_pushEAX_storeEAX caution=localSequenceOnly\n";
   // Identify CALL rel32 candidates within the bounded upstream window.
   size_t calls127=0;
   for(size_t p=522848;p+5<=std::min(size_t(523024),d127.size());++p){
    if(d127[p]!=0xE8)continue;
    const uint32_t v=uint32_t(d127[p+1])|
     (uint32_t(d127[p+2])<<8)|(uint32_t(d127[p+3])<<16)|
     (uint32_t(d127[p+4])<<24);
    const int32_t disp=static_cast<int32_t>(v);
    ++calls127;
    std::cout<<"RM_PALETTE_OWNER_PRODUCER_CALL offset="<<p
             <<" flatTarget="<<(int64_t(p)+5+disp)
             <<" caution=rawOpcodeCandidateNotDisassembled\n";
   }
   std::cout<<"RM_PALETTE_OWNER_PRODUCER_CALL_SUMMARY rawCandidates="<<calls127<<"\n";
   const bool fvf127=match127(523142,{
    0xA1,0xA4,0x97,0x09,0x00,0x81,0xB8,0xCC,0x01,0x00,0x00,
    0x46,0x56,0x46,0x20,0x74,0x35});
   std::cout<<"RM_PALETTE_OWNER_FVF_TEMPLATE offset=523142 match="<<(fvf127?1:0)
            <<" signatureBytes=46564620 conditionalBranch=74_35"
            <<" caution=signatureDoesNotProveRMOwnership\n";
   dump127("fvf_branch",523120,523208);
   std::cout<<"RM_PALETTE_OWNER_PRODUCER_LIMITATION"
            <<" instructionBoundaries=notFullyDecoded"
            <<" runtimeEAX=unknown"
            <<" objectAllocation=unverified"
            <<" resourceLoaderLink=unverified"
            <<" rmPaletteProvenance=unverified\n";
  }
 }


 // Milestone 128: bounded investigation of the function at 518504 whose
 // returned EAX is stored into global 0x000997A4 by the caller at 522991.
 // This is raw file-offset evidence, not a control-flow disassembly.
 {
  std::ifstream in128(std::filesystem::path(argv[1])/"STTNG.OVL",std::ios::binary);
  if(!in128){
   std::cout<<"RM_PALETTE_OWNER_FACTORY_STATUS read=0\n";
  }else{
   const std::vector<uint8_t> d128(
    (std::istreambuf_iterator<char>(in128)),std::istreambuf_iterator<char>());
   const size_t start128=518504,end128=518888;
   std::cout<<"RM_PALETTE_OWNER_FACTORY_STATUS read=1 bytes="<<d128.size()
            <<" start="<<start128<<" endExclusive="<<end128
            <<" scope=boundedHexAndRawCallReturnSurvey\n";
   for(size_t p=start128;p<std::min(end128,d128.size());p+=16){
    std::cout<<"RM_PALETTE_OWNER_FACTORY_HEX offset="<<p<<" hex=";
    for(size_t j=p;j<std::min(p+16,std::min(end128,d128.size()));++j){
     if(j!=p)std::cout<<" ";
     std::cout<<std::hex<<std::setw(2)<<std::setfill('0')<<unsigned(d128[j]);
    }
    std::cout<<std::dec<<std::setfill(' ')<<"\n";
   }
   size_t calls128=0,returns128=0,branches128=0;
   for(size_t p=start128;p<std::min(end128,d128.size());++p){
    if(d128[p]==0xC3||d128[p]==0xC2){
     ++returns128;
     std::cout<<"RM_PALETTE_OWNER_FACTORY_RETURN offset="<<p
              <<" opcode="<<std::hex<<unsigned(d128[p])<<std::dec
              <<" caution=rawByteNotProvenInstruction\n";
    }
    if(p+5<=d128.size()&&p+5<=end128&&d128[p]==0xE8){
     const uint32_t v=uint32_t(d128[p+1])|(uint32_t(d128[p+2])<<8)|
      (uint32_t(d128[p+3])<<16)|(uint32_t(d128[p+4])<<24);
     const int32_t disp=static_cast<int32_t>(v);
     ++calls128;
     std::cout<<"RM_PALETTE_OWNER_FACTORY_CALL offset="<<p
              <<" flatTarget="<<(int64_t(p)+5+disp)
              <<" caution=rawOpcodeCandidateNotDisassembled\n";
    }
    if(p+2<=end128&&p+2<=d128.size()&&
       (d128[p]==0x74||d128[p]==0x75||d128[p]==0xEB)){
     ++branches128;
     const int8_t disp=static_cast<int8_t>(d128[p+1]);
     std::cout<<"RM_PALETTE_OWNER_FACTORY_BRANCH offset="<<p
              <<" flatTarget="<<(int64_t(p)+2+disp)
              <<" caution=rawOpcodeCandidateNotDisassembled\n";
    }
   }
   std::cout<<"RM_PALETTE_OWNER_FACTORY_SUMMARY rawCalls="<<calls128
            <<" rawReturns="<<returns128<<" rawShortBranches="<<branches128<<"\n";
   const std::vector<uint8_t> caller128={
    0xE8,0x82,0xEE,0xFF,0xFF,0x83,0xC4,0x0C,0x68,0x98,0x02,
    0x00,0x00,0x50,0xA3,0xA4,0x97,0x09,0x00};
   const bool callerMatch128=522977+caller128.size()<=d128.size()&&
    std::equal(caller128.begin(),caller128.end(),d128.begin()+522977);
   std::cout<<"RM_PALETTE_OWNER_FACTORY_CALLER offset=522977 match="
            <<(callerMatch128?1:0)
            <<" callTarget=518504 storeOffset=522991"
            <<" caution=returnValueSemanticsUnverified\n";
   std::cout<<"RM_PALETTE_OWNER_FACTORY_LIMITATION"
            <<" functionExtent=unverified"
            <<" returnPathReachability=unverified"
            <<" EAXMeaning=unknown"
            <<" allocatorOrLoader=unverified"
            <<" rmPaletteProvenance=unverified\n";
  }
 }

 std::cout<<"NOTE: Trailer plausibility does not establish palette ownership.\n";
 std::cout<<"NOTE: Palette overlap is numerical only; it does not establish RM palette provenance.\n";
 return 0;
}
