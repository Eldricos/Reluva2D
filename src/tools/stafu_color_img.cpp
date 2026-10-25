// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only investigation of color.img against known IMG and RM resources.
#include "engines/stafu/resource_manager.h"
#include "engines/stafu/color_img_probe.h"
#include "engines/stafu/rm_palette_scan.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
namespace {
void hex(const std::vector<uint8_t>& bytes,size_t begin,size_t count){
 for(size_t i=begin;i<bytes.size()&&i-begin<count;++i)
  std::cout<<" "<<std::hex<<std::setfill('0')<<std::setw(2)<<unsigned(bytes[i]);
 std::cout<<std::dec<<"\n";
}
void describe(const char* name,const std::vector<uint8_t>& bytes){
 const auto s=reluva::stafu::inspectColorImg(bytes.data(),bytes.size());
 std::cout<<"RESOURCE "<<name<<" bytes="<<s.bytes<<" distinctBytes="<<s.distinct
  <<" min="<<s.minimum<<" max="<<s.maximum<<" zeroBytes="<<s.zeros
  <<" vga6Bytes="<<s.vga6Bytes<<"\n";
 std::cout<<"HEAD:";hex(bytes,0,64);
 std::cout<<"TAIL:";hex(bytes,bytes.size()>32?bytes.size()-32:0,32);
 std::cout<<"TOP_VALUES";
 std::vector<unsigned> values(256);
 for(unsigned i=0;i<256;++i)values[i]=i;
 std::sort(values.begin(),values.end(),[&](unsigned a,unsigned b){
  return s.frequency[a]!=s.frequency[b]?s.frequency[a]>s.frequency[b]:a<b;
 });
 for(size_t i=0;i<8;++i)std::cout<<" "<<values[i]<<":"<<s.frequency[values[i]];
 std::cout<<"\n";
}
}
int main(int argc,char** argv){
 if(argc!=3){std::cerr<<"Usage: reluva-stafu-color-img <installed-folder> <cd-folder>\n";return 2;}
 reluva::stafu::ResourceManager resources;std::string error;
 if(!resources.open(argv[1],argv[2],error)){std::cerr<<error<<"\n";return 1;}
 const char* names[]={"color.img","BRIDGE.IMG","DBRIDGE.IMG","BOXFRONT.IMG","BRIDGE.RM"};
 for(const auto* name:names){
  std::vector<uint8_t> bytes;
  if(!resources.read(name,bytes,error,2*1024*1024)){
   std::cout<<"UNAVAILABLE "<<name<<" "<<error<<"\n";
   if(std::string(name)=="color.img")return 1;
   continue;
  }
  describe(name,bytes);
  if(std::string(name)=="color.img"){
   std::cout<<"COLOR_IMG alignedVga6Windows768="
    <<reluva::stafu::countVga6TripletWindows(bytes.data(),bytes.size())<<"\n";
   const auto hits=reluva::stafu::scanVgaPaletteWindows(bytes.data(),bytes.size());
   std::cout<<"COLOR_IMG heuristicVga6Candidates="<<hits.size()<<"\n";
   for(size_t i=0;i<std::min<size_t>(hits.size(),8);++i)std::cout<<" VGA6_OFFSET "<<hits[i]<<"\n";
  }
 }
 std::cout<<"NOTE: Byte statistics and candidate offsets do not prove palette ownership or IMG structure.\n";
 return 0;
}
