// SPDX-License-Identifier: GPL-3.0-or-later
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/rm_palette_coverage.h"
#include "engines/stafu/rm_candidate_palette.h"
#include <algorithm>
#include <iostream>
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
 }
 for(const char* name:{"BRIDGE.RM","VIEWSCR.RM"}){
  std::vector<uint8_t> bytes;
  if(!resources.read(name,bytes,error,2*1024*1024)){std::cerr<<name<<": "<<error<<"\n";return 1;}
  const auto r=reluva::stafu::rmPaletteCoverage(bytes.data(),bytes.size());
  if(!r.pixels){std::cerr<<name<<": insufficient raster bytes\n";return 1;}
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
 std::cout<<"NOTE: Palette overlap is numerical only; it does not establish RM palette provenance.\n";
 return 0;
}
