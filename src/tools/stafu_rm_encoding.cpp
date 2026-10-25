// SPDX-License-Identifier: GPL-3.0-or-later
// Compare indexed-raster and RGB-triplet hypotheses without modifying assets.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/rm_encoding_probe.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
namespace {
void region(const char* label,const std::vector<uint8_t>& bytes,size_t begin,size_t length){
 const auto s=reluva::stafu::inspectRmRegion(bytes.data(),bytes.size(),begin,length);
 std::cout<<"REGION "<<label<<" offset="<<begin<<" bytes="<<s.bytes
  <<" distinct="<<s.distinct<<" indices128to255="<<s.highIndices
  <<" adjacentEqual="<<s.adjacentEqual
  <<" vga6Triplets="<<s.vga6Triplets<<"/"<<s.triplets<<"\n";
}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-rm-encoding <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager resources;std::string error;
 if(!resources.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 constexpr size_t raster=640u*480u;
 for(const char* name:{"BRIDGE.RM","VIEWSCR.RM"}){
  std::vector<uint8_t> bytes;
  if(!resources.read(name,bytes,error,2*1024*1024)){std::cerr<<name<<": "<<error<<"\n";return 1;}
  std::cout<<"RESOURCE "<<name<<" bytes="<<bytes.size()<<" expectedRaster="<<raster<<"\n";
  region("full",bytes,0,bytes.size());
  region("first64",bytes,0,64);
  region("firstRow",bytes,0,640);
  region("topQuarter",bytes,0,640*120);
  region("middleQuarter",bytes,640*120,640*120);
  region("lowerQuarter",bytes,640*240,640*120);
  region("bottomQuarter",bytes,640*360,640*120);
  region("trailer",bytes,raster,bytes.size()>raster?bytes.size()-raster:0);
  for(size_t offset=0;offset<3;++offset){
   const auto s=reluva::stafu::inspectRmRgbHypothesis(bytes.data(),std::min(bytes.size(),raster),offset);
   std::cout<<"RGB_HYPOTHESIS offset="<<offset<<" triplets="<<s.triplets
    <<" vga6Triplets="<<s.vga6Triplets
    <<" adjacentEqualRgb="<<s.adjacentRgbEqual<<"\n";
  }
 }
 std::cout<<"NOTE: RGB6-like prefixes alone do not establish pixel encoding; the 640x480 grayscale interpretation remains provisional.\n";
 return 0;
}
